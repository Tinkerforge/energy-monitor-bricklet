<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletEnergyMonitor.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletEnergyMonitor;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Energy Monitor Bricklet

// Callback function for energy data callback
function cb_energyData($voltage, $current, $energy, $real_power, $apparent_power,
                       $reactive_power, $power_factor, $frequency)
{
    echo "Voltage: " . $voltage/100.0 . " V\n";
    echo "Current: " . $current/100.0 . " A\n";
    echo "Energy: " . $energy/100.0 . " Wh\n";
    echo "Real Power: " . $real_power/100.0 . " h\n";
    echo "Apparent Power: " . $apparent_power/100.0 . " VA\n";
    echo "Reactive Power: " . $reactive_power/100.0 . " var\n";
    echo "Power Factor: " . $power_factor/1000.0 . "\n";
    echo "Frequency: " . $frequency/100.0 . " Hz\n";
    echo "\n";
}

$ipcon = new IPConnection(); // Create IP connection
$em = new BrickletEnergyMonitor(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Register energy data callback to function cb_energyData
$em->registerCallback(BrickletEnergyMonitor::CALLBACK_ENERGY_DATA, 'cb_energyData');

// Set period for energy data callback to 1s (1000ms)
$em->setEnergyDataCallbackConfiguration(1000, FALSE);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
