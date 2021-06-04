#!/bin/bash

pwd
EX_PATH="./bin/server"

#Make a temp config file
echo "
listen 80;

ssl-port 443;
private_key_root localhost-key.pem;
public_key_root localhost.pem;

location /echo/ EchoHandler {
}

location /proxy/ ProxyHandler {
    host http://example.com/;
    port 80;
}
" > temp_config_file

#Start the server
$EX_PATH temp_config_file & 
pid_server=$!
echo $pid_server

sleep 1

#Checks if https server correctly echos 'hell' message
curl --cacert localhost.pem https://localhost/echo  > test_response

echo -n "Test 1:"

DIFF=$(diff ../tests/https_echo test_response)
EXIT_STATUS=$?

if [ "$EXIT_STATUS" -eq 0 ]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

sleep 1

kill -9 $pid_server
exit 0