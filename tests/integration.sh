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
    exit 0; 
else 
    echo "FAIL"; 
    sudo kill -9 $pid_server
    exit 1;
fi

exit 0;