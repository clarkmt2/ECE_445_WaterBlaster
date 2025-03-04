/*
 * Newhaven Display Slim OLED Driver
 * ---------------------------------
 *
 * See header file for info and license
 *
 */



#include "newhaven_slim_oled.h"


// Display Geometry
uint8_t OLED_DISP_ROWS = 4;
uint8_t OLED_DISP_COLUMNS = 20;


// NHD_OLED_begin
//
// Performs all required initialization steps in a single command.
//
// NOTE: Define pin OLED_MOSI for data, and OLED_CLK for clock, in the
// header file.
//
// Parameters:
//    rows: number of rows/lines on the display.
//    columns: number of columns/characters per line on the display.
//
void NHD_OLED_begin()
{
NHD_OLED_setupPins();
NHD_OLED_setupInit();
}


// NHD_OLED_SPIBitBang
//
// This function performs a simple send-only SPI connection using any two
// pins on the STM32. These pins don't have to be hardware SPI pins, but do
// have to be available and not used by other hardware like UARTs or ADC
// inputs. Unless helper functions have been removed to reduce memory usage,
// there's no real reason to call this directly.
//
// Parameters:
//   data: uint8_t to send to display
//   isCommand: command/data flag, where 0 = data and !0 = command
//
void NHD_OLED_SPIBitBang(uint8_t data, uint8_t isCommand)
{
uint8_t i, cb;

// If var "isCommand" is zero, we're sending a data uint8_t.
// If it's NON-zero, we're sending a command uint8_t.
if (isCommand == 0)
cb = 0xFA; // Var "data" is just that, data.
else
cb = 0xF8; // Var "data" is a command/control uint8_t.

// Send the command-or-data type specifier...
for (i = 0; i < 8; i++) {
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 0);
HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, (cb & 0x80) >> 7);
cb = cb << 1;
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 1);
}

// Then send the lowest 4 bits of the data uint8_t, in little-endian order...
for (i = 0; i < 4; i++) {
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 0);
HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, (data & 0x01));
data = data >> 1;
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 1);
}

// Then send four zero bits...
for (i = 0; i < 4; i++) {
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 0);
HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, 0);
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 1);
}

// Then send the highest 4 bits of the data uint8_t, in little-endian order...
for (i = 0; i < 4; i++) {
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 0);
HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, (data & 0x01));
data = data >> 1;
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 1);
}

// And wrap up the send with four zero bits...
for (i = 0; i < 4; i++) {
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 0);
HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, 0);
HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, 1);
}
}


// NHD_OLED_sendCommand
//
// Provides an alternative means to send a command byte to the display.
//
// Parameters:
//   command: command uint8_t to send.
//
void NHD_OLED_sendCommand(uint8_t command)
{
NHD_OLED_SPIBitBang(command, 1);

HAL_Delay(1);
}


// NHD_OLED_sendData
//
// Provides an alternative means to send a data byte to the display.
//
// Parameters:
//   data: command uint8_t to send.
//
void NHD_OLED_sendData(uint8_t data)
{
NHD_OLED_SPIBitBang(data, 0);
HAL_Delay(1);
}



// NHD_OLED_setupPins
//
// Configures pins for driving the display. Note that these don't have to be
// hardware SPI - any two available pins should work as long as they're not
// doing something else.
//
// Parameters:
//    NONE. Define pin OLED_MOSI for data and OLED_CLK for clock in the header file.
//
void NHD_OLED_setupPins()
{
GPIO_InitTypeDef GPIO_InitStruct = { 0 };

GPIO_InitStruct.Pin = OLED_MOSI_Pin | OLED_CLK_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

HAL_Delay(50);
}


