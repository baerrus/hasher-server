#!/bin/bash

if command -v nc >/dev/null 2>&1; then
    NETCAT="nc"
elif command -v netcat >/dev/null 2>&1; then
    NETCAT="netcat"
else
    echo "Error: nc or netcat not found." >&2
    exit 1
fi

$NETCAT -w 1 localhost 8000 < big.txt > result1 &
$NETCAT -w 1 localhost 8000 < big.txt > result2 &
$NETCAT -w 1 localhost 8000 < big.txt > result3 &
$NETCAT -w 1 localhost 8000 < big.txt > result4 &
$NETCAT -w 1 localhost 8000 < big.txt > result5 &
$NETCAT -w 1 localhost 8000 < big.txt > result6 &
$NETCAT -w 1 localhost 8000 < big.txt > result7 &
$NETCAT -w 1 localhost 8000 < big.txt > result8 &
$NETCAT -w 1 localhost 8000 < big.txt > result9 &
$NETCAT -w 1 localhost 8000 < big.txt > result10 &
$NETCAT -w 1 localhost 8000 < big.txt > result11 &
$NETCAT -w 2 localhost 8000 < big.txt > result12 &
$NETCAT -w 2 localhost 8000 < big.txt > result13 &
$NETCAT -w 2 localhost 8000 < big.txt > result14 &
$NETCAT -w 2 localhost 8000 < big.txt > result15 &
$NETCAT -w 2 localhost 8000 < big.txt > result15 &
$NETCAT -w 2 localhost 8000 < big.txt > result17 &
$NETCAT -w 2 localhost 8000 < big.txt > result18 &
$NETCAT -w 2 localhost 8000 < big.txt > result19 &
$NETCAT -w 2 localhost 8000 < big.txt > result20 &
$NETCAT -w 2 localhost 8000 < big.txt > result21 &
$NETCAT -w 2 localhost 8000 < big.txt > result22 &
