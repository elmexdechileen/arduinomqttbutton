/*


*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Declare vars
int i;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastHighTime = 0;     // Last time the input was high
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long longPressTime = 500; // Detect  long press
char msg[10];


// Define inputs
//int iopins[] = {4,5,6,7,8,9,10,11,12};
int iopins[] = {2};
String payloadlabels[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};

// Number of inputs
const int nInputs = sizeof(iopins);
int pinStatus[nInputs];
int prevPinStatus[nInputs];

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

// Set up mqtt server
const char* server = "192.168.1.145";

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

PubSubClient mqttClient(client);

void setup() {
  // Set pins to listen on
  for (i = 0; i <= nInputs; i = i + 1) {
    pinMode(iopins[i], INPUT);
  }
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // this check is only needed on the Leonardo:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  // print your local IP address:
  printIPAddress();

  // Start mqtt client
  mqttClient.setServer(server, 8883);

  // Connect Mqtt client
  if (mqttClient.connect("343cf7c571ae4c549aeb774820a78f2c")) {
    // connection succeeded
    Serial.println("Connection success");
  } else {
    // connection failed
    // mqttClient.state() will provide more information
    // on why it failed.
    Serial.println(mqttClient.state());
  }
  
}

void loop() {
  
  for (i = 0; i < (nInputs - 1); i = i + 1) {
    boolean reading = digitalRead(iopins[i]);

    // If the switch changed, due to noise or pressing:
    if (reading != prevPinStatus[i]) {
      // reset the debouncing timer
      lastDebounceTime = millis();
    }
  
    // Debounce protection
    if ((millis() - lastDebounceTime) > debounceDelay) {
      String pl = payloadlabels[i];
      pl.toCharArray(msg, 10);
     
      if (reading != pinStatus[i] && reading == 1) {
        // Button pressed
        lastHighTime = millis();
        pinStatus[i] = reading;
      } else if (reading != pinStatus[i] && reading == 0)
        // Detect long press
        if ((millis() - lastHighTime) > longPressTime) {
          Serial.println("Long press detected");
          mqttClient.publish("home-assistant/arduinoIO/long", msg);
        } else {
          Serial.println("Short press detected");
          mqttClient.publish("home-assistant/arduinoIO/short", msg);
        }
        pinStatus[i] = reading;
      }
      prevPinStatus[i] = reading;
  }
  Ethernet.maintain();

}

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

