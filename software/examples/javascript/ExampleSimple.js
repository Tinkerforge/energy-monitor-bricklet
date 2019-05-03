var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your Energy Monitor Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var em = new Tinkerforge.BrickletEnergyMonitor(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Get current Energy Data
        em.getEnergyData(
            function (voltage, current, energy, realPower, apparentPower, reactivePower, powerFactor, frequecy) {
                console.log('Voltage: ' + voltage/100.0 + ' V');
                console.log('Current: ' + current/100.0 + ' A');
                console.log('Energy: ' + energy/100.0 + ' Wh');
                console.log('Real Power: ' + realPower/100.0 + ' h');
                console.log('Apparent Power: ' + apparentPower/100.0 + ' VA');
                console.log('Reactive Power: ' + reactivePower/100.0 + ' VAR');
                console.log('Power Factor: ' + powerFactor/1000.0);
                console.log('Frequency: ' + frequecy/100.0 + ' Hz');
            },
            function (error) {
                console.log('Error: ' + error);
            }
        );
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
