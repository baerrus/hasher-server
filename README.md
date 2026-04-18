# hasher-server

### This example server in C++ demonstates a vertically scalable network server design. Accepts newline delimited stream of strings and returns hex-encoded MD5 hash of each string. While computing hash is hardly ever useful by itself, the point is demonstrating a server that performs units of work in highly scalable manner. Hashing can be replaced with anything else.

# The Architecture

The architecture is built around three elements: main thread that listens and accepts new client connections. 
Handling client connections is farmed out to thread pool #1. So it can service up to N connections simultaneously (configurable).
Executing hash computations is performed on a another thread. The 'compute' thread is allocated per connection. This choice is preferred due to natural, sequential order in input data. A drawback of this 
 approach is that dormant clients (clients sending little to no data) would allocate as much valuable server resource as clients that send a lot of data. If the data had no order we could use the alternative described below (with expected higher performance). 

The server listening interface and port as well as capacity of both pools are configurable via environment variables.

Also added a basic, single thread python implementation to compare performance.

 ## Alternative Design
 All clients collectively send their computational load to pool #2. Note that each client's computations are executed on different thread thus spreading the load in the most efficient way. The pre-requisite to such approach is, of course, that the input data has no order to it.

# Dependencies

1. Depends on asio library for asyncronous primitive and base networking. Source included.
2. Depends on openssl libcrypto. It is expected to be installed in an default accessible path.

# Build / Run

## Bazel

`>bazel build haser_server`

## Legacy make
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

# Performance

I tested two load scenarios. First, client connections come sequencially. This is a rather impractical use case. However, it is useful as a baseline to compare python vs C++ implementations. In this scenario python and C++ implementations show the same performance. Note that C++ code must be compiled with full optimization.

The second scenario is a swarm of client connections hitting the server simultaneously. In that use case the C++ implementation shows ~ 100% better performance that sequencial processing in python.

# TODO
1. Handle any streamable hash function supported by openssl, not just MD5
2. Support IPv6
3. Better logging
