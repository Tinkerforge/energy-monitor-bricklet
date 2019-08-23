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

# Register Energy Data callback
em.register_callback(BrickletEnergyMonitor::CALLBACK_ENERGY_DATA) do |voltage, current,
                                                                      energy, real_power,
                                                                      apparent_power,
                                                                      reactive_power,
                                                                      power_factor,
                                                                      frequency|
  puts "Voltage: #{voltage/100.0} V"
  puts "Current: #{current/100.0} A"
  puts "Energy: #{energy/100.0} Wh"
  puts "Real Power: #{real_power/100.0} h"
  puts "Apparent Power: #{apparent_power/100.0} VA"
  puts "Reactive Power: #{reactive_power/100.0} VAR"
  puts "Power Factor: #{power_factor/1000.0}"
  puts "Frequency: #{frequency/100.0} Hz"
  puts ''
end

# Set period for Energy Data callback to 1s (1000ms)
em.set_energy_data_callback_configuration 1000, false

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
