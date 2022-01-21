/*
 * Node no.2 is 'ESP32' and Coordinator in zigbee network
 */

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

#define RXD2 13
#define TXD2 12

WiFiClientSecure wifiClient;
SoftwareSerial xbee_serial(RXD2,TXD2);

char *ssid = "dummy";       // maximum allowable ssid length is 27, as one terminating null character is reserved
char *password = "bank";   // maximum allowable password lenth is 27
const char* mqtt_username = "dummy";
const char* mqtt_password = "bank";

const char* mqtt_server = "glints-proj.xyz";
const char* clientID = "Device1";
PubSubClient client(mqtt_server, 8883, wifiClient); // 8883 is the secure listener port for the Broker

//char light_status[4] = {'0', '0', '0', '0'};
char light_status[3] = {'0', '0', '0'};
char last_gesture_status = '0';

const char* caCert = \
"-----BEGIN CERTIFICATE-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"-----END CERTIFICATE-----\n" ;

void setup()
{ 
  Serial.begin(115200);
  xbee_serial.begin(9600);
  Serial.println();
 
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(5, OUTPUT);   
  digitalWrite(16, LOW);
  digitalWrite(17, LOW);
  digitalWrite(5, LOW);

  // Connect to the WiFi
  WiFi.begin(ssid, password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  wifiClient.setCACert(caCert);

  client.setCallback(ReceivedMessage);

  // Connect to MQTT Broker
  if (Connect()) {
    Serial.println("Connected Successfully to MQTT Broker!");  
  }
  else {
    Serial.println("Connection Failed!");
    //Serial.print(client.state());
  }
}


void loop() {
   char msg[256];
   char msg_xbee_trans[256];
   
   // check WiFi status
   // if WiFi is not connected, powercycle the device
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Lost WiFi connection.  Restarting");
      delay(50);
      ESP.restart();  // this is software reset which may not work all the time
   }
   
   // If the connection is lost, try to connect again
   if (!client.connected()) {
     Connect();
   }

  if(xbee_serial.available()>0)
  {
      String gesture_input = "0";
      String serial_input = xbee_serial.readString();
      Serial.println(serial_input);
      gesture_input = getValue(serial_input, ':', 1);
      char buf[gesture_input.length()+1];
      gesture_input.toCharArray(buf,gesture_input.length());
      last_gesture_status = buf[0];
      
      if (last_gesture_status == '1') 
      {
          Serial.println("Toggle_light_1");
      } 
      if (last_gesture_status == '2') 
      {
          Serial.println("Toggle_light_2");
      } 
      if (last_gesture_status == '3') 
      {
          Serial.println("Toggle_light_3");
      } 
      sprintf(msg_xbee_trans, "%c%c%c", light_status[0], light_status[1], light_status[2], light_status[3]);
      xbee_serial.println(msg_xbee_trans);
  }
   client.loop();
}

void ReceivedMessage(char* topic, byte* payload, unsigned int payload_length) {
  // Output the first character of the message to serial (debug)
  char msg[256];
  char msg_xbee_trans[256];
  int i;
  uint16_t motion_sensor;
  float temperature, humidity;
  char* motion_str;
  boolean send_sensors;
 
  if (payload_length > 0) {
    
     if (strncmp((char*)payload, "light_01:OFF", payload_length) == 0) 
     { Serial.println("light1 : off"); light_status[0]='0';} 
     else if (strncmp((char*)payload, "light_01:ON", payload_length) == 0) 
     { Serial.println("light1 : on");  light_status[0]='1';} 
     
     else if (strncmp((char*)payload, "light_02:OFF", payload_length) == 0) 
     { Serial.println("light2 : off"); light_status[1]='0';} 
     else if (strncmp((char*)payload, "light_02:ON", payload_length) == 0) 
     {  Serial.println("light2 : on");  light_status[1]='1';} 
     
     else if (strncmp((char*)payload, "light_03:OFF", payload_length) == 0) 
     { Serial.println("light3 : off"); light_status[2]='0'; } 
     else if (strncmp((char*)payload, "light_03:ON", payload_length) == 0) 
     { Serial.println("light3 : on");  light_status[2]='1';} 
  }


  sprintf(msg, "%c%c%c%c", light_status[0], light_status[1], light_status[2], last_gesture_status);
  mqtt_client_publish("RESPOND", msg);
  sprintf(msg_xbee_trans, "%c%c%c", light_status[0], light_status[1], light_status[2]);
  xbee_serial.println(msg_xbee_trans);
  Serial.print("respond_msg: ");
  Serial.println(msg);
}

bool Connect(){
  // Connect to MQTT Server and subscribe to the topic
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
      client.subscribe("COMMAND");
      return true;
    }
    else {
      return false;
  }
}

boolean mqtt_client_publish(char* topic, char* msg) {
   boolean result;

   result = client.publish(topic, msg);
   if (!result) {
      Serial.println("Connected, but could not publish");
   }
   return result;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
