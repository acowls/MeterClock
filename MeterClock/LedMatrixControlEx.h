#pragma once

#include <avr/pgmspace.h>
#include <Arduino.h>

// There are three font tables of characters. All are 7H 5W characters
// Obtained verbatim from here: http://www.ccsinfo.com/forum/viewtopic.php?p=24132

// Declare and initialise the font array data 
const static byte fontTable1[] PROGMEM{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 0x20, Space 
   0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04,    // 0x21, ! 
   0x09, 0x09, 0x12, 0x00, 0x00, 0x00, 0x00,    // 0x22, " 
   0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a,    // 0x23, # 
   0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04,    // 0x24, $ 
   0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13,    // 0x25, % 
   0x04, 0x0a, 0x0a, 0x0a, 0x15, 0x12, 0x0d,    // 0x26, & 
   0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,    // 0x27, ' 
   0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02,    // 0x28, ( 
   0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08,    // 0x29, ) 
   0x04, 0x15, 0x0e, 0x1f, 0x0e, 0x15, 0x04,    // 0x2a, * 
   0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00,    // 0x2b, + 
   0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08,    // 0x2c, , 
   0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00,    // 0x2d, - 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c,    // 0x2e, . 
   0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10,    // 0x2f, / 
   0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e,    // 0x30, 0 
   0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e,    // 0x31, 1 
   0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f,    // 0x32, 2 
   0x0e, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0e,    // 0x33, 3 
   0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02,    // 0x34, 4 
   0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e,    // 0x35, 5 
   0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e,    // 0x36, 6 
   0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08,    // 0x37, 7 
   0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e,    // 0x38, 8 
   0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c,    // 0x39, 9 
   0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00,    // 0x3a, : 
   0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x04, 0x08,    // 0x3b, ; 
   0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02,    // 0x3c, < 
   0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00,    // 0x3d, = 
   0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08,    // 0x3e, > 
   0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04,    // 0x3f, ? 
};

const static byte fontTable2[] PROGMEM = {
   0x0e, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0f,    // 0x40, @ 
   0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11,    // 0x41, A 
   0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e,    // 0x42, B 
   0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e,    // 0x43, C 
   0x1e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1e,    // 0x44, D 
   0x1f, 0x10, 0x10, 0x1c, 0x10, 0x10, 0x1f,    // 0x45, E 
   0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10,    // 0x46, F 
   0x0e, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0f,    // 0x37, G 
   0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11,    // 0x48, H 
   0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e,    // 0x49, I 
   0x1f, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c,    // 0x4a, J 
   0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11,    // 0x4b, K 
   0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f,    // 0x4c, L 
   0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11,    // 0x4d, M 
   0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11,    // 0x4e, N 
   0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e,    // 0x4f, O 
   0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10,    // 0x50, P 
   0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d,    // 0x51, Q 
   0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11,    // 0x52, R 
   0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e,    // 0x53, S 
   0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,    // 0x54, T 
   0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e,    // 0x55, U 
   0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04,    // 0x56, V 
   0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11,    // 0x57, W 
   0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11,    // 0x58, X 
   0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04,    // 0x59, Y 
   0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f,    // 0x5a, Z 
   0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e,    // 0x5b, [ 
   0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01,    // 0x5c, \ 
   0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e,    // 0x5d, ] 
   0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00,    // 0x5e, ^ 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,    // 0x5f, _ 
  };