// NHD_OLED_setupInit
//
// Initializes and configures the display. Note that the command set provided
// below is for Newhaven's slim OLED line, and individual products may require
// different settings.
//
void NHD_OLED_setupInit()
{
// Internal voltage regulator configuration
NHD_OLED_sendCommand(0x2A); // Function set select > extended command set enable (RE = 1)
NHD_OLED_sendCommand(0x71);     // Internal Vdd regualtor control (function selection A) - command
NHD_OLED_sendData(0x00);        // Internal Vdd regualtor control (function selection A) - value

// Display off
NHD_OLED_sendCommand(0x28); // Function set select > fundamental (default) command set (RE = 0)
NHD_OLED_sendCommand(0x08);     // Display off, cursor off, blink off

// Timing configuration
NHD_OLED_sendCommand(0x2A); // Function set select > extended command set enable (RE = 1)
NHD_OLED_sendCommand(0x79);  // Function set select > OLED command set enable (SD = 1)
NHD_OLED_sendCommand(0xD5);   // Set display clock divide ratio/oscillator frequency - command
NHD_OLED_sendCommand(0x70);   // Set display clock divide ratio/oscillator frequency - value
NHD_OLED_sendCommand(0x78);     // Function set > OLED command set disable (SD = 0)

NHD_OLED_sendCommand(0x09); // Function set select > extended command set enable (RE = 1)
NHD_OLED_sendCommand(0x06);  // Function set select > OLED command set enable (SD = 1)
NHD_OLED_sendCommand(0x72);   // Set display clock divide ratio/oscillator frequency - command
NHD_OLED_sendData(0x00);        // Internal Vdd regualtor control (function selection A) - value

// CGROM select configuration
NHD_OLED_sendCommand(0x2A); // Function set select > extended command set enable (RE = 1)
NHD_OLED_sendCommand(0x79);     // CGROM selection (function selection B) - command
NHD_OLED_sendCommand(0xDA);     // SEG pins hardware configuration - command
NHD_OLED_sendCommand(0x10); // SEG pins hardware configuration - value - NOTE: Some displays require 0x00 here.
NHD_OLED_sendCommand(0xDC);    // VSL & GPIO control (function selection C) - command
NHD_OLED_sendCommand(0x00);     // VSL & GPIO control (function selection C) - value
NHD_OLED_sendCommand(0x81);     // Contrast control - command
NHD_OLED_sendCommand(0x7F);     // Contrast control - value
NHD_OLED_sendCommand(0xD9);     // Phase length - command
NHD_OLED_sendCommand(0xF1);     // Phase length - value
NHD_OLED_sendCommand(0xDB);     // VCOMH deselect level - command
NHD_OLED_sendCommand(0x40);     // VCOMH deselect level - value
NHD_OLED_sendCommand(0x78); // Function set select > OLED command set disable (SD = 0)

// Wrapping up and switching on
NHD_OLED_sendCommand(0x28); // Function set select > fundamental (default) command set (RE = 0)
NHD_OLED_sendCommand(0x01);     // Clear display
NHD_OLED_sendCommand(0x80);   // Set DDRAM address to 0x00 (home on topmost row/line)
NHD_OLED_sendCommand(0x0C);     // Display ON

HAL_Delay(100);
}


// NHD_OLED_displayControl
//
// En/disables the entire display, the cursor, and whether the cursor is a
// flashing block.
//
// Parameters:
//   display: display on/off - non-zero for "on."
//   cursor: cursor on/off - non-zero for "on."
//   block: cursor block/line - non-zero for block.
//
void NHD_OLED_displayControl(uint8_t display, uint8_t cursor, uint8_t block)
{
uint8_t value = 0x08;

if (display != 0)
value = value + 4;

if (cursor != 0)
value = value + 2;

if (block != 0)
value = value + 1;

NHD_OLED_sendCommand(value);
}


// NHD_OLED_displayOn
//
// Switches the entire display on. Cursor and blink are disabled - to enable
// these, use the displayControl() function.
//
void NHD_OLED_displayOn()
{
NHD_OLED_sendCommand(0x0C);
HAL_Delay(10);
}


