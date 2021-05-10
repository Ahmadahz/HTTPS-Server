#!/bin/bash
# rebuilds docker file
docker build -f docker/base.Dockerfile -t lark:base .
docker build -f docker/Dockerfile -t my_test . 
docker build -f docker/coverage.Dockerfile -t my_coverage . 
docker run --rm -p 127.0.0.1:80:80 --name my_run my_test:latest

#To stop it later: docker container stop my_run