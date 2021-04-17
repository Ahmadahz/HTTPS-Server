#!/bin/bash

pwd
EX_PATH="./bin/server"
RESPONSE_PATH="../tests"
CONFIG_PATH="../config/server_config"

$EX_PATH $CONFIG_PATH & 
pid_server=$!
echo $pid_server

sleep 1
#Checks if server correctly echoe 'hello' message
#Expect Success
(printf '%s\r\n\r\n' 'hello' | nc 35.197.104.232 80) > test_hello
echo -n "Test 1:..."
diff test_hello ${RESPONSE_PATH}/hello_response 

if [[ $? -eq 0 ]]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

response=$(printf '%s\r\n%s\r\n%s\r\n'  \
    "GET / HTTP/1.1"                    \
    "Host: www.test.com"                \
    "Connection: close"                 \
    | nc 35.197.104.232 80) &
pid=$!
sleep 1 && kill -9 $pid
echo -n "Test 2:..."
if [[ $response = "" ]]; then 
    echo "SUCCESS"; 
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi
kill -9 $pid_server
exit 0