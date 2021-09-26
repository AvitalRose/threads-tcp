#!/bin/bash

# My first script
gcc client_threads.c -o client -pthread

#testing client
#testing when not adding in ip to connect to- should say: "Usage: ./client <ip of server> "
./client >>results.txt

#testing when server is not connected- should say: "Error: Connect Failed "
./client "127.0.0.1" >>results.txt


#testing server
#need to test- socket, bind, listen, accept, recv, send;
gcc server4.c -o server

#testing bind fail- should say: "Socket bind error: : Address already in use"
./server
./server >>results.txt
#testing server connected
#for N in {1..50}
#do
#    ./client "127.0.0.1"
#    done
#    echo "done"