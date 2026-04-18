#
# (C) 2025 Vlad Troyanker
# This file is part of hasher-server. It demonstrates performance of a scalable TCP server
# and used to investigate comparable performance as provided by various language implementations
# of the same algorithm.
#

import asyncio, signal, sys
import hashlib
from concurrent.futures import ThreadPoolExecutor
import threading



class HashEngine:
    def __init__(self):
        pass

    def process_line(self, line: bytes) -> bytes:
        line = bytearray(line).removesuffix(b'\n')  # remove the newline character if present
#        print(f"Processing line: '{line.decode()}'")
        digest =  hashlib.md5(line).hexdigest()  # digest is a str
        return digest.encode() + b'\n'  # return bytes with newline


class ClientConnection:
    def __init__(self, reader, writer):
        self.reader = reader
        self.writer = writer
        self.addr = writer.get_extra_info("peername")
        self.engine = HashEngine()
        self._thread_id = None
        print(f"client {self.addr} connected")

    def run_thread(self):
        self._thread_id = threading.get_ident()
        print(f"client {self.addr} running on thread {self._thread_id}")
        asyncio.run(self.run())

    async def run(self):
        try:
            while data := await self.reader.readline():
                self.writer.write(self.engine.process_line(data))
                await self.writer.drain()
            print(f"client {self.addr} disconnected")
        except asyncio.CancelledError:
            print(f"connection {self.addr} aborted")
        finally:
            self.writer.close()
            await self.writer.wait_closed()


async def main():
    # Create thread pool with max 16 worker threads
    executor = ThreadPoolExecutor(max_workers=16)
    
    async def client_connected(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        client = ClientConnection(reader, writer)
        executor.submit(client.run_thread)

    server = await asyncio.start_server(client_connected, host="127.0.0.1", port=8000)
    addr = ", ".join(str(s.getsockname()) for s in server.sockets)
    print(f"listening on {addr}")

    loop = asyncio.get_running_loop()
    stop = asyncio.Event()

    def request_shutdown():
        print("\nshutting down…")
        stop.set()

    # POSIX signals (works on Linux/macOS)
    if sys.platform != "win32":
        for sig in (signal.SIGINT, signal.SIGTERM):
            loop.add_signal_handler(sig, request_shutdown)
    else:
        # Fallback for Windows: catch Ctrl-C
        loop.add_reader(sys.stdin, lambda: None)  # ensure loop wakes on console input

    async with server:
        try:
            # wait until Ctrl-C / SIGTERM
            await stop.wait()
        finally:
            executor.shutdown(wait=True)  # Wait for all threads to complete
            server.close()
            await server.wait_closed()

# run
if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
