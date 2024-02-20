# hasher-server

### A highly scalable MD5 hash caclulating server in C++. Demonstates a vertically scalable network server design.  Accepts line delimited string inputs and returns hex-encoded MD5 hash of each string.

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
 approach is that dormant clients (clients sending little to no data) would consume as much valuable server resource
 as clients that send a lot of data. An advantage is maintaining a stable sequential order of inputs vs output. The
 drawbacks vs advantages of the main approach that was implemented are the exact reverse.

# Dependencies

1. Depends on asio library for asyncronous primitive and base networking. Source included.
2. Depends on openssl libcrypto. It is expected to be installed in an default accessible path.

# Build / Run

Run once to prepare dependencies

`> make deps`

To build on ubuntu 20 run:

`> make`

To launch the server. By default the server will listen on port 8000 of every IPv4 interface on the host.

`>./hasher-server`

To launch with non-default config:

`> HASHER_SERVER_COMPUTE_CAP=1 HASHER_SERVER_PORT=6000 ./hasher-server`

To run unit tests. The server is expected to be found on localhost:8000

`>cd test; python -m unittest oneshot_tests.py`

# Testing

To run a load test of ~ 128K lines through a server

`> netcat localhost 8000 < test/big.txt > test/result`

Stop netcat and verify the number of lines in result file is the same as in big.txt file:

`> wc -l test/big.txt` 
128457 test/big.txt
`> wc -l test/result`
128457 test/result`

# TODO
1. Handle any streamable hash function supported by openssl, not just MD5
2. Support IPv6
3. Better logging
