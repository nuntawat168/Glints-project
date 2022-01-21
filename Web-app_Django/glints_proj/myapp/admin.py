from django.contrib import admin

# Register your models here.
from .models import device_hub
class device_hubAdmin(admin.ModelAdmin):
    list_display = ['device_id', 'light_01', 'light_02', 'light_03', 'last_gesture']

admin.site.register(device_hub,device_hubAdmin)