#!/bin/bash
if [[ $1 == "help" || $# != 3 ]]
then
	echo -e "Usage: ./stress_test_start.sh [INSERT|GET] [TEST_COUNT] [NUM_THREADS]"
else
	./bin/stress $1 $2 $3
fi
