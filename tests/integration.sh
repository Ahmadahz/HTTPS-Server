#!/bin/bash

pwd
EX_PATH="./bin/server"
RESPONSE_PATH="../tests"
CONFIG_PATH="../config/static_config"
FILE_PATH="../files/bar"

$EX_PATH $CONFIG_PATH & 
pid_server=$!
echo $pid_server

sleep 1
#Checks if server correctly echos 'hell.txt' message
#Expect Success
(printf '%s\r\n%s\r\n%s\r\n\r\n'            \
    "GET /echo/hell.txt HTTP/1.1"          \
    "Host: www.test.com"                    \
    "Connection: close"                     \
    | nc 35.197.104.232 80) > bin/echo_test

diff bin/echo_test bin/echo_response
echo -n "Test 1:"
if [[ $? -eq 0 ]]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

#Test should not return without \r\n\r\n at the end
#Wait for 1s timeout then check
response=$(printf '%s\r\n%s\r\n%s\r\n'  \
    "GET /echo HTTP/1.1"                \
    "Host: www.test.com"                \
    "Connection: close"                 \
    | nc 127.0.0.1 80) &
pid=$!
sleep 1 && kill -9 $pid
echo -n "Test 2:"
if [[ $response = "" ]]; then 
    echo "SUCCESS"; 
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi


(printf '%s\r\n%s\r\n%s\r\n\r\n'            \
    "GET /static/hell.txt HTTP/1.1"          \
    "Host: www.test.com"                    \
    "Connection: close"                     \
    | nc 35.197.104.232 80) > bin/txt_test

# Check if txt file response isn't corrupted

diff bin/txt_test bin/txt_response
echo -n "Test 3:"
if [[ $? -eq 0 ]]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

kill -9 $pid_server
exit 0