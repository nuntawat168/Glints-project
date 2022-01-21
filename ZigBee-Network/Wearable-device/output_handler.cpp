/*  Adapted by Nuntawat Maliwan.
 *  Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "output_handler.h"
#include <SoftwareSerial.h>
#include "Pangodream_18650_CL.h"

#define SW_1 15
#define SW_2 2
#define SW_3 4
#define LED_LOW_BATT 19
#define LED_MID_BATT 23
#define LED_HIGH_BATT 18
#define SIG_VIBRATION 27
#define RXD2 16
#define TXD2 17

SoftwareSerial xbee_serial(RXD2,TXD2);
Pangodream_18650_CL BL(34, 1.62, 20);

int batt_level ;
int sw_state[3] = {0, 0, 0};
int sw_prev_state[3] = {0, 0, 0};
long start_drive_vibration_motor_millis = -1001;
long duration_vibration_motor = 250;
int num_repetitions = 1;

void HandleOutput(tflite::ErrorReporter* error_reporter, int kind) {

  static bool is_initialized = false;
  if(!is_initialized){
    xbee_serial.begin(9600);
    pinMode(SW_1, INPUT);
    pinMode(SW_2, INPUT);
    pinMode(SW_3, INPUT);
    pinMode(LED_LOW_BATT, OUTPUT);
    pinMode(LED_MID_BATT, OUTPUT);
    pinMode(LED_HIGH_BATT, OUTPUT);
    pinMode(SIG_VIBRATION, OUTPUT);
    digitalWrite(LED_LOW_BATT, LOW);
    digitalWrite(LED_MID_BATT, LOW);
    digitalWrite(LED_HIGH_BATT, LOW);
    digitalWrite(SIG_VIBRATION, LOW);
    is_initialized = true;
  }

  if ((millis() - start_drive_vibration_motor_millis) <= duration_vibration_motor*num_repetitions) {
    digitalWrite(SIG_VIBRATION, HIGH);
  }
  else{
    digitalWrite(SIG_VIBRATION, LOW);
  }
  //prev_millis = millis();


  sw_state[0] = digitalRead(SW_1);
  sw_state[1] = digitalRead(SW_2);
  sw_state[2] = digitalRead(SW_3);
  for(int i =0; i<3;i++){
    if(sw_state[i] == 0 && sw_prev_state[i] == 1){
      error_reporter->Report("sw_%d is presssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss\n",i+1);
      start_drive_vibration_motor_millis = millis();
      xbee_serial.printf("gesture:%d\n",i+1);
    }
    sw_prev_state[i] = sw_state[i];
  }
  
  batt_level =  BL.getBatteryChargeLevel();
  if(batt_level>75){
    digitalWrite(LED_HIGH_BATT, HIGH);
    digitalWrite(LED_MID_BATT, LOW);
    digitalWrite(LED_LOW_BATT, LOW);
  }
  else if(batt_level>25){
    digitalWrite(LED_HIGH_BATT, LOW);
    digitalWrite(LED_MID_BATT, HIGH);
    digitalWrite(LED_LOW_BATT, LOW);
  }
  else{
    digitalWrite(LED_HIGH_BATT, LOW);
    digitalWrite(LED_MID_BATT, LOW);
    digitalWrite(LED_LOW_BATT, HIGH);
  }
  
  if (kind == 0) {
    xbee_serial.println("gesture:1");
    digitalWrite(SIG_VIBRATION, HIGH);
    error_reporter->Report(
        "WING:\n\r*         *         *\n\r *       * *       "
        "*\n\r  *     *   *     *\n\r   *   *     *   *\n\r    * *       "
        "* *\n\r     *         *\n\r");
  } else if (kind == 1) {
    xbee_serial.println("gesture:2");
    digitalWrite(SIG_VIBRATION, HIGH);
    error_reporter->Report(
        "RING:\n\r          *\n\r       *     *\n\r     *         *\n\r "
        "   *           *\n\r     *         *\n\r       *     *\n\r      "
        "    *\n\r");
  } else if (kind == 2) {
    xbee_serial.println("gesture:3");
    digitalWrite(SIG_VIBRATION, HIGH);
    error_reporter->Report(
        "SLOPE:\n\r        *\n\r       *\n\r      *\n\r     *\n\r    "
        "*\n\r   *\n\r  *\n\r * * * * * * * *\n\r");
  }
}
