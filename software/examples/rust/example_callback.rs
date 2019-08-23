use std::{error::Error, io, thread};
use tinkerforge::{energy_monitor_bricklet::*, ip_connection::IpConnection};

const HOST: &str = "localhost";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Energy Monitor Bricklet.

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection.
    let em = EnergyMonitorBricklet::new(UID, &ipcon); // Create device object.

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd.
                                          // Don't use device before ipcon is connected.

    let energy_data_receiver = em.get_energy_data_callback_receiver();

    // Spawn thread to handle received callback messages.
    // This thread ends when the `em` object
    // is dropped, so there is no need for manual cleanup.
    thread::spawn(move || {
        for energy_data in energy_data_receiver {
            println!("Voltage: {} V", energy_data.voltage as f32 / 100.0);
            println!("Current: {} A", energy_data.current as f32 / 100.0);
            println!("Energy: {} Wh", energy_data.energy as f32 / 100.0);
            println!("Real Power: {} h", energy_data.real_power as f32 / 100.0);
            println!("Apparent Power: {} VA", energy_data.apparent_power as f32 / 100.0);
            println!("Reactive Power: {} VAR", energy_data.reactive_power as f32 / 100.0);
            println!("Power Factor: {}", energy_data.power_factor as f32 / 1000.0);
            println!("Frequency: {} Hz", energy_data.frequency as f32 / 100.0);
            println!();
        }
    });

    // Set period for Energy Data callback to 1s (1000ms).
    em.set_energy_data_callback_configuration(1000, false);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
