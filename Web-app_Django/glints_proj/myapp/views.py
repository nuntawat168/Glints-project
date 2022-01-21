from django.shortcuts import render
from django.http import HttpResponseRedirect
from django.urls import reverse

import paho.mqtt.client as mqtt
from .models import device_hub

def index(request):

      devices_obj = device_hub.objects.get(device_id=1)

      light_01_status = devices_obj.light_01
      light_02_status = devices_obj.light_02
      light_03_status = devices_obj.light_03
      last_gesture    = devices_obj.last_gesture

    #-----GET input command from user-------#
      light_01_status_new = request.GET.get("light_01_status")
      light_02_status_new = request.GET.get("light_02_status")
      light_03_status_new = request.GET.get("light_03_status")

      just_call_request = request.GET.get("dummy")

    #----Create message from command (msg = "LIGHT_XX: ON/OFF")----#
      msg = ""
      if light_01_status_new == "ON":
            msg = "light_01:ON"
      elif light_01_status_new == "OFF":
            msg = "light_01:OFF"

      elif light_02_status_new == "ON":
            msg = "light_02:ON"
      elif light_02_status_new == "OFF":
            msg = "light_02:OFF"

      elif light_03_status_new == "ON":
            msg = "light_03:ON"
      elif light_03_status_new == "OFF":
            msg = "light_03:OFF"

      respond = device_send_rec_msg(msg)

    #----Update status of light and last gesture detected parameter to database---#
      if respond:
         if respond[0] == "0":
            light_01_status = "OFF"
            devices_obj.light_01 = light_01_status
         elif respond[0] == "1":
            light_01_status = "ON"
            devices_obj.light_01 = light_01_status

         if respond[1] == "0":
            light_02_status = "OFF"
            devices_obj.light_02 = light_02_status
         elif respond[1] == "1":
            light_02_status = "ON"
            devices_obj.light_02 = light_02_status

         if respond[2] == "0":
            light_03_status = "OFF"
            devices_obj.light_03 = light_03_status
         elif respond[2] == "1":
            light_03_status = "ON"
            devices_obj.light_03 = light_03_status
         
         last_gesture = "gesture_" + respond[3]
         devices_obj.last_gesture = last_gesture
         devices_obj.save()

      context = {
         'light_01_status': light_01_status,
         'light_02_status': light_02_status,
         'light_03_status': light_03_status,
         'last_gesture': last_gesture,
      }
    #----Render Web app with status of light and last gesture detected---#
      return render(request, 'myapp/device_status_1.html', context)

def device_send_rec_msg(msg):

      broker_address = "127.0.0.1" #localhost 
      mqtt_username = "dummy"
      mqtt_password = "bank"
      client = mqtt.Client("SERVER") #Client ID
      client.username_pw_set(mqtt_username, mqtt_password) # Sets the username and password
      client.on_message=on_message #attach subscribe_callback_function
      client.on_publish = on_publish#attach publish_callback_function 
      client.connect(broker_address) #connect to broker

      global published
      published = None
      global rec_msg
      rec_msg = None
      client.subscribe('RESPOND') #subscribe topic 'RESPOND' 
      client.publish("COMMAND", msg)#publish topic 'COMMAND' with message 'msg'
      
      #----wait 5 second ,Is published succeed?----#
      count = 50
      while count > 0:
         client.loop(0.1) # This wait for 0.1 seconds
         count -= 1
         if published:
            break
      
       #----wait 5 second ,Is received a response message succeed?----#
      if published:
         count = 50 # Wait a maximum of 5 seconds.  1 count is 0.1 seconds
         while count > 0:
            client.loop(0.1) # Waits within the loop for 0.1 seconds
            count -= 1
            if rec_msg:
               break
      client.loop_stop()
      client.disconnect()
      return rec_msg

def on_message(client, userdata, message):
    global rec_msg
    rec_msg = message.payload.decode("utf-8")

def on_publish(client, userdata, message):
   global published
   published = True