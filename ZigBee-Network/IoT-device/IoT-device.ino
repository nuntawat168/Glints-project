#include <SoftwareSerial.h>

#define LED_1 3
#define LED_2 4
#define LED_3 5
#define RXD2 6
#define TXD2 7

SoftwareSerial xbee_serial(RXD2,TXD2);

char light_status[3] = {'0', '0', '0'};
String input_data = "";

void setup() {
  Serial.begin(115200);
  xbee_serial.begin(9600);
  
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  delay(500);
}

void loop() {
  //char msg[256];
  while(xbee_serial.available()>0)
  {
    input_data = xbee_serial.readString();
    Serial.println(input_data);
    light_status[0] = input_data.charAt(0);
    light_status[1] = input_data.charAt(1);
    light_status[2] = input_data.charAt(2);

    //sprintf(msg, "%c%c%c", light_status[0], light_status[1], light_status[2]);
    //Serial.println(msg);
    active_light();
   }  
}

void light_1_on(){digitalWrite(LED_1, HIGH);}
void light_1_off(){digitalWrite(LED_1, LOW);}

void light_2_on(){digitalWrite(LED_2, HIGH);}
void light_2_off(){digitalWrite(LED_2, LOW);}

void light_3_on(){digitalWrite(LED_3, HIGH);}
void light_3_off(){digitalWrite(LED_3, LOW);}

void active_light()
{
  if (light_status[0]== '1')
  {
    light_1_on();
  }
  else if (light_status[0]== '0')
  {
    light_1_off();
  }

  if (light_status[1]== '1')
  {
    light_2_on();
  }
  else if (light_status[1]== '0')
  {
    light_2_off();
  }

  if (light_status[2]== '1')
  {
    light_3_on();
  }
  else if (light_status[2]== '0')
  {
    light_3_off();
  }
}
