#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "XYZ" # Change XYZ to the UID of your Energy Monitor Bricklet

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    em = BrickletEnergyMonitor(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Get current energy data
    voltage, current, energy, real_power, apparent_power, reactive_power, power_factor, \
      frequency = em.get_energy_data()

    print("Voltage: " + str(voltage/100.0) + " V")
    print("Current: " + str(current/100.0) + " A")
    print("Energy: " + str(energy/100.0) + " Wh")
    print("Real Power: " + str(real_power/100.0) + " h")
    print("Apparent Power: " + str(apparent_power/100.0) + " VA")
    print("Reactive Power: " + str(reactive_power/100.0) + " var")
    print("Power Factor: " + str(power_factor/1000.0))
    print("Frequency: " + str(frequency/100.0) + " Hz")

    input("Press key to exit\n") # Use raw_input() in Python 2
    ipcon.disconnect()
