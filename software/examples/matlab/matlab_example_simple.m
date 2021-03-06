function matlab_example_simple()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletEnergyMonitor;

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your Energy Monitor Bricklet

    ipcon = IPConnection(); % Create IP connection
    em = handle(BrickletEnergyMonitor(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Get current energy data
    energyData = em.getEnergyData();

    fprintf('Voltage: %g V\n', energyData.voltage/100.0);
    fprintf('Current: %g A\n', energyData.current/100.0);
    fprintf('Energy: %g Wh\n', energyData.energy/100.0);
    fprintf('Real Power: %g h\n', energyData.realPower/100.0);
    fprintf('Apparent Power: %g VA\n', energyData.apparentPower/100.0);
    fprintf('Reactive Power: %g var\n', energyData.reactivePower/100.0);
    fprintf('Power Factor: %g\n', energyData.powerFactor/1000.0);
    fprintf('Frequency: %g Hz\n', energyData.frequency/100.0);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end
