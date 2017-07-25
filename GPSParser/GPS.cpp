//
//  GPS.cpp
//  ArduinoGPSParsingLibrary
//
//  Created by Chris Blust on 7/5/17.
//  Copyright © 2017 Chris Blust. All rights reserved.
//
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "GPS.h"

GPS::GPS(){
    longDeg = 0.0;
    longMin = 0.0;
    latDeg = 0.0;
    latMin = 0.0;
    velocity = 0.0;
    angle = 0.0;
}

void GPS::setupSerial(int rx, int tx){
    gpsSerial = new SoftwareSerial(rx, tx);
    gpsSerial->begin(9600);
    sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
    sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
}

void GPS::sendCommand(const char *str){
    gpsSerial->println(str);
}

void GPS::update(){
    sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
    sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    char c = 0;
    //wait for start
    String prefix = "";
    do{
        while(c != '$'){
            if(gpsSerial->available()){
                c = gpsSerial->read();
                
            }
        }
        prefix = c;
        int index = 0;
        while(index < 5){
            if(gpsSerial->available()){
                c = gpsSerial->read();
                prefix += c;
                index++;
            }
        }
        
    }while(prefix != "$GPRMC");
    sentence = prefix;
    c = 0;
    //parse till end
    bool shouldAppend = false;
    do{
        
        if(shouldAppend){
            sentence += c;
        }
        if(gpsSerial->available()){
            c = gpsSerial->read();
            shouldAppend = true;
        }else{
            shouldAppend = false;
        }
        
        
    }while(c != '$');
    //Serial.println(sentence);
    sendCommand(PMTK_SET_NMEA_OUTPUT_OFF );
    //Serial.println(sentence);
    //now extract the data
    //skip to start of latitude
    sentence = sentence.substring(sentence.indexOf('A') + 2);
    //get the degrees
    latDeg = sentence.substring(0, 2).toFloat();
    //get the ones and tens place of min
    latMin = sentence.substring(2,4).toFloat();
    //move past decimal point
    sentence = sentence.substring(sentence.indexOf('.') + 1);
    //get the 4 decimal places
    latMin += ("." + sentence.substring(0,4)).toFloat();
    
    //skip to start of longitude
    sentence = sentence.substring(sentence.indexOf('N') + 2);
    //get the degrees
    longDeg = sentence.substring(0, 3).toFloat();
    //get the ones and tens place of min
    longMin = sentence.substring(3,5).toFloat();
    //move past decimal point
    sentence = sentence.substring(sentence.indexOf('.') + 1);
    //get the 4 decimal places
    longMin += ("." + sentence.substring(0,4)).toFloat();
    //skip to start of velocity
    sentence = sentence.substring(sentence.indexOf('W') + 2);
    velocity = 1.151 * sentence.substring(0, sentence.indexOf(',')).toFloat();
    //skip to start of track angle
    sentence = sentence.substring(sentence.indexOf(',')+1);
    angle = sentence.substring(0, sentence.indexOf(',')).toFloat();
}

double GPS::getLatitude(){
    return latDeg + (latMin/60);
}

double GPS::getLongitude(){
    return longDeg + (longMin/60);
}
