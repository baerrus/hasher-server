# hasher-server

## A  highly scalable MD5 hash caclulating server in C++. Demonstates a vertically scalable network server design.

# The Architecture

The architecture is built around three elements: main thread that listens and accepts new client connections. 
Handling client connections is farmed out to thread pool #1. So it can service up to N connections simultaneously.
Executing hash computations is performed on a another thread pool (pool #2). All clients collectively send their 
computational load to pool #2. Note that each client's computations are executed on different threads but 
sequientially to preserve correctness and order of the result stream. As a bonus **there are no mutexes**
 or any other synchronisation primitives used (on top of asio elements). Both capacity pools are configurable
 via environment variables.

 ## Alternative Design

 A solid alternative would be to use a dedicated compute thread per each client connection. A drawback of this 
 approach is that dormant clients (clients sendig little to no data) would consume as much valuable server resource
 as clients that send a lot of data. An advantage is maintaining a stable sequential order of inputs vs output. The
 drawbacks vs advantages of the main approach that was implemented are the exact reverse.

# Dependencies

1. Depends on asio library for asyncronous primitive and base networking. Source included.
2. Depends on openssl libcrypto. It is expected to be installed in an default accessible path.

# Build

Run once to prepare dependencies

`> make deps`

To build on ubuntu 20 run:

`> make`

# TODO
1. Handle any streamable hash function supported by openssl, not just MD5
2. Support IPv6
