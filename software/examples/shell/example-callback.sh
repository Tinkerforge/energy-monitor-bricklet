#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Energy Monitor Bricklet

# Handle incoming energy data callbacks
tinkerforge dispatch energy-monitor-bricklet $uid energy-data &

# Set period for energy data callback to 1s (1000ms)
tinkerforge call energy-monitor-bricklet $uid set-energy-data-callback-configuration 1000 false

echo "Press key to exit"; read dummy

kill -- -$$ # Stop callback dispatch in background
