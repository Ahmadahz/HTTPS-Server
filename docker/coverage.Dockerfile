FROM lark:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

RUN pwd
RUN cmake ..
RUN make
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage