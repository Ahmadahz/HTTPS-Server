# #!/bin/bash

# pwd
# EX_PATH="./bin/server"
# RESPONSE_PATH="../tests"
# CONFIG_PATH="../config/static_config"
# FILE_PATH="../files/bar"

# $EX_PATH $CONFIG_PATH & 
# pid_server=$!
# echo $pid_server

# sleep 1
# #Checks if server correctly echos 'hello' message
# #Expect Success
# (printf '%s\r\n\r\n' 'hello' | nc 127.0.0.1 80) > bin/test_hello
# echo -n "Test 1:"
# diff bin/test_hello ${RESPONSE_PATH}/hello_response 

# if [[ $? -eq 0 ]]; then
#     echo "SUCCESS";
# else 
#     echo "FAIL"; 
#     kill -9 $pid_server
#     exit 1;
# fi

# #Test should not return without \r\n\r\n at the end
# #Wait for 1s timeout then check
# response=$(printf '%s\r\n%s\r\n%s\r\n'  \
#     "GET /echo HTTP/1.1"                \
#     "Host: www.test.com"                \
#     "Connection: close"                 \
#     | nc 127.0.0.1 80) &
# pid=$!
# sleep 1 && kill -9 $pid
# echo -n "Test 2:"
# if [[ $response = "" ]]; then 
#     echo "SUCCESS"; 
# else 
#     echo "FAIL"; 
#     kill -9 $pid_server
#     exit 1;
# fi

# Check if jpg file response isn't corrupted
# (printf '%s\r\n%s\r\n%s\r\n\r\n'            \
#     "GET /static/new.jpg HTTP/1.1"          \
#     "Host: www.test.com"                    \
#     "Connection: close"                     \
#     | nc 127.0.0.1 8080) 

# kill -9 $pid_server
# exit 0