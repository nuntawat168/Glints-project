from django.contrib import admin
from django.urls import path

from django.urls import include
from myapp import views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('myapp/', include('myapp.urls')),
    path('', views.index, name='index'),    
]