//NHD_OLED_dispOff
//
// Switches the entire display off.
//
void NHD_OLED_displayOff()
{
NHD_OLED_sendCommand(0x08);
HAL_Delay(10);
}


// NHD_OLED_textClear
//
// Sends a "clear" command to the display.
//
void NHD_OLED_textClear()
{
NHD_OLED_sendCommand(0x01);
HAL_Delay(10);
}


// NHD_OLED_cursorHome
//
// Sends a "home" command to the display, which moves the cursor without
// removing text from the display.
//
void NHD_OLED_cursorHome()
{
NHD_OLED_sendCommand(0x02);
HAL_Delay(10);
}


// NHD_OLED_cursorMoveToRow
//
// Move cursor to start of selected line.
//
// Parameters:
//   rowNumber: row/line number to move to (zero-indexed, where 0 is topmost).
//
void NHD_OLED_cursorMoveToRow(uint8_t rowNumber)
{
uint8_t row_command[4] = { 0x80, 0xA0, 0xC0, 0xE0 };

NHD_OLED_sendCommand(row_command[rowNumber]);
HAL_Delay(10);
}


// NHD_OLED_textClearRow
//
// Clears a line on the display by writing spaces to the entire row/line.
//
// Parameters:
//   rowNumber: row/line number to clear (zero-indexed, where 0 is topmost).
//
void NHD_OLED_textClearRow(uint8_t rowNumber)
{
char temp[OLED_DISP_COLUMNS];

for (int i = 0; i < OLED_DISP_COLUMNS; i++)
temp[i] = 0x20;

NHD_OLED_cursorMoveToRow(rowNumber);
NHD_OLED_print_len(temp, OLED_DISP_COLUMNS);
}


// NHD_OLED_shift
//
// Shifts the cursor or the displayed text one position right or left.
//
// Parameters:
//   dc: display or cursor - non-zero for display.
//   rl: direction right/left - non-zero for right.
//
void NHD_OLED_shift(uint8_t dc, uint8_t rl)
{
uint8_t value = 0x10;

if (dc != 0)
value = value + 8;

if (rl != 0)
value = value + 4;

NHD_OLED_sendCommand(value);
}


// NHD_OLED_cursorPos
//
// Moves the cursor to the given column on the given row/line.
//
// Parameters:
//   row: row/line number (0-1/2/3).
//   column: column number (0-16/20).
//
void NHD_OLED_cursorPos(uint8_t row, uint8_t column)
{
uint8_t row_command[4] = { 0x80, 0xA0, 0xC0, 0xE0 };

if (row >= OLED_DISP_ROWS)
row = OLED_DISP_ROWS - 1;
if (column >= OLED_DISP_COLUMNS)
column = OLED_DISP_COLUMNS - 1;

NHD_OLED_sendCommand(row_command[row] + column);
}

// NHD_OLED_print_len
//
// Sends text to the display to be, well, displayed. Note that the text is
// printed from the current cursor position, so calling NHD_OLED_home(),
// NHD_OLED_pos(), or NHD_OLED_cursorMoveToLine() functions first would
// probably be a good idea.
//
// Parameters:
//   text: text to display. This should be a full string if a length is
//         provided.
//   len: length of text to print, in characters.
//
void NHD_OLED_print_len(char *text, uint8_t len)
{
for (uint8_t i = 0; i < len; i++)
NHD_OLED_sendData(text[i]);
}


// NHD_OLED_print_char
//
// Sends a single character to the display. No length parameter is needed.
// Again, printing happens where the cursor is positioned.
//
// Parameters:
//   text: text to display. This must be a single character.
//
void NHD_OLED_print_char(char text)
{
NHD_OLED_sendData(text);
}


