#!/bin/bash

pwd
EX_PATH="./bin/server"

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
" > temp_config_file

#Start the server
$EX_PATH temp_config_file & 
pid_server=$!
echo $pid_server

sleep 1
curl -s 127.0.0.1:80/echo/hell
curl -s 127.0.0.1:80/static/hell.txt
curl -s 127.0.0.1:80/static
curl -s 127.0.0.1:80/status/ > test_status.html
curl -s 127.0.0.1:80/status/ > status_out.html

DIFF=$(diff status_out.html test_status.html)
EXIT_STATUS=$?

#check that status page gets updated
if [ "$EXIT_STATUS" -eq 0 ]; then
    echo "FAIL"; 
    kill -9 $pid_server
    exit 1;
else 
    echo "SUCCESS";
fi

kill -9 $pid_server
exit 0