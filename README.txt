TideWeatherAPRS 

3/16/2013

Arduino code for a portable tide and weather monitoring APRS station.  

Idea here is to have the ability to deploy a portable box that can measure
water levels and weather information during a flood event.  Also could be used for quick 
deployment on fishing or camping trips to monitor water levels near camp.

A mounted GPS unit is combined with a BMP085 temp and barometer sensor, a light sensor (model ), and a sonar measuring unit (model ).

These sensors feed to an arduino Uno microcontroller which sends serial data to a TNC (model ) 
which sends audio to a model() 2 watt? transmitter operating on the US APRS frequency 149.390MHz.  

Power to the arduino is by a x watt solar panel feeding a x amp hour dry cell.

Tasks are to:

Re-write cleaner code with calls
Provide code so the gps can be physically detached/removed after a few cycles
Lots of testing to do
Make the hardware container 



