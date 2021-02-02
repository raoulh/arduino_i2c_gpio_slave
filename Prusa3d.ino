// Copyright: Raoul Hecky
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
//
//Libraries used:
// https://github.com/milesburton/Arduino-Temperature-Control-Library.git
// https://github.com/PaulStoffregen/OneWire

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

const byte I2C_ADDR = 0x08;

const byte PIN_ONEWIRE = 14;
const byte PIN_LIGHT = 2;

OneWire ds(PIN_ONEWIRE);
DallasTemperature sensors(&ds);

byte getTemperature(float *temperature, byte reset_search);

unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0;

uint8_t lightState = HIGH;

void setup()
{
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(PIN_LIGHT, OUTPUT);
    digitalWrite(PIN_LIGHT, HIGH);

    sensors.begin();
    //disable blocking code
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    delayInMillis = 750; 
    lastTempRequest = millis();
}

void loop()
{
    requestTemperature();
    digitalWrite(PIN_LIGHT, lightState);
}

void requestTemperature()
{
    if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
    {
        temperature = sensors.getTempCByIndex(0);
        sensors.requestTemperatures();
        lastTempRequest = millis(); 
    }
}

/* Action            | Operation | Register | Data to write
 * ------------------+-----------+----------+--------------
 * Set Light On      | Write     | 0x01     | 1
 * Set Light Off     | Write     | 0x01     | 0
 * Get Light state   | Read      | 0x01     |
 * Get Temp          | Read      | 0x02     |
 */

const byte REG_LIGHT = 0x01;
const byte REG_TEMP  = 0x02;

uint8_t opcode; // register

void receiveEvent(int numBytes)
{
    //read register
    opcode = Wire.read();

    if (numBytes > 1)
    {
        if (opcode == REG_LIGHT)
        {
            uint8_t data = Wire.read();
            if (data == 0 || data == 1) //discard non valid values
                lightState = !data;
        }
    }
}

void requestEvent()
{
    if (opcode == REG_LIGHT)
    {
        Wire.write(!lightState);
    }
    else if (opcode == REG_TEMP)
    {
        //Send temp as string
        char buff[2];
        buff[0] = temperature;
        buff[1] = (temperature - buff[0]) * 100.0;
        Wire.write(buff);
    }
}
