/*
  FG022: Gabo Bank

  For the purspose of lovely's 21st birthday, a digital chore wallet is 
  kept to keep track of chorse and tasks that have been given or assigned to the other lovely.

  Other components in this project are the mechanical housing, dislpay, nightlights, and chore cards.

*/

// Pro Micro has 1KB of EEPROM, can prolly load up 256 tasks, each with 4 bytes where each task can be
// 32 bits, meaning a max theoretical of 4294967295 counts. That's a lot of hugs but is not enough >:(

#include <EEPROM.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

// Fonts
#include <Fonts/FreeSans9pt7b.h>   //.kbv use FreeFonts etc
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#define Arial_24 &FreeSans9pt7b    //.kbv FreeFonts use address
#define Arial_48 &FreeSans12pt7b
#define Arial_60 &FreeSans18pt7b

#define DEBUG false

/*
  Board I/O for the Arduino Pro Micro 

  Component List:
  - TFT Display (Only interested in the display function for now, no touch)
  - 4 Buttons
  - 7 Microswitches
  - 1 Power switch (not connected to any board IO)


*/
// TFT Display
#define SCK           15 // SPI clock
#define TFT_CS        10 // LCD_CS
#define TFT_BL        9  // LCD_BL
#define TFT_DC        8  // LCD_DC
#define BLACK         0x0000      //.kbv hard-coded colors 
#define RED           0xF800
#define GREEN         0x07E0
#define YELLOW        0xFFE0
// instantiate tft 
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_BL);

// Card Reader
#define B0            0 // TX as DigitalPin
#define B1            1 // RX as DigitalPin
#define B2            2
#define B3            3
#define B4            4
#define B5            5
#define B6            6

// Selector Buttons
#define BUTTON_MENU   7
#define BUTTON_SELECT 14
#define BUTTON_FWD    16
#define BUTTON_BACK   A0

// Internal Parameters
#define Y_MAX         240
#define X_MAX         340
#define BRIGHT_MAX    255
#define LIGHT_MAX     255
#define PAGE_LAST     20

// EEPROM Location for Tasks, task of 128
/*
  This is a long list! Currently 0 to 37 (38 items)
  The carry byte is +128 of the current location, so for HUG @ 0, HUG_carry @ 128.
    This allows 2 byte values, for a count of over 65000! But ofc I would give more hugs than that :triumph:

  See excel sheet for generating or updating the list
*/
enum Tasks 
{HUG, 
KISS, 
CUDDLE, 
BITE, 
LICK, 
GET_SNACKS, 
COOK_FOOD, 
ATTENTION, 
AFFECTION, 
DATE_NIGHT, 
FOCUS_MODE, 
CALL_UNEXPECTEDLY, 
FACE_MASKS, 
MOVIE_NIGHT, 
DINNER_TOGETHER, 
BORGAR, 
RAMEN, 
PHO, 
ICE_CREAM, 
OUTDOOR_DATE, 
VENT, 
TRAVEL_TOGETHER, 
DANCE, 
NIGHTTIME_ACTIVITY, 
SARAH_MOVIE, 
VIKTOR_MOVIE, 
KPOP, 
BTS, 
DANCING, 
GIVE_SHIRT, 
SILLY_PICTURES, 
STUDY_DATE, 
PLUCK, 
VIDEO_CALL, 
PASSENGER_PRINCESS, 
PASSENGER_PRINCE, 
DRUNK, 
GET_HIGH
};
// String description of each task, see excel sheet for generating or updating the list
const String taskNames[] = 
{"HUG",
"KISS",
"CUDDLE",
"BITE",
"LICK",
"GET_SNACKS",
"COOK_FOOD",
"ATTENTION",
"AFFECTION",
"DATE_NIGHT",
"FOCUS_MODE",
"CALL_UNEXPECTEDLY",
"FACE_MASKS",
"MOVIE_NIGHT",
"DINNER_TOGETHER",
"BORGAR",
"RAMEN",
"PHO",
"ICE_CREAM",
"OUTDOOR_DATE",
"VENT",
"TRAVEL_TOGETHER",
"DANCE",
"NIGHTTIME_ACTIVITY",
"SARAH_MOVIE",
"VIKTOR_MOVIE",
"KPOP",
"BTS",
"DANCING",
"GIVE_SHIRT",
"SILLY_PICTURES",
"STUDY_DATE",
"PLUCK",
"VIDEO_CALL",
"PASSENGER_PRINCESS",
"PASSENGER_PRINCE",
"DRUNK",
"GET_HIGH"
};

// States for each condition the device could be in 
enum States {TIME, BALANCE, INSERTED, MENU, TIME_SET, BRIGHTNESS, ABOUT};

// Primary pages 
enum Pages  {TIME_P, BALANCE_P, INSERTED_P, MENU_P, TIME_SET_P, BRIGHTNESS_P, ABOUT_P};

/*
  Global Variables
*/
int pageNum = 0;
int screenBrightness = 127;
int lightBrightness = 127;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  

  States state = TIME;

  switch (state)
  {
    case TIME:

    case BALANCE:

    case INSERTED:

    case MENU:

    case TIME_SET:

    case BRIGHTNESS:

    case ABOUT:

    default: state = 1;
  }

}

