import os
from time import sleep

#Array of Integration Times (5, 4.75, 4.5, 4.25, 4, 3.75, 3.5, 3.25, 3, 2.75, 2.5, 2.25, 2, 1.75, 1.5, 1.25, 1, .9, .8, .7, .6, .5, .4, .3, .2, .1, .05, .025)
intTimes = [5000000, 4750000, 4500000, 4250000, 4000000, 3750000, 3500000, 3250000, 3000000, 2750000, 2500000, 2250000, 2000000, 1750000, 1500000, 1250000, 1000000, 900000, 800000, 700000, 600000, 500000, 400000, 300000, 200000, 100000, 50000, 25000]

if os.path.isfile('/home/pi/Documents/Data_Files/DataExists.txt'):
	print("Prevented from Overwriting Integration Time Test Data")
else:
	print("Beginning Integration Time Test")

	i = 0

	while i < len(intTimes): #Run the intergationCalibration 28 times with each of the times in the array (This loop = 584 s = 9.75 minutes)
		systemCall = "/home/pi/Documents/seabreeze-3.0.11/SeaBreeze/sample-code/c/demo-integrationCalibration " + str(intTimes[i]) + " " + str(0)
		#Call program with arguments intTimes[i] and 0
		os.system(systemCall)
		i = i + 1

	sleep(600) #Delay 10 minutes to allow time to disconnect Spark Gap and to account for weird time variance in first loop

	i = 0

	while i < len(intTimes): #Run through again, but this time with spark gap off (This loop = 584 s = 9.75 minutes)
		systemCall = "/home/pi/Documents/seabreeze-3.0.11/SeaBreeze/sample-code/c/demo-integrationCalibration " + str(intTimes[i]) + " " + str(1)
		#Call program with arguments intTimes[i] and 1
		os.system(systemCall)
		i = i + 1

	f = open("/home/pi/Documents/Data_Files/DataExists.txt", "w+")
	f.write("Please don't overwrite the data in this directory, I worked hard to get it.")

print("Integration Time Test Complete")
