# Team Lark Webserver
## 1. Structure

We have 5 main folders for this webserver:
1. Config:
  The configuration files that the server use to process the requests
2. Docker:
  This folder has the base and main docker files to create the required images for the webserver
3. Files:
  Contains all the data files such as the jpg and zip files that the client can request
4. Src:
  The main code for server, session and other necessary components of the webserver is in this folder
5. Test:
  All unit tests that check the functionality of main files
  
The main process looks like this:
##### server_main.cc --> server.cc --> session.cc --> dispatcher.cc --> file_handler.cc or echo_handler.cc  

The webserver starts from the server.cc file. This file uses the config file to set the port number and 
after getting a request message from a client it starts a session by using the session.cc file. At first, 
the session file uses the dispatcher to find the required information and path from the request. After that we
call the file_handler.cc or echo_handler.cc to create the response depending on the request.

## 2. Build and Test
#### Build: 
Create a build directory and use cmake to build 
~~~
  mkdir build
  cd build
  cmake ..
  make
~~~
#### Unit Test:
This command will check all unit tests
~~~
  cd build
  make test
~~~
#### Integration Test:
~~~
  cd build
  ../tests/test.sh
~~~
#### Run The Server:
Run the server with this command. You can add a new config file to the config folder and run the server with that
~~~
  cd build
  ./bin/server ../config/static_config
~~~
#### Docker Build:
If you want you can build and run the server with the docker commands below.
Run this command for making the base image. (Note: After using these commands, when you change 
the code in any of the files you don't need to run the first command again)
~~~~
  docker build -f docker/base.Dockerfile -t lark:base .
~~~~

For creating the main image run this command (Note: run this again after each change)
~~~
  docker build -f docker/Dockerfile -t my_test .
~~~

For running the server use this command
~~~
 docker run --rm -p 127.0.0.1:80:80 --name my_run my_test:latest
~~~

For testing with different requests you can enter links similar to "http://localhost/static/hell.txt" in your browser

## 3. Add a Request Handler

To add a request handler, the handler must be derived from the RequestHandler class and implement its own
"handle_request" method that has the same signature as the following: 
~~~~
http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
~~~~

Once the handler class has been implemented, it needs to be incorporated into the Dispatcher class so that it
will get initialized and called.

The file containing the definition of the handler must be included in the dispatcher.cc file.
~~~
#include "echo_handler.h"
~~~
Then a line must be added in the find_path method of the dispatcher in the if else chain to initialize the 
handler in the following manner:
~~~~
else if (handler_type == "EchoHandler") {
    handlers_[path] = new EchoHandler(path, config);
    handler_info[path] = "EchoHandler";
}
~~~~
The handler must be initialized using a string variable called "path" and a NginxConfig& config variable called "config".
In addition, the handler type must be specified in the config file for the dispatcher to identify the handler and initialize it appropriately. An example of a config file format is shown in the next section.

## 4. Config File Format
~~~~
listen 80;
location /static/ FileHandler {
    root /bar;
}
~~~~
A port that the server can listen on must be specified using the keyword "listen" followed by the port number. After the port has been
declared, multiple handlers can be initialized using a similar format as shown above. A handler statement must start with the keyword 
"location", followed by a request path that can be optionally enclosed in quotes, followed by the handler type. In this case, the 
handler type is "FileHandler". The path after "location" is used to store the handler in a map within the Dispatcher class and will be called on if the path of the handler is the longest matching prefix to a given request. Within the statement block, any additional
initializion information can be added and the handler specified can parse this config object to properly initialize itself.