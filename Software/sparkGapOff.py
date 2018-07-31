#!/usr/bin/env python

import RPi.GPIO as GPIO #For GPIO functions
import sys #For taking in command line arguments (triggerPin value)

triggerPin = int(sys.argv[1]) #Set triggerPin equal to value passed in command line argument

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM) #Set pins to go by GPIO numbers (not board numbers)
GPIO.setup(triggerPin, GPIO.OUT) #Set the trigger pin as output

GPIO.output(triggerPin, 0) #Set trigger pin as low
