# ESP_Dashboard_Webserver_SensorPods
This is my SmartHome/ IOT project.
ESP Now: One receiver and three sensor pods.
Look in the folders for all the files.

If you can, please suggest a sensor I for sonething I don't have.

As of 24 October, 2022, these all work.
Two Pods send data to a Receiver, which displays it on TFT and publishes a webserver. These photos are old.
I'm trying to figure out how I got Uv to display like in the photo below. Lost it.

There is no Pod 3 today. Pod 1 is for outside, a D1 Mini/32 with LM35 for temperature, VEML6070 for Uv and MQ-2 for smoke. Particulate and Lightning will go there. Pod 2 is D1 R32 with BME 680 (Temp, Hum, Press, VOC), SGP 30 (CO2, H2, EtOH, TVOC), "6184"(NH3, NO2, CO) and MQ-6 (LPG).

Receiver code is Phoenix_Supergrover.

Receiver is D1 R32 with ILI98xx display. It collects data, displays it on TFT and publishes a webserver. The three pods send data without delay, but I'd like to start looking at power consumption. I'll have them on batteries eventually. Receiver refreshes in 15 seconds. It will display all three pods' data randomly, whichever is in the buffer when the TFT loops.

![TFT](https://user-images.githubusercontent.com/61639361/193425883-034fed0f-6c04-41b6-8d8a-f24e8af5dc0b.JPG)

There's a third pod in here. That one was the exterior pod.
![IMG_0885](https://user-images.githubusercontent.com/61639361/193425877-424b6176-1996-4669-9306-0f7c2755b478.JPG)
I can only get the webserver to display four items. I added Pressure and Uv to Random Nerds' Temperature and Humidity, but nothing else will display.
![タイトルなし](https://user-images.githubusercontent.com/61639361/191618904-fc2ddc0d-9545-43df-a9b5-794c42a51079.png)

