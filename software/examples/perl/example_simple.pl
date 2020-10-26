#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletEnergyMonitor;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your Energy Monitor Bricklet

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $em = Tinkerforge::BrickletEnergyMonitor->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Get current energy data
my ($voltage, $current, $energy, $real_power, $apparent_power, $reactive_power,
    $power_factor, $frequency) = $em->get_energy_data();

print "Voltage: " . $voltage/100.0 . " V\n";
print "Current: " . $current/100.0 . " A\n";
print "Energy: " . $energy/100.0 . " Wh\n";
print "Real Power: " . $real_power/100.0 . " h\n";
print "Apparent Power: " . $apparent_power/100.0 . " VA\n";
print "Reactive Power: " . $reactive_power/100.0 . " var\n";
print "Power Factor: " . $power_factor/1000.0 . "\n";
print "Frequency: " . $frequency/100.0 . " Hz\n";

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
