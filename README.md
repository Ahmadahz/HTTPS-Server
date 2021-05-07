# Team Lark Webserver
## 1. Structure

We have 5 main folders for this webserver:
1. Config:
  The configuration files that the server use to process the requests
2. Docker:
  This file has the base and main docker files to create the required images for the webserver
3. Files:
  Contains all the data files such as the jpg and zip files that the client can request
4. Src:
  The main code for server, session and other necessary components for the webserver is in this folder
5. Test:
  All unit tests that check the functionality of main files
  
The webserver starts from the server.cc file. This file uses the config file to set the port number and 
after getting a request message from a client it starts a session by using the session.cc file. At first, 
the session file uses the handler to get the information of the request and after that it sends the 
information to echo or static handlers to create the response message and send it back to the client. 

## 2. Build and Test

For building, checking all unit tests and running the server you need to enter these commands: 

Run this command for making the base image.(Note: After using these commands, when you change 
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

For testing different requests you can enter links similar to "http://localhost/static/hell.txt" in your browser

## 3. Add a Request Handler


