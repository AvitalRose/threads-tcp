#!/bin/bash

# My first script
#client compile, should not write anything
gcc client_threads.c -o client -pthread>results_testing_client_no_server.txt

#testing client
#1. testing when not adding in ip to connect to- should say: "Usage: ./client <ip of server> "
# shellcheck disable=SC2129
# shellcheck disable=SC2028
echo "Now testing without socket ip to connect to">>results_testing_client_no_server.txt
./client >>results_testing_client_no_server.txt

#2. testing when server is not connected- should say: "Error: Connect Failed "
#./client "127.0.0.1" >>results_testing_client_no_serverresults_testing_client_no_server.txt
# shellcheck disable=SC2028
echo "Now testing connecting with server not connected">>results_testing_client_no_server.txt
# shellcheck disable=SC2129
./client "127.0.0.1">>results_testing_client_no_server.txt <<EOF
"client 1"
EOF

echo "Now testing name length">>results_testing_client_no_server.txt
./client "127.0.0.1">>results_testing_client_no_server.txt <<EO
"way to long name that should get a thrown out right away to long"
EO







