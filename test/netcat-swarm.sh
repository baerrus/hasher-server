#!/bin/bash

if command -v nc >/dev/null 2>&1; then
    NETCAT="nc"
elif command -v netcat >/dev/null 2>&1; then
    NETCAT="netcat"
else
    echo "Error: nc or netcat not found." >&2
    exit 1
fi

ADDRESS="localhost"
PORT=8001
for i in {1..50}; do
    $NETCAT -w 1 $ADDRESS $PORT < big.txt > "result${i}" &
done
# Wait for all background processes to finish
wait
echo "** Finished\n"