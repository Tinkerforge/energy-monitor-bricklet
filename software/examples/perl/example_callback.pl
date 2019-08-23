#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletEnergyMonitor;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your Energy Monitor Bricklet

# Callback subroutine for Energy Data callback
sub cb_energy_data
{
    my ($voltage, $current, $energy, $real_power, $apparent_power, $reactive_power,
        $power_factor, $frequency) = @_;

    print "Voltage: " . $voltage/100.0 . " V\n";
    print "Current: " . $current/100.0 . " A\n";
    print "Energy: " . $energy/100.0 . " Wh\n";
    print "Real Power: " . $real_power/100.0 . " h\n";
    print "Apparent Power: " . $apparent_power/100.0 . " VA\n";
    print "Reactive Power: " . $reactive_power/100.0 . " VAR\n";
    print "Power Factor: " . $power_factor/1000.0 . "\n";
    print "Frequency: " . $frequency/100.0 . " Hz\n";
    print "\n";
}

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $em = Tinkerforge::BrickletEnergyMonitor->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Register Energy Data callback to subroutine cb_energy_data
$em->register_callback($em->CALLBACK_ENERGY_DATA, 'cb_energy_data');

# Set period for Energy Data callback to 1s (1000ms)
$em->set_energy_data_callback_configuration(1000, 0);

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
