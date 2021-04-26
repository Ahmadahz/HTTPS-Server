pwd
EX_PATH="./bin/server"
RESPONSE_PATH="../tests"
CONFIG_PATH="../config/static_config"
FILE_PATH="../files/bar"

$EX_PATH $CONFIG_PATH & 
pid_server=$!
echo $pid_server

sleep 1
# TODO modify tests to work in docker

#Check if text file response works correctly
(printf '%s\r\n%s\r\n%s\r\n\r\n'             \
    "GET /static/hell.txt HTTP/1.1"          \
    "Host: www.test.com"                    \
    "Connection: close"                     \
    | nc 35.197.104.232 80) > bin/test_txt

echo -n "Test 3:"
diff bin/txt_response bin/test_txt

if [ $? -eq 0 ]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

#Check if jpg file response works correctly
(printf '%s\r\n%s\r\n%s\r\n\r\n'             \
    "GET /static/new.jpg HTTP/1.1"          \
    "Host: www.test.com"                    \
    "Connection: close"                     \
    | nc 35.197.104.232 80) > bin/test_jpg

echo -n "Test 4:"
diff bin/jpg_response bin/test_jpg

if [ $? -eq 0 ]; then
    echo "SUCCESS";
else 
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
fi

kill -9 $pid_server
exit 0