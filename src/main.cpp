// GarageController
// <tec ( att ) sixtopia.net>


#include <Arduino.h> //Include the Arduino Library
#include <Controllino.h> //Include the Controllino Library
#include <Bounce2.h> // Include the Bounce2 library
#include <Ethernet.h> //Include the ENC28J60 Eth Library
#include <SPI.h> //Include the SPI Library

// Garage door controller pins
const int buttonPin = CONTROLLINO_A9;        // Pin for the button
const int doorPowerPin = CONTROLLINO_RELAY_08;    // Pin to control the door power
const int doorDirectionPin = CONTROLLINO_RELAY_09; // Pin to control the door direction (up/down)
const int statusLightPin = CONTROLLINO_D11;   // Pin for the status light
const int statusLightPinLocked = CONTROLLINO_D10;   // Pin for status light1
const int endSwitchPin = CONTROLLINO_A8; // Pin for the end switch
const int endSwitchPowerOffDelay = 100; // Delay before turning off power pin when end switch is triggered (milliseconds)
const int doorMovementTime = 30000; // 60 seconds in milliseconds
const int blinkInterval = 300;      // Status light blink interval in milliseconds
const int debounceInterval = 50;   // Debounce interval in milliseconds
const int doorChangeDelay = 1000;  // 1-second delay before changing door state (milliseconds)

// Create a Bounce2 object for button debouncing
Bounce debouncer = Bounce();

int buttonState = LOW;
int lastButtonState = LOW;
int doorState = 0; // 0 for idle, 1 for opening, 2 for closing

unsigned long doorStartTime = 0;
boolean doorMoving = false;
boolean statusLightOn = false;
unsigned long lastBlinkTime = 0;

int endSwitchState = LOW; // The end switch is initially in the fully closed state
int lastEndSwitchState = HIGH;
unsigned long endSwitchTriggerTime = 0;
unsigned long lastDoorStateChangeTime = 0; // Track the time when the door state was last changed
bool isDoorFullyClosed = false; // Track if door is fully closed by endswitch state

// Network settings for Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xED };
IPAddress ip(10, 100, 0, 55);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(10, 100, 0, 1); 
IPAddress dns(10, 100, 0, 1);
EthernetServer server(80);

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(doorPowerPin, OUTPUT);
  pinMode(doorDirectionPin, OUTPUT);
  pinMode(statusLightPin, OUTPUT);
  pinMode(statusLightPinLocked, OUTPUT);
  pinMode(endSwitchPin, INPUT); // Use INPUT_PULLUP to enable internal pull-up resistor
  pinMode(CONTROLLINO_D9, OUTPUT); // Set CONTROLLINO_D9 as an output pin for fully closed state

  // Attach the debouncer to the buttonPin and set debounce interval
  debouncer.attach(buttonPin);
  debouncer.interval(debounceInterval);

  Serial.begin(115200);
  Serial.print("GarageControl v1.0");
  Serial.print(endSwitchState);

  // Start the Ethernet connection and the server
  Ethernet.begin(mac, ip, dns, gateway, subnet);
  server.begin();
}

void controlDoor(const String& action) {
  unsigned long currentTime = millis();

  if (action == "open" && doorState != 1) {
    // Open the door only if it's not already opening
    doorState = 1; // Set state to opening
    doorStartTime = currentTime;
    doorMoving = true;
    statusLightOn = true;
    digitalWrite(doorDirectionPin, HIGH); // Set direction to up
    delay(300); // Wait before enabling the power pin
    digitalWrite(doorPowerPin, HIGH); // Turn on door power
  } else if (action == "close" && doorState != 2) {
    // Close the door only if it's not already closing
    doorState = 2; // Set state to closing
    doorStartTime = currentTime;
    doorMoving = true;
    statusLightOn = true;
    digitalWrite(doorDirectionPin, LOW); // Set direction to down
    delay(300); // Wait before enabling the power pin
    digitalWrite(doorPowerPin, HIGH); // Turn on door power
  } else if (action == "stop") {
    // Stop the door if it's moving
    if (doorMoving) {
      doorState = 0; // Set state to idle
      doorMoving = false;
      statusLightOn = false;
      digitalWrite(doorPowerPin, LOW); // Turn off door power
    }
  }

  // Update the time of the last door state change
  lastDoorStateChangeTime = currentTime;
}

