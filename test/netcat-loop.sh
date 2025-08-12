#!/bin/bash

if command -v nc >/dev/null 2>&1; then
    NETCAT="nc"
elif command -v netcat >/dev/null 2>&1; then
    NETCAT="netcat"
else
    echo "Error: nc or netcat not found." >&2
    exit 1
fi

for i in {1..50}; do
    $NETCAT -w 1 localhost 8000 < big.txt > "result${i}"
done