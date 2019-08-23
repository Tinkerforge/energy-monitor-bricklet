<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletEnergyMonitor.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletEnergyMonitor;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Energy Monitor Bricklet

$ipcon = new IPConnection(); // Create IP connection
$em = new BrickletEnergyMonitor(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Get current Energy Data
$energy_data = $em->getEnergyData();

echo "Voltage: " . $energy_data['voltage']/100.0 . " V\n";
echo "Current: " . $energy_data['current']/100.0 . " A\n";
echo "Energy: " . $energy_data['energy']/100.0 . " Wh\n";
echo "Real Power: " . $energy_data['real_power']/100.0 . " h\n";
echo "Apparent Power: " . $energy_data['apparent_power']/100.0 . " VA\n";
echo "Reactive Power: " . $energy_data['reactive_power']/100.0 . " VAR\n";
echo "Power Factor: " . $energy_data['power_factor']/1000.0 . "\n";
echo "Frequency: " . $energy_data['frequency']/100.0 . " Hz\n";

echo "Press key to exit\n";
fgetc(fopen('php://stdin', 'r'));
$ipcon->disconnect();

?>
