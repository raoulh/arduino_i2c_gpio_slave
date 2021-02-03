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
const byte PIN_PRINTER = 3;

OneWire ds(PIN_ONEWIRE);
DallasTemperature sensors(&ds);

byte getTemperature(float *temperature, byte reset_search);

unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0;

uint8_t lightState = HIGH;
uint8_t printerState = HIGH;

void setup()
{
//    Serial.begin(115200);

    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(PIN_LIGHT, OUTPUT);
    digitalWrite(PIN_LIGHT, HIGH);

    pinMode(PIN_PRINTER, OUTPUT);
    digitalWrite(PIN_PRINTER, HIGH);

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
    digitalWrite(PIN_PRINTER, printerState);
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
 * Set Printer On    | Write     | 0x03     | 1
 * Set Printer Off   | Write     | 0x03     | 0
 * Get Printer state | Read      | 0x03     |
 */

const byte REG_LIGHT    = 0x01;
const byte REG_TEMP     = 0x02;
const byte REG_PRINTER  = 0x03;

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
        else if (opcode == REG_PRINTER)
        {
            uint8_t data = Wire.read();
            if (data == 0 || data == 1) //discard non valid values
                printerState = !data;
        }
    }
}

union floatBytes
{
    uint8_t bytes[4];
    float value; 
} floatContainer;

void requestEvent()
{
    if (opcode == REG_LIGHT)
    {
        Wire.write(!lightState);
    }
    else if (opcode == REG_PRINTER)
    {
        Wire.write(!printerState);
    }
    else if (opcode == REG_TEMP)
    {
/*        Serial.print("Read temp: ");
        Serial.print(temperature);
        Serial.println("");
        */
        floatContainer.value = temperature;
        Wire.write(floatContainer.bytes, sizeof(float));
    }
}
