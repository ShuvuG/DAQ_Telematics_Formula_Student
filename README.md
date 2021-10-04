#Data_Acquisition_System 

DAQ and telematics system for the 2020 Brighton Racing Motors Formula Team:

The data acquisition system collected data from the Solartron Metrology LVDT displacement sensor, and a HCSR04 ultrasonic sensor. The sensors were attached to the Arduino Uno microcontrollers. Arduino Uno performed simple mean-value computations and datatype truncations to smoothen any non-linear spikes observed in the data. Sensor readings were then transferred to the master controller at a user-defined sample period via a high- speed CAN 2.0b bus.

Upon receiving the CAN bus data, the master controller extracted the relevant sensor information, timestamped the data and made a bulk upload of sensor readings to a ThingSpeak cloud server. This was carried out every 15 seconds. Simultaneously, the data was logged to a CSV file. Simple scatter-plotting based codes were written in MATLAB and imported to the ThingSpeak server for data visualisations.

Included in the repository are:
1. Modified CANOpen Network for ATMega328P 'slave' microcontroller
2. Mofified CANOpen Network for ATMega328P 'master' microcontroller
3. [Work in progress] GUI for CANOpen calibration
4. [Work in progress] MATLAB based data visualisation


