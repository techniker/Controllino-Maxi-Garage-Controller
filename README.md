# Garage Door Controller for Controllino

The Garage Door Controller is a solution designed for controlling garage doors using the Controllino platform.
This project integrates hardware control with web-based interaction, allowing users to open, close, and stop their garage doors through a simple web interface. 
The system is built on the Arduino framework, utilizing libraries such as Bounce2 for button debouncing, Ethernet for network connectivity, and SPI for serial communication.

## Features

- **Button Control**: A physical button interface for manually triggering door operations.
- **Web Interface**: A simple HTTP server providing a web page to control the door remotely.
- **Status Indication**: LED indicators for showing the door's current status and operation.
- **Safety Features**: Utilizes an end switch to detect the door's fully closed position, enhancing safety and reliability.
- **Configurable Settings**: Includes adjustable parameters for door movement times, debounce intervals, and more.

## Hardware Requirements

- **Controllino**: The project is designed around the Controllino platform, which offers a robust solution for industrial applications.
- **Ethernet Module (ENC28J60)**: For network connectivity, enabling web interface access.
- **Button & LEDs**: For manual control and status indication.
- **Relays**: To manage the power and direction of the garage door motor.

## Software Dependencies

- **Arduino IDE**: For compiling and uploading the firmware to the Controllino.
- **Controllino Library**: Provides easy access to the Controllino's hardware features.
- **Bounce2 Library**: For debouncing the physical button input.
- **Ethernet Library**: To handle networking functionalities.

## Installation

1. **Prepare the Arduino Environment**: Ensure that the Arduino IDE is installed and properly configured for the Controllino device.
2. **Install Required Libraries**: Through the Arduino IDE, install the `Controllino`, `Bounce2`, and `Ethernet` libraries.
3. **Configure Network Settings**: Modify the `mac`, `ip`, `subnet`, `gateway`, and `dns` variables in the code to match your network configuration.
4. **Upload the Firmware**: Connect the Controllino to your computer, select the appropriate board and port in the Arduino IDE, and upload the code.
5. **Connect the Hardware**: Attach the button, LEDs, relays, and Ethernet module as defined in the code's pin configuration.

## Usage

- **Manual Control**: Use the attached button to manually open, close, or stop the garage door.
- **Web Interface**: Access the web interface through a browser by navigating to the Controllino's IP address.
- The interface provides buttons for opening, closing, and stopping the door. Additionally, it displays the current status of the door (open/closed).

Before deploying the system, ensure that the network settings (MAC address, IP address, subnet mask, gateway, and DNS) are correctly configured to match your local network environment!!

## Safety Considerations

This project involves controlling potentially large and powerful garage door mechanisms. Ensure all safety protocols are followed, including the use of end switches and testing all controls thoroughly before regular use.
