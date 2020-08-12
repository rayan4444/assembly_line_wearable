# Crimson <> Bunnie meeting (24/09/2019)

## How to improve python running on Rpi zero (faster, less resource intensive)
* Pickling (https://pythontips.com/2013/08/02/what-is-pickle-in-python/)
* Microphone: Reduce sample count to 8kHz for microphone, record in mono
* Pre-compile python

Basically spend a bunch of time optimising your python code.

## Real time? 
Live counting with python: if you do live stream processing you need at least 2 cores, one for sampling , one for processing)

## If you want to add AI for fanciness: 
* Google AI kit: https://www.blog.google/technology/ai/introducing-aiy-vision-kit-make-devices-see/
* Pi HAT format train drill sound as a “attention word” 
* You can train it on different tools (microphone data and maybe even IMU?)

## IOT approach vs processing everythign locally 
* everything on the glove, it’s self contained: porting python to a more contained MCU might be difficult, strong-ish processor needed that's going to be on the more expensive side. 

vs 

* ioT : (more potential problems having a whole server infrastructure, antenna/electrical interference etc….)> hidden costs. So if your team is not especially strong in IOT going that route is not recommended. It might end up costing you a lot more than the self contained solution


