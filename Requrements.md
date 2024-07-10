# Project Overview

To produce a Visual Studio Code platformIO solution for an Expressif ESP-WROOM-32 device that satisfies the following requirements;

# Requirements

## Functional Requirements

## Sensor setup

The ultrasonic sensor is connected as: Trigger on pin D5, Echo on pin D18

## network setup

Must: connect to an available SSID before leaving the setup method.
Should: scan the SSID's and thier RSSI values to obtain a preferred list based on best RRSSI value now.
Should: then Use the 'preferredSSIDs' comma separated list of SSID provided in an included <MyWiFiCreds.h> file, and the 'ssid_password' from the same file to try to connect.
Should: fail over to the next preferred SSID in the list if not connected within 15 seconds.
Must: pause forever during setup if not able to connect to any SSID. I.e. the device must connect to network before ever starting to get the time, or detect from the sensor, or serve client endpoints 
Should: attempt a reconnect to the last known SSID when a network disconnect occurs waiting up to 15 seconds before retrying
Must: not prevent sensor data being recorded whilst attempting a network re-connect. 
Should: Not require security for the device endpoints.
Must: provide multiple aysnc end points as per the API specification for external client tasks to query.
Should: output the SSID, MAC address, IP Address and NNTP Current Time (YYYY-MM-DD HH:MM:SS) on completion of the setup.

### Current Time

Must: first connect to an NNTP endpoint to get the UTC time before starting to gather sensor data
Should: use NNTP client update to get accurate currentTime for each sensorData value.
Should: keep an alternate track of elepased time in case NNTP updates cannot be obtained and use that alternate time value for currentTime when storing SensorData.


### Depth calcuations

Must: Calculate the sensors 'distance' from the water level converted to CM every 10 seconds regardless of ongoing network connection.
Should: Assume a knownDepth of 300CM, calculate the 'depth' by sutracting 'distance' from 'knownDepth'.


### Data

Must: Every 10 seconds a new value and time is taken and stored in the FIFO array
Must: Ensure the sensorData data is stored in memory as a FIFO Array that does not exceed 25% of the free memory. 
Must: Ensure all access to the FIFO array for reading or writing is via a MUTEX to handle concurrency for multiple threads.

# Data Models
## SensorData
- depth: float
- time: unsigned long 


## Non functional requirements
# Code files
1. Declare for inclusion in the solution file which Libraries need to be added to the project as part of the platformio.ini file itself
2. Assume Serial communications is the default and at a BAUD of 115200 within the devidce and also in the platformio.ini file for smoother build wworkflow
3. Assume the upload speed is left as the fastest supported by the platform.
4. Note any special setup instructions for Linux regarding permissions and hardware access.
5. Comment the solution as if it were an educational example

# Technical Specifications
- Programming Language: [c++]
- Framework: [Arduino]
- Libraries: [To be specified]
- Tools: [VS Code/PlatformIO plugin].  

# APIs

Should: On accessing the sensorData array, first lock it via the mutex, then read it, then clear it if the data was output as Json then unloc it.
Should: On reading the SensorData the unsigned long currentTime is converted to a String of format YYYY-MM-DD HH:MM:SS for output purposes.

## Endpoint /
- Method: GET
- URL: /
- Request Parameters: [none]
- Response: [HTML]
- Action: [Output, on separate lines, the devices: Host name if known, IP Address, MAC Address, available memory in KiloBytes, Free Memory in Kilobytes, current size of the sensorData array in KiloBytes and the current time as YYYY-MM-DD HH:MM:SS, and refresh every 10 seconds] 

## Endpoint /data
- Method: GET
- URL: /
- Request Parameters: [none]
- Response: [Json]
- Action: [Output the sensorData array as a Json array, return teh response to the client, then clears the array down] 

## Endpoint /data with reset
- Method: GET
- URL: /
- Request Parameters: [reset]
- Response: [Json]
- Action: [Same as for the /data endpoint but then waits 3 seconds and performs a board reset] 

## Endpoint /monitor
- Method: GET
- URL: /monitor
- Request Parameters: [none]
- Response: [HTML]
- Action: [Output the sensorData array an item at a time in reverse chronological order as an HTML Data Table, then refresh every 10 seconds] 



# Mockups/Examples
None

## Example Input
No input