const static byte fontTable3[] PROGMEM = {
   0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00,    // 0x60, ` 
   0x00, 0x0e, 0x01, 0x0d, 0x13, 0x13, 0x0d,    // 0x61, a 
   0x10, 0x10, 0x10, 0x1c, 0x12, 0x12, 0x1c,    // 0x62, b 
   0x00, 0x00, 0x00, 0x0e, 0x10, 0x10, 0x0e,    // 0x63, c 
   0x01, 0x01, 0x01, 0x07, 0x09, 0x09, 0x07,    // 0x64, d 
   0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0f,    // 0x65, e 
   0x06, 0x09, 0x08, 0x1c, 0x08, 0x08, 0x08,    // 0x66, f 
   0x0e, 0x11, 0x13, 0x0d, 0x01, 0x01, 0x0e,    // 0x67, g 
   0x10, 0x10, 0x10, 0x16, 0x19, 0x11, 0x11,    // 0x68, h 
   0x00, 0x04, 0x00, 0x0c, 0x04, 0x04, 0x0e,    // 0x69, i 
   0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c,    // 0x6a, j 
   0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12,    // 0x6b, k 
   0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,    // 0x6c, l 
   0x00, 0x00, 0x0a, 0x15, 0x15, 0x11, 0x11,    // 0x6d, m 
   0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11,    // 0x6e, n 
   0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e,    // 0x6f, o 
   0x00, 0x1c, 0x12, 0x12, 0x1c, 0x10, 0x10,    // 0x70, p 
   0x00, 0x07, 0x09, 0x09, 0x07, 0x01, 0x01,    // 0x71, q 
   0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10,    // 0x72, r 
   0x00, 0x00, 0x0f, 0x10, 0x0e, 0x01, 0x1e,    // 0x73, s 
   0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06,    // 0x74, t 
   0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d,    // 0x75, u 
   0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04,    // 0x76, v 
   0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0a,    // 0x77, w 
   0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11,    // 0x78, x 
   0x00, 0x11, 0x11, 0x0f, 0x01, 0x11, 0x0e,    // 0x79, y 
   0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f,    // 0x7a, z 
   0x06, 0x08, 0x08, 0x10, 0x08, 0x08, 0x06,    // 0x7b, { 
   0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04,    // 0x7c, | 
   0x0c, 0x02, 0x02, 0x01, 0x02, 0x02, 0x0c,    // 0x7d, } 
   0x08, 0x15, 0x02, 0x00, 0x00, 0x00, 0x00,    // 0x7e, ~ 
   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,    // 0x7f, DEL 
  };

#define MAX_MAX_MODULES 4

class LedMatrixControl {
    private :
        // this array holds the data that is shifted out to the LED drivers
        byte spidata[MAX_MAX_MODULES][2];

        // sends an opcode command to just one of the LED matrix modules (and NOOPs to the rest)
        void spiTransfer(int addr, byte opcode, byte data);

        // sends an opcode command to all the LED matrix modules. 
        void spiTransferToAll(const char *op_name, byte opcode, byte data);

        // data pin
        int SPI_MOSI;

        // clock pin
        int SPI_CLK;

        // chip select pin - active LOW
        int SPI_CS;

        // the maximum number of module devices in use. A module is an 8x8 module of LEDs. Never more than 8.
        int maxModules;

        // the frame buffer - each byte is a row of LEDs in a [8x8 matrix module][row] format

        // three buffers 0 = main, 1 = above, 2 = below
        byte frame[3][MAX_MAX_MODULES][8];

        // write our the SPI stream of 8 bytes.
        // op_name is used for tracing (debug) purposes
        void spiWrite(const char *op_name);

        void drawCharInOneFrameBuffer(int buffer_index, int x, char character);
        void clearOneFrameBuffer(int buffer_index);
        
    public:

      // Creates a LedMatrixController for controlling from 1 to 8 LED 8x8 matrix modules 
      // that are driven by MAX7219 or MAX722 controllers. 
      // dataPin - the pin on the Arduino used for shifting out the data, look for "DIN" on some PCBs
      // clockPin - the pin on the Arduino used for clocking the data out, look for "CLK" on some PCBs
      // csPin - chip select pin, often labelled as "CS" on the PCB
      // numModules - the number of 8x8 LED modules that are connected (1 is default, 8 is max)
      LedMatrixControl(int dataPin, int clkPin, int csPin, int numModules = 1);

      // Gets the number of 8x8 LED matrix moduels that are connected
      int getModuleCount();

      // Draws one of the 7H 5W characters in to the frame buffer
      // x is the x-axis offset 0 to 31 (for a 4 module display)
      void drawCharInFrameBuffer(int x, char character);

      void drawCharInUpperFrameBuffer(int x, char character);

      void drawCharInLowerFrameBuffer(int x, char character);

      // shift the frame buffer one dot to the left.
      void shiftLeftFrameBuffer();

      void shuffleDownFrameBuffer();

      void shuffleUpFrameBuffer();

      void dumpFrameBuffer();

      void dumpLedMatrix();

      // Display the buffer on to the group of LED matrix module(s)
      void updateDisplay();

      void clearFrameBuffer();

      void clearUpperFrameBuffer();

      void clearLowerFrameBuffer();

      // puts the buffer on to the serial port so we can see what the LED matrix looks like. 
      void serialPrintBuffer();

      // Sets the shutdown (power saving) mode for the device. 
      // true - device goes into power-down mode. false - normal operation.
      void shutdown(bool status);

      // Sets the display intensity brightness (0..15)
      void setIntensity(int intensity);

      // Turn all the leds off on the display
      void clearDisplay();
};
