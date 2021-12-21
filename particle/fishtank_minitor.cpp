/*

Sample project based on a template provided by Particle.

I used this to measure temperature in a fish tank at home and push the data to their
cloud service to chart temperature fluctuations in our freshwater tank. The idea was to
gradually expand the sensors to include pH, and others, to create a battery-powered
IoT device to wireless transmit the data, but this started with a water-safe 
temperature probe. (the other sensors were too expensive for a hobbyist-level project)





Use this sketch to find the address(es) of any 1-Wire devices
you have attached to your Particle device (core, p0, p1, photon, electron)

It is an example of the enumeration process to identify all devices attached
to the 1-wire bus. It will identify all 'known' devices like DS18B20.
Normally you would save all the found addresses in an array for later use. This
code just prints the info out to the serial port.

Pin setup:
These made it easy to just 'plug in' my 18B20

D3 - 1-wire ground, our just use regular pin and comment out below.
D4 - 1-wire signal, 2K-10K resistor to...
D5 - 1-wire power, ditto ground comment.

A pull-up resistor is required on the signal line. The spec calls for a 4.7K.
I have used 1K-10K depending on the bus configuration and what I had out on the
bench. If you are powering the device, they all work. If you are using parisidic
power it gets more picky about the value. I probably use 10K the most.

*/
#include "Particle.h"
// Only include One of the following depending on your environment!
#include "OneWire/OneWire.h"  // Use this include for the Web IDE:
// #include "OneWire.h" // Use this include for Particle Dev where everything is in one directory.

OneWire ds = OneWire(D0);  // 1-wire signal on pin D4

unsigned long lastUpdate = 0;
int minutes = 60 * 1000;
unsigned long MS_WAIT = minutes * 1; // number of minutes

unsigned long samples = 0;

void setup() {
    Serial.begin(9600);
    // Set up 'power' pins, comment out if not used!
//   pinMode(D3, OUTPUT);
//   pinMode(D5, OUTPUT);
//   digitalWrite(D3, LOW);
//   digitalWrite(D5, HIGH);
}

// Every 3 seconds check for the next address on the bus
// The scan resets when no more addresses are available

void loop() {

    byte addr[8];
    unsigned long now = millis();

    if (samples == 0 || (now - lastUpdate) > MS_WAIT) {
        lastUpdate = now;
        ds.search(addr);

        // if we get here we have a valid address in addr[]
        // you can do what you like with it
        // see the Temperature example for one way to use
        // this basic code.

        // the first ROM byte indicates which chip family
        switch (addr[0]) {
            case 0x28:
                // Particle.publish("device", "Chip = DS18B20 Temp sensor");
                break;
            default:
                Particle.publish("device", "Device type is unknown.");
                // Just dumping addresses, show them all
                return;  // uncomment if you only want a known type
        }

        samples += getTemp(addr);
        ds.reset(); // clear bus for next use
    }
    delay(1000);
}


char * rom_addr(byte addr[8]) {

    int size = sizeof(addr);
    int i;
    char *buf_str = (char *) malloc(2 * size + 1);
    char *buf_ptr = buf_str;
    for (i = 0; i < size; i++) {
        buf_ptr += sprintf(buf_ptr, "%02X", addr[i]);
    }
    sprintf(buf_ptr, "\n");
    *(buf_ptr + 1) = '\0';
    return buf_str;

}

float getTemp(byte addr[8]) {
    //returns the temperature from one DS18S20 in DEG Fahrenheit

    byte data[12];
    byte payload[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//   if ( !ds.search(addr)) {
//       //no more sensors on chain, reset search
//       ds.reset_search();
//       Particle.publish("debug", "no sensor found");
//       return -1000;
//   }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Particle.publish("debug", "CRC is not valid!");
        return 0;
    }

    if (addr[0] != 0x10 && addr[0] != 0x28) {
        Particle.publish("debug", "Device is not recognized");
        return 0;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end

    ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad

    for (int i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = ds.read();
    }

    ds.reset_search();

    byte MSB = data[1];
    byte LSB = data[0];

    payload[0] = data[0];
    payload[1] = data[1];
    // Particle.publish("payload", rom_addr(payload));

    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    float TemperatureSum = tempRead / 16;
    float fTemp = (TemperatureSum * 18 + 5) / 10 + 32;

    // sprintf((char *)data, "%0.2f", tempRead);
    // Particle.publish("raw temp read", (char *)data);
    sprintf((char *)data, "%0.2f F", fTemp);
    Particle.publish("temp", (char *)data);
    sprintf((char *)data, "%0.2f C", TemperatureSum);
    Particle.publish("temp", (char *)data);

    return 1;
}
