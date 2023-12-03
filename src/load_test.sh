#!/bin/bash

# Define the URL of the server
URL="http://10.65.255.109:7078/"

# Define the duration of the test in seconds
DURATION=60

# Define the number of requests per second
TPS=1000

# Calculate the end time
end=$((SECONDS+$DURATION))

# Start the load test
while [ $SECONDS -lt $end ]; do
  # Send TPS requests in parallel
  for ((i=1;i<=$TPS;i++)); do
    # Use curl to send a GET request and append the response time to a log file
    curl -X GET $URL -o /dev/null -s -w '%{time_total}\n' >> response-times.log &
  done
  # Wait for one second
  sleep 1
done

# Wait for all requests to finish
wait

# Print a message
echo "Load test has been completed"
