FROM lark:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project
COPY tests/keys build_coverage
WORKDIR build_coverage

RUN pwd
RUN cmake ..
RUN make
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage