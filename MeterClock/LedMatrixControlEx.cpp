#include "LedMatrixControlEx.h"

// these are the opcodes for the MAX7221 and MAX7219 - refer datasheet
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

#define BUFFER_MAIN 0
#define BUFFER_UPPER 1
#define BUFFER_LOWER 2

#include <stdarg.h>
void printout(char *fmt, ...) {
  #if 0
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.print(buf);
  #endif
}

LedMatrixControl::LedMatrixControl(int dataPin, int clkPin, int csPin, int numModules) 
{
    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS=csPin;

    // always at most 8 matrix modules.
    if (numModules <= 0 || numModules > MAX_MAX_MODULES )
        numModules = MAX_MAX_MODULES;

    maxModules = numModules;
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_CLK, OUTPUT);
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);

    // turn any display 'testing' OFF by sending a zero 
    spiTransferToAll("DISPLAYTEST", OP_DISPLAYTEST, 0);
    // scanlimit is set to the maximum (value 7) which is for 8 "digits", where a digit is the 7-seg LED. <-- this is a bit confusing but trust me.
    spiTransferToAll("SCANLIMIT", OP_SCANLIMIT, 7);
    // for 8x8 LED matrix modules we want all decoding done by use as we are not dealing with 7-seg LEDs
    spiTransferToAll("DECODEMODE", OP_DECODEMODE, 0);
    // clear the display
    clearDisplay();
    // now we get out of shutdown-mode on startup
    spiTransferToAll("SHUTDOWN", OP_SHUTDOWN, 0);
}

int LedMatrixControl::getModuleCount() {
    return maxModules;
}

void LedMatrixControl::shutdown(bool b) 
{
    if (b)
      spiTransferToAll("SHUTDOWN", OP_SHUTDOWN, 0);
    else
      spiTransferToAll("SHUTDOWN", OP_SHUTDOWN, 1);
}

void LedMatrixControl::setIntensity(int intensity) 
{
    if (intensity >= 0 && intensity < 16) 
      spiTransferToAll("INTENSITY", OP_INTENSITY, intensity);
}

void LedMatrixControl::clearDisplay()
{
  for (int module = 0; module < maxModules; module++)
  {
    for (int row = 0; row <= 7; row++)
    {
      frame[BUFFER_MAIN][module][row] = 0;
    }
  }
  updateDisplay();
}

void LedMatrixControl::spiTransfer(int addr, volatile byte opcode, volatile byte data) 
{
  // fill with NOOPs first
  for (int module = 0; module < maxModules; module++)
  {
    spidata[module][0] = (byte)OP_NOOP;
    spidata[module][1] = (byte)0;
  }
  
  // put our module data into the array
  spidata[addr][0]=opcode;
  spidata[addr][1]=data;
  spiWrite("spiTransfer");
}   

void LedMatrixControl::spiTransferToAll(const char *op_name, volatile byte opcode, volatile byte data) 
{
  for (int module = 0; module < maxModules; module++)
  {
    spidata[module][0] = opcode;
    spidata[module][1] = data;
  }
  spiWrite(op_name);
}

void LedMatrixControl::dumpFrameBuffer()
{
  for (byte row = 1; row <= 7; row++)
  {  
    for (int module = 0; module < maxModules; module++)
    {
      printout("0x%02x ", frame[BUFFER_MAIN][module][row]);
    }
    printout("\n");
  }
}

void LedMatrixControl::dumpLedMatrix()
{
  for (byte row = 1; row <= 7; row++)
  {  
    for (int module = 0; module < maxModules; module++)
    {
      for (int abit = 0x80; abit > 0; abit = abit >> 1)
      {
        printout("%c", frame[BUFFER_MAIN][module][row] & abit ? 'X' : '.');
      }
      printout(" ");
    }
    printout("\n");
  }
}


void LedMatrixControl::shiftLeftFrameBuffer()
{
  for (byte row = 1; row <= 7; row++)
    {
    for (int module = 0; module < maxModules; module++)
    {
      // ADAM it is left shiting not right need to mask 0x80 !!!!
      frame[BUFFER_MAIN][module][row] = (frame[BUFFER_MAIN][module][row] << 1) & 0xfe;
      if (frame[BUFFER_MAIN][module + 1][row] >> 7 == 0x01)
      {
        //printout("shiftLeftFrameBuffer m=%d,r=%d frame=0x%x carried=0x%x from m=%d\n", module, row, frame[module][row], 
        //  frame[module + 1][row], module + 1);
        frame[BUFFER_MAIN][module][row] |= 0x01;        
      }
    }
  }
}

/**
 * Shuffles from the upper buffer to the visible buffer - just the last line. call 7 times to complete shuffle
 */
void LedMatrixControl::shuffleDownFrameBuffer()
{
  for (int module = 0; module < maxModules; module++) {
    for (byte row = 7; row >0; row--) {
      // copy the one just above
      frame[BUFFER_MAIN][module][row] = frame[BUFFER_MAIN][module][row - 1];
      }
    // copy down the upper frame buffer last item to the first item one
    frame[BUFFER_MAIN][module][0] = frame[BUFFER_UPPER][module][7];
    
    for (byte row = 7; row >0; row--) {
      frame[BUFFER_UPPER][module][row] = frame[BUFFER_UPPER][module][row - 1];
      }    
    // clear the top of the upper buffer
    frame[BUFFER_UPPER][module][0] = 0x00;      
   }
}

/**
 * Shuffles from the lower buffer to the visible buffer - just the top line. call 7 times to complete shuffle
 */
void LedMatrixControl::shuffleUpFrameBuffer()
{
  for (int module = 0; module < maxModules; module++) {
    for (byte row = 0; row <7; row++) {
      // copy the one just below
      frame[BUFFER_MAIN][module][row] = frame[BUFFER_MAIN][module][row + 1];
      }
    // copy up the lower frame buffer first item to the last item one
    frame[BUFFER_MAIN][module][7] = frame[BUFFER_LOWER][module][0];
    
    for (byte row = 0; row <7; row++) {
      frame[BUFFER_LOWER][module][row] = frame[BUFFER_LOWER][module][row + 1];
      }    
    // clear the bottom of the lower buffer
    frame[BUFFER_LOWER][module][7] = 0x00;      
   }
}

void LedMatrixControl::drawCharInFrameBuffer(int x, char character)
{
  drawCharInOneFrameBuffer(BUFFER_MAIN, x, character);
}


void LedMatrixControl::drawCharInOneFrameBuffer(int buffer_index, int x, char character)
{
  int module = x / 8;
  int start_col_on_module = x % 8;
  int width = 5; 
  // used for clearing the background before we place the letter. 
  byte background = 0xf8; // ones for the upper 5 bits, 0's for the lower 3

  printout("drawCharInFrameBuffer x=%d character=0x%x\n", x, character);

  // Module 0 | Module 1 | Module 2 | Module 3 |
  // MSB  LSB | MSB  LSB | MSB  LSB | MSB  LSB |
  // 87654321 | 87654321 | 87654321 | 87654321 |

  // each font item is seven rows tall
  for (byte row = 1; row <= 7; row++)
  {
    byte v;

    if (character >= 0x20 && character < 0x40) 
    {
      v = pgm_read_byte_near(fontTable1 + (character - 0x20) * 7 + row - 1);
    }
    else if (character >= 0x40 && character < 0x60)
    {
      v = pgm_read_byte_near(fontTable2 + (character - 0x40) * 7 + row - 1);
    }
    else if (character >= 0x60 && character < 0x80)
    {
      v = pgm_read_byte_near(fontTable3 + (character - 0x60) * 7 + row - 1);
    }
    // all characters neeed to move to the right 3
    v = v << 3;

    printout("frame[m=%d][r=%d] v 0x%0x start_col_on_module=%d\n",module, row, v, start_col_on_module);
    frame[buffer_index][module][row] &= ~(background >> start_col_on_module);
    frame[buffer_index][module][row] |= v >> start_col_on_module;
    if (start_col_on_module + width > 8)
    {
      if (module == maxModules)
        return;
      frame[buffer_index][module + 1][row] &= ~(background << (8 - start_col_on_module));
      frame[buffer_index][module + 1][row] |= v << (8 - start_col_on_module);
      printout("also on frame[m=%d][r=%d] = 0x%0x\n", module + 1, row, frame[buffer_index][module + 1][row]);
    }
  }
}

void LedMatrixControl::drawCharInUpperFrameBuffer(int x, char character)
{
  drawCharInOneFrameBuffer(BUFFER_UPPER, x, character);
}

void LedMatrixControl::drawCharInLowerFrameBuffer(int x, char character)
{
  drawCharInOneFrameBuffer(BUFFER_LOWER, x, character);
}

void LedMatrixControl::updateDisplay() 
{
  // row is equivalent to "digit" in the MAX 7219 data sheet
  for (int row = 7; row >= 0; row--)
  {
    for (int module = 0; module < maxModules; module++)
    {
      // MSByte has the row that this message is for, "0" is NOOP the first row is "1", the last is 8
      spidata[maxModules - module - 1][0] = row + 1;
      // LSByte has the 8 bit data of which LEDs are going to be 'lit' across the row 
      spidata[maxModules - module - 1][1] = frame[BUFFER_MAIN][module][row];
    }
    spiWrite("updateDisplay");
  }
}


void LedMatrixControl::clearFrameBuffer() {
  clearOneFrameBuffer(BUFFER_MAIN);
}

void LedMatrixControl::clearOneFrameBuffer(int buffer_index)
{
  for (int module = 0; module < maxModules; module++)
  {
    for (int row = 0; row <= 7; row++)
    {
      frame[buffer_index][module][row] = 0;
    }
  }
}

void LedMatrixControl::clearUpperFrameBuffer()
{
  clearOneFrameBuffer(BUFFER_UPPER);
}

void LedMatrixControl::clearLowerFrameBuffer()
{
  clearOneFrameBuffer(BUFFER_LOWER);
}

void LedMatrixControl::spiWrite(const char *op_name)
{
  // 'Chip Select' enable the line 
  digitalWrite(SPI_CS, LOW);

  // Shift out the data
  for (int module = maxModules - 1; module >= 0; module--)
  {
    shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, spidata[module][0]);
    shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, spidata[module][1]);
  }

  // latch data onto the display
  digitalWrite(SPI_CS, HIGH);

  //if (op_name == "updateDisplay")
  //{
  //  Serial.print("spi write for ");
  //  Serial.print(op_name);
  //  Serial.print(" : ");
  //  for (int module = maxModules - 1; module >= 0; module--)
  //  {
  //    printout("%0x %0x  ", spidata[module][0], spidata[module][1]);
  //  }
  //  Serial.println("");
  //}
}
 
void LedMatrixControl::serialPrintBuffer() 
{
  for (int row = 0; row <= 7; row++)
  {
    for (int module = 0; module < maxModules; module++)
    {
      byte this_row = frame[module][row];
      Serial.print((this_row & 0x80) == 0x80 ? "o" : ".");
      Serial.print((this_row & 0x40) == 0x40 ? "o" : ".");
      Serial.print((this_row & 0x20) == 0x20 ? "o" : ".");
      Serial.print((this_row & 0x10) == 0x10 ? "o" : ".");
      Serial.print((this_row & 0x08) == 0x08 ? "o" : ".");
      Serial.print((this_row & 0x04) == 0x04 ? "o" : ".");
      Serial.print((this_row & 0x02) == 0x02 ? "o" : ".");
      Serial.print((this_row & 0x01) == 0x01 ? "o" : ".");
    }
    Serial.println("");
  }
}
