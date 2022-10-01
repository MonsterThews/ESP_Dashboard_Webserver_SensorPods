# ESP_Dashboard_Webserver_SensorPods
This is my SmartHome/ IOT project.
ESP Now: One receiver and three sensor pods.

As of 1 October, 2022, these might all work. I broke a few parts of it, so I'm not uploading the current versions.

Receiver collects data, displays it on TFT and publishes a webserver. The three pods send data with no delay, but the receiver displays 15 seconds before refreshing. It will display all three pods' data in time.
I use WeMos D1 R32 with ILI98xx display. It has SD slot. 
I plan to datalog pressure and predict weather.
I want to load this onto my 8266 Mega, but I haven't gotten it to work. RobotDyn is a ridiculous contraption.
![TFT](https://user-images.githubusercontent.com/61639361/193425883-034fed0f-6c04-41b6-8d8a-f24e8af5dc0b.JPG)

Pod1 is in flux. This is the outdoor sensor pod. It has LM35 for analog temperature, VEML 6070 for Uv. Adding AS3935 for lightning and PM2.5 and PM10. And I should add MQ-2 for smoke.

Pod2 is for interiors. It has BME 680 for Temperature, Humidity, Pressure and VOC, SGP30 for CO2, H2 and some other things, CJMCU 6814 (analog) for CO, NH3, NO2, MQ-6 for LPG (analog).

There's a third pod in here. That one was the exterior pod.
![IMG_0885](https://user-images.githubusercontent.com/61639361/193425877-424b6176-1996-4669-9306-0f7c2755b478.JPG)
I can only get the webserver to display four items. I added Pressure and Uv to Random Nerds' Temperature and Humidity, but nothing else will display.
![タイトルなし](https://user-images.githubusercontent.com/61639361/191618904-fc2ddc0d-9545-43df-a9b5-794c42a51079.png)

