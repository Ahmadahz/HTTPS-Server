#!/bin/bash

pwd
EX_PATH="./bin/server"

# Make a temp config file
echo "
listen 80;

ssl-port 443;
private_key_root domain.key;
public_key_root domain_certs.pem;

location /echo/ EchoHandler {

}

location /sleep/ SleepHandler {

}
" > temp_config_file

# Start the server
$EX_PATH temp_config_file & 
pid_server=$!
echo $pid_server

block() {
    SECONDS=0
    curl -s 127.0.0.1:80/sleep > /dev/null
    echo "$SECONDS"
}

delayed_echo() {
    SECONDS=0
    sleep 1
    curl -s 127.0.0.1:80/echo > /dev/null
    echo "$SECONDS"
}

sleep 1

# Run both requests in parallel and output the times to files
block >> block_time &
EXIT_STATUS_1=$?
delayed_echo >> echo_time &
EXIT_STATUS_2=$?
sleep 6

# Get time results
block_time=$(cat block_time)
echo_time=$(cat echo_time)

rm block_time echo_time

if [[ "$EXIT_STATUS_1" -ne 0 || "$EXIT_STATUS_2" -ne 0 ]]; then
    echo "Fail: Server did not respond gracefully."; 
    kill -9 $pid_server
    exit 1;
else
    echo "Blocking duration: $block_time"
    echo "Echo duration: $echo_time"

    if [[ $echo_time -lt $block_time && $echo_time -lt 2 ]]; then
        echo "SUCCESS"
    else
        echo "Fail: Echo took too long"
        kill -9 $pid_server
        exit 1
    fi
fi

rm temp_config_file
kill -9 $pid_server
exit 0
