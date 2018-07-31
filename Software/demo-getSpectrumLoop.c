/*
* This program serves to request data from the spectrometer and write that data as a series of doubles to files (1 spectrum per file) where each
* file is titled "SpecData_[INTEGER VALUE].txt" (each spectrum is numbered sequentially, starting at 0). This program will request spectrums
* immediately following one another infinitely. One request cycle takes the value of the camera's integration time. During one cycle, the program
* will determine whether the given cycle is even or odd. During an even cycle the program will set the spark gap trigger pin to HIGH and then
* request a spectrum from the camera. Once this spectrum request is complete the program will set the spark gap trigger pin to LOW and write
* the spectrum data to a file. During an odd cycle the process is the same except the trigger pin will never be set HIGH. The process of setting
* the trigger pin is done via OS system calls. These calls are to external python scripts which serve to set the spark gap trigger pin either
* HIGH or LOW. Upon this program being called, the first spectrum request will begin 0.852 seconds later.
*
* -Created for the Aether Payload flying on RockSat-XN in January of 2019 by Sam Lawson
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h> //Added for timestamping data
#include "api/SeaBreezeWrapper.h"
#include "util.h"

//Program Settings
int trigger_mode = 0; //Sets the camera's trigger mode (0 = default = only setting that doesn't require an external trigger through the breakout board)
int integration_time = 3500000; //Stare time of camera (in microseconds)
char directory[40] = "/home/pi/Documents/Data_Files/"; //File path to where data .txt files are to be stored
int triggerPin = 26; //The pin that will trigger the spark gap NOTE THAT THIS IS THE GPIO PIN NUMBER, NOT THE BOARD PIN NUMBER
char turnOnCode[50] = "/home/pi/Documents/codeTesting/sparkGapOn.py"; //Name and filepath of python script to activate spark gap pin
char turnOffCode[50] = "/home/pi/Documents/codeTesting/sparkGapOff.py"; //Name and filepath of python script to de-activate spark gap pin

int error = 0;      // global for convenience error-handling function
char turnOn[55]; //Will hold the OS system() call for activating the spark gap trigger pin
char turnOff[55]; //Will hold the OS system() call for de-activating the spark gap trigger pin

time_t rawtime; //Global time variable

int saveToFile(int dataNum, int pixels, unsigned char *spectrum, char *timeStamp){ //Takes a byte spectrum, converts it to doubles, and writes it to a file
    unsigned char lsb, msb;
    unsigned int counts;
    double* buffer = (double*) malloc(pixels * sizeof(double));

    //Creates and opens (for writing) a file called SpecData_[dataNum].txt
    FILE *data;
    char fileName[60];
    snprintf(fileName, sizeof(fileName), "%sSpecData_%d.txt", directory, dataNum);
    data = fopen(fileName, "w");

    fprintf(data, timeStamp); //Timestamps data
    if((dataNum % 2) == 0){ //Indicates whether plasma was on or off when data was taken
	fprintf(data, "PLASMA\n"); //Plasma on
    }else{
	fprintf(data, "BACKGROUND\n"); //Plasma off
    }
    for(int i = 0; i < pixels; i++){ //Takes the high and low bytes of each pixel, converts them into one double value, and writes that value to file
	lsb = spectrum[i*2];
	msb = spectrum[i*2 + 1];
	counts = (msb << 8) + lsb;
	buffer[i] = counts;
	fprintf(data, "P: %4d: %8u\n", i, counts); //Print line of data (1 pixel worth) to file
    }
    fclose(data); //Close file

    return 0;
}

int getASpectrum(int dataNum, int index, int raw_length) { //Requests a spectrum, converts its values to doubles, and writes them to a file (1 file per spectrum). Also activates the spark gap for every other spectrum
    //Variables for use in timestamping the data
    struct tm *timeStamper;
    char timeStamp[30];

    unsigned char *bSpectrum = (unsigned char*) malloc(raw_length); //Allocate storage space to hold a spectrum

    if((dataNum % 2) == 0){ //Turns the spark gap on for even numbered spectrums
	system(turnOn); //Call to python script which sets spark gap trigger pin to 1
    }

    //Get unformatted spectrum with its read time
    seabreeze_get_unformatted_spectrum(index, &error, bSpectrum, raw_length); //Loads spectrum data into bSpectrum
    time(&rawtime); //Updates rawtime
    timeStamper = localtime(&rawtime); //Converts rawtime to local format and loads into timeStamper
    snprintf(timeStamp, sizeof(timeStamp), asctime(timeStamper)); //Puts data of timeStamper into a char array timeStamp

    system(turnOff); //Call to python script which sets spark gap trigger pin to 0

    // strip synchronization byte if present (not all devices have one)
    if (raw_length % 2) {
        raw_length--;  // ignore last byte hereafter
    }

    int pixels = raw_length / 2; //The spectrum returns 2 bytes per pixel

    saveToFile(dataNum, pixels, bSpectrum, timeStamp); //Send spectrum to function to be converted and written to file

    free((void*) bSpectrum); //Un-allocate memory for bSpectrum

    return 0;
}

int main(int argc, char **argv){ //Applies settings to spectrometer and repeatedly calls get_unformatted_spectrum_doubles()
    //Parses data into char arrays for OS system() calls
    snprintf(turnOn, sizeof(turnOn), "%s %d", turnOnCode, triggerPin);
    snprintf(turnOff, sizeof(turnOff), "%s %d", turnOffCode, triggerPin);

    system(turnOff); //Call to python script which sets spark gap trigger pin to 0

    int raw_length, i;
    i = 0;

    seabreeze_open_spectrometer(0, &error); //Opens the spectrometer

    seabreeze_set_integration_time_microsec(0, &error, integration_time); //Sets the integration time (see program settings above)
    check_error(0, &error, "seabreeze_set_integration_time_microsec");

    seabreeze_set_trigger_mode(0, &error, trigger_mode); //Sets trigger mode (see program settings above)
    check_error(0, &error, "seabreeze_set_trigger_mode");

    raw_length = seabreeze_get_unformatted_spectrum_length(0, &error); //Gets value for raw_length (number of bytes it will take to store a spectrum)
    check_error(0, &error, "seabreeze_get_unformatted_spectrum_length");

    while(1){ //Repeatedly call get_unformatted_spectrum_doubles()
	getASpectrum(i, 0, raw_length);
        if (check_error(0, &error, "seabreeze_get_spectrum_doubles")){
            continue;
        }
	i++;
    }

    seabreeze_close_spectrometer(0, &error); //Closes spectrometer
    check_error(0, &error, "seabreeze_close_spectrometer");
    system(turnOff); //Call to python script which sets spark gap trigger pin to 0 (should we somehow exit the loop, it will be off for safety)

    return 0;
}
