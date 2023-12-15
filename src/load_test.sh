#!/bin/bash

# Usage: ./test.sh <url> <number_of_requests>

url=$1
n=$2
total_time=0

for ((i=1;i<=n;i++)); do
  time=$(curl -s -o /dev/null -w "%{time_total}" $url)
  total_time=$(echo $total_time+$time | bc)
done

average_time=$(echo "scale=4; $total_time / $n" | bc)
echo "Average response time for $n requests to $url : $average_time seconds"