// NHD_OLED_print_len_pos
//
// This print subroutine accepts row/line and column numbers (zero-indexed -
// 0, 0 is home) and moves the cursor to the given position before
// printing the supplied text.
//
// Parameters:
//   text: text to display. This should be a full string.
//   len: length of text to print, in characters.
//   r: row/line number (0-1/2/3).
//   c: column number (0-16/20).
//
// ADDITIONAL USAGE NOTES:
//
// To right-justify text, determine the length of the text, e.g., with
// sizeof(text), and use (DISP-COLUMNS - length) as the column number.
//
// To center text, use ((DISP-COLUMNS - length) / 2) as the column number.
//
// In either of the above usage cases, clearing the line with NHD_OLED_clear()
// before printing text may be desirable.
//
void NHD_OLED_print_len_pos(char *text, uint8_t len, uint8_t r, uint8_t c)
{
NHD_OLED_cursorPos(r, c);

for (uint8_t i = 0; i < len; i++)
NHD_OLED_sendData(text[i]);
}


// NHD_OLED_print_pos
//
// This print subroutine accepts row/line and column numbers (zero-indexed -
// 0, 0 is home) and moves the cursor to the given position before printing
// the supplied character. Only one character is printed - no length
// parameter is needed.
//
// Parameters:
//   text: text to display. This must be a single character.
//   r: row/line number (0-1/2/3).
//   c: column number (0-16/20).
//
void NHD_OLED_print_pos(char text, uint8_t r, uint8_t c)
{
NHD_OLED_cursorPos(r, c);

NHD_OLED_sendData(text);
}


// NHD_OLED_textPrintCentered
//
// Prints the supplied text, centered, on the selected row/line. The row/line
// is cleared before printing.
//
// Parameters:
//   text: text to display. This should be a full string.
//   length: length of text to print, in characters.
//   row: row/line number (0-1/2/3).
//
void NHD_OLED_textPrintCentered(char *text, uint8_t length, uint8_t row)
{
uint8_t i;
char line[OLED_DISP_COLUMNS];

// Clear the text buffer.
for (i = 0; i < OLED_DISP_COLUMNS; i++)
line[i] = 0x20;

// Copy text into buffer, centering it relative to the line's width.
for (i = 0; i < length; i++)
line[((OLED_DISP_COLUMNS - length) / 2) + i] = text[i];

// Clear the line.
NHD_OLED_textClearRow(row);

// Move the cursor to the row/line.
NHD_OLED_cursorMoveToRow(row);

// Print the centered text.
NHD_OLED_print_len(line, OLED_DISP_COLUMNS);
}


// NHD_OLED_textPrintRightJustified
//
// Prints the supplied text, right-justified, on the selected row/line. The
// row/line is cleared before printing.
//
// Parameters:
//   text: text to display. This should be a full string.
//   length: length of text to print, in characters.
//   row: row/line number (0-1/2/3).
//
void NHD_OLED_textPrintRightJustified(char *text, uint8_t length, uint8_t row)
{
uint8_t i;
char line[OLED_DISP_COLUMNS];

// Clear the text buffer.
for (i = 0; i < OLED_DISP_COLUMNS; i++)
line[i] = 0x20;

// Copy text into buffer, centering it relative to the line's width.
for (i = 0; i < length; i++)
line[(OLED_DISP_COLUMNS - length) + i] = text[i];

// Clear the row/line.
NHD_OLED_textClearRow(row);

// Move the cursor to the row/line.
NHD_OLED_cursorMoveToRow(row);

// Print the centered text.
NHD_OLED_print_len(line, OLED_DISP_COLUMNS);
}