/*
  Advances or decrements time, then sets the device time. Note: will be reset upon restart

  Input: button state of up, button state of down, button state of select

  Output: void, sets the time counter directly
*/
void TimeSet(int up, int down, int advance)
{

}

/*
  Reads the addresss based on the state of the reader, which depends on the card inserted.

  Input: buttons [6:0] boolean state, representing 7-bit binary integer

  Output: decimal integer taskIndex
*/
long int ReadCard(int b0, int b1, int b2, int b3, int b4, int b5, int b6)
{
  // b0 is the LSB, b6 is the MSB
  return (b6*64) + (b5*32) + (b4*16) + (b3*8) + (b2*4) + (b1*2) + (b0*1);
}

/*
  Increments the count for a task by writing to EEPROM, Uses ReadCard. This module must also deal with overflow, and use the second byte
  The second byte for the task will be at +128 of the current location.

  Input: The corresponding task index

  Output: void, writes directly to EEPROM location based on task number
*/
void TaskIncrement(int taskIndex)
{
  // make sure to actually read the location first to add the value
  int tempLocation = taskIndex;
  long int tempValue = GetTaskCount(tempLocation) + 1;
  uint16_t shifter = 0;
  

  // if greater than 255, split into MSB and LSB before writing to their own EEPROM bytes
  // max of 65535, but I will do more than that lovely, it's just a limitation of the Arduino hehe
  if(tempValue > 255) // decimal 255
  {
    shifter = (tempValue >> (8*0)) & 0xff; // get the LSB
    EEPROM.update(tempLocation, shifter);
    shifter = (tempValue >> (8*1)) & 0xff; // get the MSB
    tempLocation += 128;
    EEPROM.update(tempLocation, shifter);
  }
  // just increment the value otherwise
  else
    EEPROM.update(tempLocation, tempValue);
}

/*
  Returns the value stored for the corresponding task. Must also look at the second byte!

  Input: The corresponding task index

  Output: long integer of task count, may be over 
*/
long int GetTaskCount(int taskIndex)
{
  uint16_t tempValue = EEPROM.read(taskIndex+128); // read the MSB
  tempValue <<= 8; // shift bits by 8
  tempValue += EEPROM.read(taskIndex); // add the LSB

  return (long int)tempValue;
}

/*
  Builds a string of the task and its count to the display at a specified coordinate.

  Input: xcoord, ycoord, taskIndex
  Output: void, writes to display directly
*/
void DisplayTask(int xCoord, int yCoord, int taskIndex)
{
  String stringBuilder = taskNames[taskIndex] + ": " + GetTaskCount(taskIndex);
  tft.setTextColor(YELLOW);
  tft.setFont(Arial_48);
  tft.setCursor(xCoord, yCoord);

  // displays the text, expected: "HUG: 999"
  tft.print(stringBuilder);
}

/*
  Displays all the task for a page and refreshes for more tasks whenever the page number is changed.
  Uses DisplayTask, amountToDisplay must be adjusted to fit on the physical screen!
  Note: the screen is going to be sideways, so ensure that the coordinates are rotated

  Input: int start of the index to look at, int of the amount of tasks to display on the page

  Output: void, writes directly to screen
*/
void DisplayTaskPage(int indexStart, int amountToDisplay)
{
  int xCoord = 0;
  int yCoord = 0;

  for(int i = 0; i < amountToDisplay; i++)
  {
    DisplayTask(xCoord, yCoord, indexStart+i);
    if(yCoord < Y_MAX)
      yCoord += 48; // font size is 48
    else
    {
      yCoord = 0;
      xCoord += (X_MAX / 2); // move to the second half of the screen
    }
  }
}

/*
  This module acts as the main datapath for all pages. Uses ChangePage.
  Interacts with the display module directly.
  Page Index: 
    0 - Time

    1 - Menu

    2 - Time Set

    3 - Brightness Set

    4 - About

    10 - Balance Page 1

    ... page >= 10 are dedicated to tasks

    Input: page number

    Output: void, writes directly to main display module

*/
void DisplayPage(int left, int right, int pageNumber)
{

}

/*
  Incremenet/Decrement the task pages based on button inputs 

  Input: button state left, button state right

  Output: void, writes directly to main display moduleasdasdasdasd
*/
void ChangePage(int left, int right)
{
  if(left == 1)
    if(pageNum == 0)
      pageNum = PAGE_LAST;
    else
      pageNum -= 1;
  if(right == 1)
    if(pageNum == PAGE_LAST)
      pageNum = 0;
    else
      pageNum += 1;
}

/*
  Adjusts the brightness of the screen based on button inputs.

  Input: button state left, button state right

  Output: void, writes directly to main display module
*/
void ChangeScreenBrightness(int up, int down)
{

}

/*
  Adjusts the brightness of the nightlight

  Input: button state up, button state down

  Output: void, writes directly to lights
*/
void ChangeLightBrightness(int up, int down)
{

}


















































































