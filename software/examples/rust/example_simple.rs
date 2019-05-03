use std::{error::Error, io};

use tinkerforge::{energy_monitor_bricklet::*, ip_connection::IpConnection};

const HOST: &str = "localhost";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Energy Monitor Bricklet.

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection.
    let em = EnergyMonitorBricklet::new(UID, &ipcon); // Create device object.

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd.
                                          // Don't use device before ipcon is connected.

    // Get current Energy Data.
    let energy_data = em.get_energy_data().recv()?;

    println!("Voltage: {} V", energy_data.voltage as f32 / 100.0);
    println!("Current: {} A", energy_data.current as f32 / 100.0);
    println!("Energy: {} Wh", energy_data.energy as f32 / 100.0);
    println!("Real Power: {} h", energy_data.real_power as f32 / 100.0);
    println!("Apparent Power: {} VA", energy_data.apparent_power as f32 / 100.0);
    println!("Reactive Power: {} VAR", energy_data.reactive_power as f32 / 100.0);
    println!("Power Factor: {}", energy_data.power_factor as f32 / 1000.0);
    println!("Frequency: {} Hz", energy_data.frequecy as f32 / 100.0);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
