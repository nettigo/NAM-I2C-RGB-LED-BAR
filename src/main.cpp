#include <Arduino.h>
#include <tinyNeoPixel.h>
#include <TinyWireS.h>

#define NEOPIN 10               // NeoPixel Pin
#define NEONUM 10               // Number of pixels
#define I2C_SLAVE_ADDRESS 0x32  // I2C Slave Address

tinyNeoPixel pixels = tinyNeoPixel(NEONUM, NEOPIN, NEO_GRB + NEO_KHZ800);

#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 5 )
#endif

// The "registers" we expose to I2C
volatile uint8_t i2c_regs[] =
        {
                0x0,    // mode
                0x0,    // cnt
                0x0,    // red
                0x0,    // green
                0x0,    // blue
        };
const byte reg_size = sizeof(i2c_regs);
// Tracks the current register pointer position
volatile byte reg_position=0;
boolean needsUpdate = true;

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the
 * send-buffer when using this callback
 */
void requestEvent() {
    TinyWireS.send(NEONUM);
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop instead of running them directly,
 */
void receiveEvent(uint8_t howMany) {
    if ((howMany < 1) || (howMany > TWI_RX_BUFFER_SIZE)) {
        while (TinyWireS.available()) TinyWireS.receive();
        return;
    }

    while (howMany--) {
        i2c_regs[reg_position] = TinyWireS.receive();
//        if (reg_position == 0 // If it was the first register
//            && bitRead(i2c_regs[0], 7) // And the highest bit is set
//            && !ADC_ConversionInProgress() // and we do not actually have a conversion running already
//                ) {
//            start_conversion = true;
//        }
        reg_position++;
        if (reg_position >= reg_size) {
            reg_position = 0;
        }
    }
    needsUpdate = true;
    return;
}

void testPixels() {
    for (byte i = 0; i < NEONUM; i++) {
        pixels.setPixelColor(i, pixels.Color(30,0,0));
        pixels.show();
        delay(35);
    }
    for (byte i = 0; i < NEONUM; i++) {
        pixels.setPixelColor(i, pixels.Color(0,30,0));
        pixels.show();
        delay(35);
    }
    for (byte i = 0; i < NEONUM; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 30));
        pixels.show();
        delay(35);
    }
    for (byte i = 0; i < NEONUM; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        pixels.show();
        delay(35);
    }
}


void updatePixels() {
    if (needsUpdate) {

        byte mode = i2c_regs[0];
        byte cnt = i2c_regs[1];
        byte r = i2c_regs[2];
        byte g = i2c_regs[3];
        byte b = i2c_regs[4];

        if (mode == 0) {
            // Light up first {cnt} LEDs with {r,g,b} color

            for (byte i = 0; i < cnt; i++) {
                pixels.setPixelColor(i, pixels.Color(r,g,b));
            }

            for (byte i = cnt; i < NEONUM; i++) {
                pixels.setPixelColor(i, pixels.Color(0,0,0));
            }
            pixels.show();

        } else if (mode == 1) {
            // Light up single {cnt} LED with {r,g,b} color

            pixels.setPixelColor((cnt - 1), pixels.Color(r,g,b));
            pixels.show();
        }
        needsUpdate = false;
    }
}

void setup() {
    pixels.begin();
    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);
    testPixels();
}

void loop() {

    updatePixels();
    tws_delay(10); // Wait 500 ms
//    TinyWireS_stop_check();
    return;

}
