function octave_example_callback()
    more off;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your Energy Monitor Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    em = javaObject("com.tinkerforge.BrickletEnergyMonitor", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register Energy Data callback to function cb_energy_data
    em.addEnergyDataCallback(@cb_energy_data);

    % Set period for Energy Data callback to 1s (1000ms)
    em.setEnergyDataCallbackConfiguration(1000, false);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

% Callback function for Energy Data callback
function cb_energy_data(e)
    fprintf("Voltage: %g V\n", e.voltage/100.0);
    fprintf("Current: %g A\n", e.current/100.0);
    fprintf("Energy: %g Wh\n", e.energy/100.0);
    fprintf("Real Power: %g h\n", e.realPower/100.0);
    fprintf("Apparent Power: %g VA\n", e.apparentPower/100.0);
    fprintf("Reactive Power: %g VAR\n", e.reactivePower/100.0);
    fprintf("Power Factor: %g\n", e.powerFactor/1000.0);
    fprintf("Frequency: %g Hz\n", e.frequecy/100.0);
    fprintf("\n");
end
