#!/bin/bash

if command -v nc >/dev/null 2>&1; then
    NETCAT="nc"
elif command -v netcat >/dev/null 2>&1; then
    NETCAT="netcat"
else
    echo "Error: nc or netcat not found." >&2
    exit 1
fi

ADDRESS=${ADDRESS:-localhost}
PORT=${PORT:-8000}  # Use PORT from environment or default to 8000

for i in {1..100}; do
    $NETCAT $ADDRESS $PORT < big.txt > "result${i}"
done