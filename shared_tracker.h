#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <iostream>

#if __has_include(<source_location>) && __cplusplus >= 202002L
  #include <source_location>
  using src_loc = std::source_location;
#else
  struct src_loc {
    static src_loc current() noexcept { return {}; }
    const char* file_name() const noexcept { return "?"; }
    unsigned    line() const noexcept { return 0; }
  };
#endif

// ---------------------- Tracker (thread-safe, per-instance) -------------------
struct Tracker {
  struct Hold { std::string tag; std::string where; };

  using Key     = std::weak_ptr<void>;                 // control-block identity
  using HoldMap = std::unordered_map<std::size_t, Hold>;
  using Map     = std::map<Key, HoldMap, std::owner_less<Key>>;

  // Register one "hold" (one shared_ptr instance). Returns a token to remove it.
  static std::size_t add(const std::shared_ptr<void>& sp,
                         std::string tag,
                         src_loc loc = src_loc::current())
  {
    const std::string where =
      std::string(loc.file_name()) + ":" + std::to_string(loc.line());
    std::scoped_lock lk(mutex_());
    const std::size_t id = ++counter_();
    table_()[Key(sp)][id] = Hold{std::move(tag), where};
    return id;
  }

  // Remove a specific hold by (control block, token). Safe to call multiple times.
  static void remove_hold(const std::weak_ptr<void>& key, std::size_t token) {
    std::scoped_lock lk(mutex_());
    auto it = table_().find(key);
    if (it == table_().end()) return;
    it->second.erase(token);
    if (it->second.empty() && it->first.expired())
      table_().erase(it); // tidy up fully-expired entries
  }

  // Optional: remove whole entry when the control block is gone.
  static void remove_if_expired(const std::shared_ptr<void>& sp) {
    std::scoped_lock lk(mutex_());
    auto it = table_().find(Key(sp));
    if (it != table_().end() && it->first.expired())
      table_().erase(it);
  }

  // Snapshot holds for a given owner (for logging/debug UI)
  static std::vector<Hold> snapshot(const std::shared_ptr<void>& sp) {
    std::scoped_lock lk(mutex_());
    std::vector<Hold> out;
    if (auto it = table_().find(Key(sp)); it != table_().end()) {
      out.reserve(it->second.size());
      for (auto& [_, h] : it->second) out.push_back(h);
    }
    return out;
  }

  static void dump(const std::shared_ptr<void>& sp, std::ostream& os = std::cerr){
    auto holds = snapshot(sp);          // copy without holding the mutex during I/O
    if (holds.empty()) { os << "(no holders logged)\n"; return; }
    for (auto& h : holds) os << "held by: " << h.tag << " @ " << h.where << "\n";
  }

  // Periodic full sweep (optional)
  static void sweep_expired() {
    std::scoped_lock lk(mutex_());
    auto& m = table_();
    for (auto it = m.begin(); it != m.end(); )
      it = it->first.expired() ? m.erase(it) : std::next(it);
  }

private:
  static Map& table_()             { static Map m; return m; }
  static std::mutex& mutex_()      { static std::mutex mx; return mx; }
  static std::atomic<std::size_t>& counter_() { static std::atomic<std::size_t> c{0}; return c; }
};

// ---------------------- tracked_shared_ptr wrapper (RAII) ---------------------
template<class T>
class tracked_shared_ptr {
public:
  tracked_shared_ptr() = default;

  // Wrap an existing shared_ptr and register this instance
  explicit tracked_shared_ptr(std::shared_ptr<T> sp,
                              std::string tag = "hold",
                              src_loc loc = src_loc::current())
    : sp_(std::move(sp)), key_(sp_)
  {
    if (sp_) {
      tag_   = std::move(tag);
      token_ = Tracker::add(std::shared_ptr<void>(sp_), tag_, loc);
    }
  }

  // Factory that constructs the pointed-to object and tracks the hold
  template<class... Args>
  static tracked_shared_ptr make(std::string tag, src_loc loc, Args&&... args) {
    tracked_shared_ptr r{std::make_shared<T>(std::forward<Args>(args)...), tag, loc};
    return r;
  }

  // Copy = new tracked instance → new token
  tracked_shared_ptr(const tracked_shared_ptr& o, src_loc loc = src_loc::current())
    : sp_(o.sp_), key_(o.key_), tag_(o.tag_)
  {
    if (sp_) token_ = Tracker::add(std::shared_ptr<void>(sp_), tag_, loc);
  }

  tracked_shared_ptr& operator=(const tracked_shared_ptr& o) {
    if (this == &o) return *this;
    release();
    sp_  = o.sp_;
    key_ = o.key_;
    tag_ = o.tag_;
    if (sp_) token_ = Tracker::add(std::shared_ptr<void>(sp_), tag_, src_loc::current());
    return *this;
  }

  // Move = transfer the same instance (same token)
  tracked_shared_ptr(tracked_shared_ptr&& o) noexcept
    : sp_(std::move(o.sp_)), key_(std::move(o.key_)), tag_(std::move(o.tag_)), token_(o.token_) {
    o.token_ = 0;
  }

  tracked_shared_ptr& operator=(tracked_shared_ptr&& o) noexcept {
    if (this == &o) return *this;
    release();
    sp_     = std::move(o.sp_);
    key_    = std::move(o.key_);
    tag_    = std::move(o.tag_);
    token_  = o.token_;
    o.token_ = 0;
    return *this;
  }

  ~tracked_shared_ptr() { release(); }

  // Accessors (behave like shared_ptr)
  T* get() const noexcept { return sp_.get(); }
  T& operator*() const noexcept { return *sp_; }
  T* operator->() const noexcept { return sp_.get(); }
  explicit operator bool() const noexcept { return static_cast<bool>(sp_); }
  std::shared_ptr<T> const& underlying() const noexcept { return sp_; }
  std::shared_ptr<T>&       underlying()       noexcept { return sp_; }

private:
  void release() noexcept {
    if (token_) {
      Tracker::remove_hold(key_, token_);
      token_ = 0;
    }
  }

  std::shared_ptr<T> sp_;
  std::weak_ptr<void> key_;     // identifies the control block
  std::string tag_;
  std::size_t token_ = 0;       // this instance's handle in Tracker
};

// ---------------------- Example usage ----------------------------------------
/*
struct Foo { int x = 42; };

int main() {
  auto a = tracked_shared_ptr<Foo>::make("root");
  {
    tracked_shared_ptr<Foo> b = a;      // new tracked instance
    auto c = tracked_shared_ptr<Foo>(a.underlying(), "temp copy");
    Tracker::dump(a.underlying());      // shows 3 holds
  }                                      // 'b' and 'c' destroyed → records removed
  Tracker::dump(a.underlying());         // shows 1 hold ("root")
}
*/
