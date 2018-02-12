# FSAE_SENSORS_ECU

University of New Brunswick, Fredericton 
Electrical Vehicle Front Sensors Unit

Introduction:
--------------
The "FSAE_SENSOR_ECU" project is part of a bigger project implementing the electronic control units inside of the Electric Vehicle being currently designed and built at the University of New Brunswick, Fredericton.

The Electric Vehicle will be participating in a formula race organized by the Society of Automotive Engineers, SAE. Please follow the link for more information.
https://www.sae.org/attend/student-events/formula-sae-electric/

Our facebook group page at UNB: https://www.facebook.com/groups/359276684507837/

Project Description:
--------------------
The FSAE_SENSOR_ECU project's main purpose is to collect data from several sensors located at the front part of the vehicle. It will be mainly collecting data from Analog sensors such as hall effects sensors at the front pedals. The readings collected will indicate the position of the pedals and will be used to control the power supplied to the motor and the breaks. The ECU will also be collecting data from Rotary Encoders from the same pedals for redundancy.

Components selected for the project:
------------------------------------
1- TM4C123GLX Microcontroller board (ARM Cortex M4F @ 80MHz).

2- Throttle Hall Effect Sensor.

3- Break Hall Effect Sensor.

4- Rotary Sensor for Throttle Pedal.

5- Rotary Sensor for Breaks Pedal.

6- Potentiometer for the steering wheel.

Software Design and Operating System:
-------------------------------------
OS: FreeRTOS

Design: Code designed to be modular and portable as much as possible to decrease the dependancy on chosen hardware platform. We are implementing a layered software architecture and creating several levels of abstraction to achieve this goal.

ECU configuration:
------------------
For ADC capture the ECU is currently configured to use TIMER0 to trigger the ADC conversion. The process is done through the hardware modules available on the SOC. The advantages of using this method is that it frees more processing cycles from the MCU.

For the Rotary sensors the processor has a Quadrature Encoder Interface that we are planning to interface with the sensors once they arrive.

