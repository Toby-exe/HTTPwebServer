#!/bin/bash

# Start the timer
start=$(date +%s.%N)

# Run the 50 clients in parallel and wait for them to finish
for i in {1..50}
do
    ./client_program 
done &


# Wait for both clients to finish
wait

# Stop the timer
end=$(date +%s.%N)

# Calculate the time taken
time=$(echo "$end - $start" | bc)

echo "Total time taken: $time seconds"
