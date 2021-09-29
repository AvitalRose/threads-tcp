#!/bin/bash

echo "starting tests">results.txt
# shellcheck disable=SC2129
#echo "now testing server">>results.txt
##need to test - socket, bind, listen, accpet, send, recv, close
#
#gcc server4.c -o server -pthread>>results.txt
#
#echo "testing bind error">>server_results.txt
#./server &
## Get its PID
#PID=$!
##run another server
#./server 2>>server_results.txt
## Wait for 2 seconds
#sleep 2
## Kill it
#kill $PID

gcc client_threads.c -o client -pthread
gcc server4.c -o server -pthread

./server >>server_results.txt &
PID=$!
sleep 1

#testing regular
./client "127.0.0.1" < client1.txt >>results_client1.txt &
PID1=$!
./client "127.0.0.1" < client2.txt >>results_client2.txt &
PID2=$!
##run another server
## Wait for 2 seconds
sleep 20
## Kill it
kill $PID
kill $PID1
kill $PID2

#testing regular with one socket coming later
#echo "Now testing client3 coming later">>results.txt
#echo "Now testing client3 coming later">>server_results.txt
#./server >>server_results.txt &
#PID=$!
#./client "127.0.0.1" < client1.txt >>results_client1.txt &
#PID1=$!
#./client "127.0.0.1" < client2.txt >>results.txt &
#PID2=$!
#sleep 5
#./client "127.0.0.1" < client3.txt >>results.txt &
### Wait for 2 seconds
#sleep 20
### Kill it
#kill $PID
#kill $PID1
#kill $PID2
#testing too long message
./client "127.0.0.1" < message_to_long.txt >>results.txt
PID3=$!
sleep 5
kill PID3
