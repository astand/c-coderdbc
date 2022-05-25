#!/bin/bash

if [[ -z "$1" ]]; then
    CANDB=$(ls *.dbc | head -n 1)
    echo "CAN Database file not provided. Using \"${CANDB}\""
else
    CANDB="$1"
fi

# ./build/coderdbc -dbc ${CANDB} -out generated -drvname CANMessages -canstruct "../main/dbc_to_twai.h" -usestruct -nodeutils -rw -rxcb -txcb
./build/coderdbc -dbc ${CANDB} -out generated -drvname CANMessages -canstruct "driver/twai.h" -defstruct "typedef twai_message_t __CoderDbcCanFrame_t__;" -usestruct -nodeutils -rw -rxcb -txcb
