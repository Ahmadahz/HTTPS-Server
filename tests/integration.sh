#!/bin/bash

pwd
EX_PATH="./bin/server"
#RESPONSE_PATH="../tests"
#CONFIG_PATH="../config/static_config"
#FILE_PATH="../files/bar"

#Make a temp config file
echo "
listen 80;

ssl-port 443;
private_key_root domain.key;
public_key_root domain_certs.pem;

location /echo/ EchoHandler {
}

location /static/ FileHandler {
    root /bar;
}

location /status/ StatusHandler {
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

# Checks if the server handles wrong requests
response=$(printf '%s\r\n%s\r\n%s\r\n\r\n'            \
			    "GET HTTP/1.1"                        \
			   "Host: www.test.com"                   \
               "Connection: close"                    \
               | nc 127.0.0.1 80)

echo $response > ../tests/echo_test_400

echo -n "Test 1:"

DIFF=$(diff ../tests/expected_echo_400 ../tests/echo_test_400)
EXIT_STATUS=$?

if [ "$EXIT_STATUS" -eq 0 ]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

#Checks if server correctly echos 'hell' message
curl -i -s 127.0.0.1:80/echo/hell > test_response

echo -n "Test 2:"

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
echo -n "Test 3:"
if [ "$response2" = "" ]; then 
    echo "SUCCESS"; 
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

# proxy handler test
echo -n "Test 4:"
curl -s http://www.example.com/ > example.html
curl -s 127.0.0.1:80/proxy/ > test_response.html

DIFF=$(diff example.html test_response.html)
EXIT_STATUS=$?
rm example.html
rm test_response.html

if [ "$EXIT_STATUS" -eq 0 ]; then
    echo "Proxy Test SUCCESS";
else 
    echo "Proxy Test FAIL"; 
    kill -9 $pid_server
    exit 1;
fi


sleep 1

kill -9 $pid_server
exit 0