### Build/test container ###
# Define builder stage
FROM lark:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:focal as deploy

# TODO(ahmadhz): Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/server .
# Copy configuration file output to "." (Was required from the instructions)
COPY --from=builder /usr/src/project/config/static_config .
#Copy static data files
COPY --from=builder /usr/src/project/files .
# TODO(ahmadhz): Expose some port(s)
# Note: Add more ports if it is required
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# TODO(ahmadhz): Update with real server name 
# Note: The entry point cannot be changed
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
# TODO(ahmadhz): Update with real server args
# Note: The argument can be overwritten
# Note: We can add more arguments later
CMD ["static_config"]