#!/bin/bash

# The bat file checks the first passed from the caller scope string argument
# If the argument length is greater than 0 the argument is used in path to the coderdbc.exe
# Examples:

# ./run-test-gen.sh release 
# build/release/coderdbc

# ./run-test-gen.sh debug 
# build/debug/coderdbc

# ATTENTION: this script is not tested in Linux yet!!!

# Check if an argument is provided
if [ -z "$1" ]; then
    # If no argument is provided, set the path directly
    CMD_PATH="build/coderdbc"
else
    # If an argument is provided, use it to construct the path
    CMD_PATH="build/$1/coderdbc"
fi

# Output the combined path for debugging
echo "Combined path: $CMD_PATH"

# Run the program with the constructed path and other options
"$CMD_PATH" -dbc test/testdb.dbc -out test/gencode -drvname testdb -nodeutils -rw
