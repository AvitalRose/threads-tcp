#!/bin/bash

echo "Testing with exit from input line">>results_exit.txt
gcc client_threads.c -o client -pthread
./client "127.0.0.1" >>results_exit.txt<<EOF
client name
message one sent
exit
EOF
