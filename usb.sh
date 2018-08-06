#!/bin/bash

device=/dev/ttyUSB0

stty -F $device 9600
stty -F $device raw
stty -F $device -echo
stty -F $device crtscts
stty -F $device -ixon
stty -F $device -parity

./ulm /tmp < $device >$device