// NHD_OLED_textSweep
//
// Performs a simple animation that sweeps two characters into the center from
// the outermost columns, then back outward, drawing centered text between
// them as they separate. It's a simple but surprisingly eye-catching effect.
//
// Parameters:
//   text: text to display.
//         Note that this should be equal to or less than OLED_DISP_COLUMNS in
//         character count, or unexpected results like garbled wrapping text
//         can ensue.
//   length: length of text to print, in characters.
//   row: row/line number (0-1/2/3).
//   leftSweepChar: left-to-right sweep character.
//   rightSweepChar: right-to-left sweep character.
//   time_delay: time delay between steps (in milliseconds).
//
void NHD_OLED_textSweep(char *text, uint8_t length, uint8_t row,
    char leftSweepChar, char rightSweepChar,
                uint8_t time_delay)
{
uint8_t stepnum = 0;
uint8_t outer, inner, start;
uint8_t i, ii;
char line[OLED_DISP_COLUMNS];

// Clear the text buffer.
for (i = 0; i < OLED_DISP_COLUMNS; i++)
line[i] = 0x20;

// Copy text into buffer, centering it relative to the line's width.
for (i = 0; i < length; i++)
line[((OLED_DISP_COLUMNS - length) / 2) + i] = text[i];

// Time to do work. First things first: clear the line we'll print to.
NHD_OLED_textClearRow(row);

// The first half of the process: sweep into the center.
for (i = 0; i < (OLED_DISP_COLUMNS / 2); i++) {
NHD_OLED_cursorMoveToRow(row);

// Work out some starting positions and widths.
outer = stepnum;
inner = OLED_DISP_COLUMNS - 2 - (stepnum * 2);
start = stepnum + 2;

// Print leading spaces to pad the start of the line...
for (ii = 0; ii < outer; ii++)
NHD_OLED_print_char(0x20);

// ... then print the left-to-right character...
NHD_OLED_print_char(leftSweepChar);

// ... followed by spaces in the middle...
for (ii = 0; ii < inner; ii++)
NHD_OLED_print_char(0x20);

// ... then the right-to-left character...
NHD_OLED_print_char(rightSweepChar);

// ... and finish with more spaces to clear the rest of the line.
for (ii = 0; ii < outer; ii++)
NHD_OLED_print_char(0x20);

HAL_Delay(time_delay);

stepnum++;
}

// Decrement the step number so things stay properly aligned.
stepnum--;

// The second half: sweep out from the center, leaving text behind.
for (i = (OLED_DISP_COLUMNS / 2); i < OLED_DISP_COLUMNS; i++) {
NHD_OLED_cursorMoveToRow(row);

// More starts and widths.
outer = stepnum;
inner = OLED_DISP_COLUMNS - 2 - (stepnum * 2);
start = stepnum + 1;

// This pass is similar to the first, starting with padding...
for (ii = 0; ii < outer; ii++)
NHD_OLED_print_char(0x20);

// ... then the right-to-left character, as though it passed through its
// opposite...
NHD_OLED_print_char(rightSweepChar);

// ... then the centermost part of the text...
for (ii = 0; ii < inner; ii++)
NHD_OLED_print_char(line[start + ii]);

// ... then the left-to-right character...
NHD_OLED_print_char(leftSweepChar);

// ... then spaces to clear the line.
for (ii = 0; ii < outer; ii++)
NHD_OLED_print_char(0x20);

HAL_Delay(time_delay);

stepnum--;
}

// Finish by printing the text by itself with no side-to-side characters.
NHD_OLED_cursorMoveToRow(row);
NHD_OLED_print_len(line, OLED_DISP_COLUMNS);
}

void output() {
int i;
NHD_OLED_sendCommand(0x01); //clear display
NHD_OLED_sendCommand(0x02); //return home
for(i=0;i<20;i++) {
	NHD_OLED_sendData(0x1F); //write solid blocks
}
NHD_OLED_sendCommand(0xA0); //line 2
for(i=0;i<20;i++) {
	NHD_OLED_sendData(0x1F); //write solid blocks
}
NHD_OLED_sendCommand(0xC0); //line 3
for(i=0;i<20;i++) {
	NHD_OLED_sendData(0x1F); //write solid blocks
}
NHD_OLED_sendCommand(0xE0); //line 4
for(i=0;i<20;i++) {
	NHD_OLED_sendData(0x1F); //write solid blocks
}
}
/*
 * End of file!
 */
