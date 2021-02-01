/*  This code is written to only control one water container
 *  There is one water sensor at the bottom
 *  There is one sensor at the top
 *  There is one overflow sensor
 *  There is one relay to control the valve
 */

#include <NTPClient.h> // Facilitate connection to date time server
#include <WiFiUdp.h> // Allows Udp connection to date time server
#include <ESP8266WiFi.h> // Lets you connect to wifi
#include <EEPROM.h> // Store and retrieve information from presistant storage
#include <myInfo.h> // My wifi information

// Define the ssid and password of wifi
// I keep these in a seperate file called myInfo.h and read from there
#define MySSID myInfo_SSID
#define MyWifiPassword myInfo_WiFiPassword

// Define all of the pins that we will be using
// DO NOT USE D3 D4 or D8 these are used for booting
const int coffee_relay = D1; 
const int espresso_relay = D2;
const int coffe_machine_water = D5;
const int espresso_machine_water = D6;

const int hours_to_wait_espresso = 4 * 24;

// Define NTP Client to get date and time
const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void fill_machine(int machine_water_pin, int machine_relay_pin){
  // While the machine is not full keep filling
  while(digitalRead(machine_water_pin) == 1){
    digitalWrite(machine_relay_pin, HIGH);
  }
  // If we get here we should be full lets stop filling
  digitalWrite(machine_relay_pin, LOW);
}

int get_last_espresso_fill(){
  EEPROM.begin(512); // Initialize EEPROM
  return EEPROM.read(0); // Read the last filled date
}

void set_last_espresso_fill(int today){
   EEPROM.begin(512); // Initialize EEPROM
   EEPROM.put(0,today); // Write new fill date
   EEPROM.commit(); // Store in EEPROM
}

void setup(){

  Serial.begin(115200);

  // Define relays here
  pinMode(coffee_relay, OUTPUT);
  pinMode(espresso_relay, OUTPUT);
  // Default them both off
  // THIS MAY CHANGE
  digitalWrite(coffee_relay, LOW);
  digitalWrite(espresso_relay, LOW);

  // Fill th coffee machine before wifi connect so if failure at least that will fill
  fill_machine(coffe_machine_water, coffee_relay);

  // Start to connect to wifi
  WiFi.begin(MySSID, MyWifiPassword);

  // Wait until we have joined a wifi network
  // Use . to show progress
  int tries = 0;
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );

    // After 50 trys just give up and try again later
    if(tries > 50){
      break;
    }
    tries++;
  }

  // Once we are conected we display connection information
  // Would probably get rid of this entire if
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // Start the connection to the date time server
  timeClient.begin();

  // Get the current date and time as unix timestamp (seconds from Jan 01 1970)
  timeClient.update();
  // Convert time to days since Jan 01 1970
  int today = ceil(timeClient.getEpochTime() / 60 / 60);

  // If it has been the designated amount of time lets try and fill
  if(today >= get_last_espresso_fill() + hours_to_wait_espresso){
    fill_machine(espresso_machine_water, espresso_relay);
    set_last_espresso_fill(today);
  } 

  ESP.deepSleep(0); // This will sleep until the rst pin is triggered or power is lost and resupplied
}

void loop() {
  // Nothing here will run as deep sleep is called at the end of startup
}
