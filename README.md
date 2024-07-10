# esp32-well-depth

Arudiono ESP32 device with uSound sensor to sense well depth over time and report when asked.


Please refer to the [Requirements.md](Requrements.md) document for what this does in detail.

I threw those requirements at ChatGPT to help me design the code contained here. Over a period of days to refine the requirement wording to get better resutls.

The code here is exactly what ChatGPT produced to see the correlation. of Requirements -> Output

It wasnt a perfect run as many refinements and questions occured as you can see here; [ChatGPT Conversation](https://chatgpt.com/share/783bef9c-f8eb-4074-8cbb-0433cfd6f199)

I have yet to do a GAP analysis on my original requriements and why they were ot all understood.


# Getting started

You will need to create your own `MyWiFiCreds.h' file as below, in the Include folder before compiling.


```
#ifndef MYWIFICREDS_H
#define MYWIFICREDS_H

const char* preferredSSID = "your_preferred_ssid";
const char* wifiPassword = "your_wifi_password";

#endif // MYWIFICREDS_H

```


if you are intending to use the tests then first. But it should be installed by python already.

```
pip install requests

```