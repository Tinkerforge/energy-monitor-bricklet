function octave_example_simple()
    more off;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your Energy Monitor Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    em = javaObject("com.tinkerforge.BrickletEnergyMonitor", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Get current Energy Data
    energyData = em.getEnergyData();

    fprintf("Voltage: %g V\n", energyData.voltage/100.0);
    fprintf("Current: %g A\n", energyData.current/100.0);
    fprintf("Energy: %g Wh\n", energyData.energy/100.0);
    fprintf("Real Power: %g h\n", energyData.realPower/100.0);
    fprintf("Apparent Power: %g VA\n", energyData.apparentPower/100.0);
    fprintf("Reactive Power: %g var\n", energyData.reactivePower/100.0);
    fprintf("Power Factor: %g\n", energyData.powerFactor/1000.0);
    fprintf("Frequency: %g Hz\n", energyData.frequency/100.0);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end
