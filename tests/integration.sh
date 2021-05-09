#!/bin/bash

pwd
EX_PATH="./bin/server"
#RESPONSE_PATH="../tests"
#CONFIG_PATH="../config/static_config"
#FILE_PATH="../files/bar"

#Make a temp config file
echo "
listen 80;

location /static/ FileHandler {
    root /bar;
}

location /echo/ EchoHandler {
    empty /stmt;
}" > temp_config_file

#Start the server
$EX_PATH temp_config_file & 
pid_server=$!
echo $pid_server

sleep 1
#Checks if server correctly echos 'hell' message
#Expect Success
response=$(printf '%s\r\n%s\r\n%s\r\n\r\n'            \
			    "GET /echo/hell HTTP/1.1"         \
			   "Host: www.test.com"                   \
               "Connection: close"                    \
               | nc 35.197.104.232 80)

echo $response > echo_test

curl -i -s 35.197.104.232:80/echo/hell > test_response

echo -n "Test 1:"

DIFF=$(diff ../tests/expected_echo1 test_response)
EXIT_STATUS=$?

if [ "$EXIT_STATUS" -eq 0 ]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

#Test should not return without \r\n\r\n at the end
#Wait for 1s timeout then check
response2=$(printf '%s\r\n%s\r\n%s\r\n'  \
    "GET /echo HTTP/1.1"                \
    "Host: www.test.com"                \
    "Connection: close"                 \
    | nc 127.0.0.1 80) &
pid=$!
sleep 1 && kill -9 $pid
echo -n "Test 2:"
if [ "$response2" = "" ]; then 
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
    | nc 35.197.104.232 80) > txt_test

curl -i -s 35.197.104.232:80/static/hell.txt > txt_test
# Check if txt file response isn't corrupted

echo -n "Test 3:"
diff txt_test ../tests/txt_response_new
if [ "$?" -eq 0 ]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

kill -9 $pid_server
exit 0