# hasher-server

## A  highly scalable hash caclulating server in C++. Demonstates a vertically scalable server design.

# The Architecture

The architecture is built around three elements: main thread that listens and accepts new client connections. 
Handling client connections is farmed out to thread pool #1. So it can service up to N connections simultaneously.
Executing hash computations is performed on a another thread pool (pool #2). All clients collectively send their 
computational load to pool #2. Note that each client's computations are executed on different threads but 
sequientially to preserve correctness and order of the result stream. As a bonus **there are no mutexes**
 or any other synchronisation primitives used.

# Dependencies

1. Depends on asio library for asyncronous primitive and base networking. Source included.
2. Depends on openssl libcrypto. It is expected to be installed in an default accessible path.

# Build

Run once to prepare dependencies

> make deps

To build on ubuntu 20 run:

> make

# TODO
1. Handle any streamable hash function supported by openssl
2. Support IPv6