void loop() {
  // Update the button state through debouncer
  debouncer.update();
  buttonState = debouncer.read();

  // Check if the button has been pressed
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Check if the 1-second delay has passed since the last door state change
    unsigned long currentTime = millis();
    if (currentTime - lastDoorStateChangeTime >= doorChangeDelay) {
      // Change door state only if the delay has passed
      doorState = (doorState + 1) % 3;

      // Start or stop door movement
      if (doorState == 1 || doorState == 2) {
        doorStartTime = millis();
        doorMoving = true;
        statusLightOn = true;

        // Set direction output
        if (doorState == 1) {
          // Opening
          digitalWrite(doorDirectionPin, HIGH); // Set direction to up
        } else if (doorState == 2) {
          // Closing
          digitalWrite(doorDirectionPin, LOW); // Set direction to down
        }

        // Wait for 300ms before enabling the power pin
        delay(300);
        digitalWrite(doorPowerPin, HIGH); // Turn on door power
      } else {
        doorMoving = false;
        statusLightOn = false;
        digitalWrite(doorPowerPin, LOW); // Turn off door power
      }

      // Update the time of the last door state change
      lastDoorStateChangeTime = currentTime;
    }
  }

  endSwitchState = digitalRead(endSwitchPin); // Read the current state of the end switch
  isDoorFullyClosed = (endSwitchState == LOW); // Update based on your switch configuration

  /*
  // Check if the door is moving down and the end switch is triggered
  if (doorState == 2 && endSwitchState == LOW) {
    // End switch is triggered, indicating the door is fully closed
    doorMoving = false; // Stop the door movement
    doorState = 0; // Set door state to idle
    statusLightOn = false;
    digitalWrite(doorPowerPin, LOW); // Turn off door power
    digitalWrite(statusLightPin, HIGH); // Optionally, turn on status light to indicate door is fully closed
  }
  */

  // Control door movement
  if (doorMoving) {
    unsigned long currentTime = millis();
    if (currentTime - doorStartTime < doorMovementTime) {
      // Continue with door movement
      Serial.print("endSwitchState:");
      Serial.print(endSwitchState);
    } else {
      // Time to stop door movement
      doorMoving = false;
      statusLightOn = false;
      digitalWrite(doorPowerPin, LOW); // Turn off door power
    }
  }

  // Control status light blinking
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentMillis;
    if (statusLightOn) {
      digitalWrite(statusLightPin, !digitalRead(statusLightPin)); // Toggle status light
    } else {
      digitalWrite(statusLightPin, HIGH); // Turn on status light constantly while idle
    }
  }

  // Check if the door is moving down and the end switch state has changed
  if (doorState == 2) {
    if (endSwitchState != lastEndSwitchState) {
      if (endSwitchState == HIGH) {
        // End switch has gone high, delay turning off power pin
        digitalWrite(statusLightPinLocked, HIGH);
        endSwitchTriggerTime = millis() + endSwitchPowerOffDelay; // Set the trigger time with a delay
      }
    }
  }

  // Check if the end switch delay has passed and turn off the power pin
  if (endSwitchTriggerTime > 0 && (millis() - endSwitchTriggerTime >= endSwitchPowerOffDelay)) {
    doorMoving = false;
    statusLightOn = false;
    digitalWrite(doorPowerPin, LOW); // Turn off door power
    endSwitchTriggerTime = 0; // Reset the trigger time
  }

  // Update the last end switch state
  lastEndSwitchState = endSwitchState;

// Web server handling
  EthernetClient client = server.available();
if (client) { // If a new client connects,
  String currentLine = ""; // Make a String to hold incoming data from the client
  while (client.connected()) { // Loop while the client's connected
    if (client.available()) { // If there's bytes to read from the client,
      char c = client.read(); // Read a byte, then
      if (c == '\n') { // If the byte is a newline character

        // If the current line is blank, you got two newline characters in a row.
        // That's the end of the client HTTP request, so send a response:
        if (currentLine.length() == 0) {
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE html><html>");
          client.println("<head><title>Garage Door Controller</title>");
          client.println("<style>");
          client.println("  body { background-color: #333; color: white; font-family: Arial, sans-serif; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; flex-direction: column; height: 100vh; }");
          client.println("  h1 { text-align: center; margin-bottom: 20px; }");
          client.println("  .container { text-align: center; width: 100%; }");
          client.println("  button { background-color: #555; color: white; border: none; padding: 20px 40px; text-align: center; text-decoration: none; font-size: 20px; margin: 10px; cursor: pointer; border-radius: 12px; width: 80%; max-width: 300px; }"); // Larger buttons
          client.println("  .led { height: 24px; width: 24px; background-color: #bbb; border-radius: 50%; display: inline-block; margin-bottom: 20px; }");
          client.println("  .led.on { background-color: #0f0; }"); // Green when on
          client.println("</style>");
          client.println("</head>");
          client.println("<body>");
          client.println("<div class='container'>");
          client.println("<head><title>Garage Door Controller</title></head>");
          client.println("<body>");
          client.println("<h1>Garage Door Controller</h1>");
          client.println("<p>Click a button to control the garage door:</p>");
          client.println("<button onclick=\"location.href='/open'\">Open Door</button>");
          client.println("<button onclick=\"location.href='/close'\">Close Door</button>");
          client.println("<button onclick=\"location.href='/stop'\">Stop Door</button>");
          client.println("<p>Door Status:</p>");
          client.println("<div style='text-align: center;'>"); // Center-align the content of this div
          client.print("<div style='display: inline-block; width: 30px; height: 30px; border-radius: 15px; background-color: ");
          client.print(isDoorFullyClosed ? "#00FF00" : "#FF0000"); // Green if closed, red otherwise
          client.println(";'></div>");
          client.println("</div>"); // Close the centering div

          // The HTTP response ends with another blank line
          client.println();
          // Break out of the while loop
          break;
        } else { // If you got a newline, then clear currentLine
          currentLine = "";
        }
      } else if (c != '\r') { // If you got anything else but a carriage return character,
        // Add it to the end of the currentLine
        currentLine += c;
      }

      // Check the request route
      if (currentLine.endsWith("GET /open")) {
        controlDoor("open");
      } else if (currentLine.endsWith("GET /close")) {
        controlDoor("close");
      } else if (currentLine.endsWith("GET /stop")) {
        controlDoor("stop");
      }
    }
  }
  // Close the connection
  client.stop();
}
  // Update the last button state
  lastButtonState = buttonState;
}