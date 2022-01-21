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

#define I2C_SDA 33
#define I2C_SCL 32
#define BNO08X_RESET -1

#include "accelerometer_handler.h"

#include <Wire.h>
#include <Adafruit_BNO08x.h>

int begin_index = 0;

float save_data[600] = {0.0};
bool pending_initial_data = true;
long last_sample_millis = 0;

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
TwoWire I2CBNO08X= TwoWire(0);

TfLiteStatus SetupAccelerometer(tflite::ErrorReporter* error_reporter) {
  I2CBNO08X.begin(I2C_SDA, I2C_SCL, 100000);
  if (!bno08x.begin_I2C(0x4A, &I2CBNO08X)) {
      error_reporter->Report("IMU Init Fail");
      return kTfLiteError;
  }
  bno08x.enableReport(SH2_ACCELEROMETER);  
  
  error_reporter->Report("IMU Init");
  return kTfLiteOk;
}

static bool UpdateData() {
  if (bno08x.wasReset()) {
    if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
        return false;
    }
  }

  bool new_data = false;
  if ((millis() - last_sample_millis) < 40) {
    return false;
  }
  last_sample_millis = millis();

  float accX = 0.0F;
  float accY = 0.0F;
  float accZ = 0.0F;
  bno08x.getSensorEvent(&sensorValue);
  accX = round((sensorValue.un.accelerometer.z)*101.97162129779283);
  accY = -round((sensorValue.un.accelerometer.y)*101.97162129779283);
  accZ = round((sensorValue.un.accelerometer.x)*101.97162129779283);

  save_data[begin_index++] = accX;
  save_data[begin_index++] = accY;
  save_data[begin_index++] = accZ;

  if (begin_index >= 600) {
    begin_index = 0;
  }
  new_data = true;

  return new_data;
}

bool ReadAccelerometer(tflite::ErrorReporter* error_reporter, float* input,
                       int length, bool reset_buffer) {
  if (reset_buffer) {
    memset(save_data, 0, 600 * sizeof(float));
    begin_index = 0;
    pending_initial_data = true;
  }

  if (!UpdateData()) {
    return false;
  }

  if (pending_initial_data && begin_index >= 200) {
    pending_initial_data = false;
  }

  if (pending_initial_data) {
    return false;
  }

  for (int i = 0; i < length; ++i) {
    int ring_array_index = begin_index + i - length;
    if (ring_array_index < 0) {
      ring_array_index += 600;
    }
    input[i] = save_data[ring_array_index];
  }
  return true;
}
