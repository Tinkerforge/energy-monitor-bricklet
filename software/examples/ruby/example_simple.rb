#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_energy_monitor'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your Energy Monitor Bricklet

ipcon = IPConnection.new # Create IP connection
em = BrickletEnergyMonitor.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Get current energy data as [voltage, current, energy, real_power, apparent_power,
#                             reactive_power, power_factor, frequency]
energy_data = em.get_energy_data

puts "Voltage: #{energy_data[0]/100.0} V"
puts "Current: #{energy_data[1]/100.0} A"
puts "Energy: #{energy_data[2]/100.0} Wh"
puts "Real Power: #{energy_data[3]/100.0} h"
puts "Apparent Power: #{energy_data[4]/100.0} VA"
puts "Reactive Power: #{energy_data[5]/100.0} var"
puts "Power Factor: #{energy_data[6]/1000.0}"
puts "Frequency: #{energy_data[7]/100.0} Hz"

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
