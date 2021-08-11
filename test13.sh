#!/bin/bash

# My first script
gcc t13_2.c -o client
for N in {1..50}
do
  ./client "127.0.0.1"
done
echo "done"
