from django.db import models

# Create your models here.
class device_hub(models.Model):
   device_id =  models.PositiveIntegerField(unique=True) 
   light_01  = models.CharField(max_length=3, default = 'OFF')
   light_02  = models.CharField(max_length=3, default = 'OFF')
   light_03  = models.CharField(max_length=3, default = 'OFF')
   last_gesture = models.CharField(max_length=10, default = 'gesture_00')