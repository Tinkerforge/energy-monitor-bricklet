#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Energy Monitor Bricklet

# Get current Energy Data
tinkerforge call energy-monitor-bricklet $uid get-energy-data
