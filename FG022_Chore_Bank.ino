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
#include <XPT2046_Touchscreen.h> // Touchscreen for The Wavesaher screen
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

// For Arduino Pro Micro
//#define SCK           15 // SPI clock
    
#define TFT_CS        10 // LCD_CS
#define TFT_BL        8  // LCD_BL
#define TFT_DC        9  // LCD_DC
// SCK = 15, MISO = 14, MOSI = 16
// For touchscreen
#define TIRQ_PIN  7
#define CS_PIN    A0
/*
// For Arduino Uno
#define CS_PIN  4  //Waveshare Touch
#define TFT_DC  7  //Waveshare 
#define TFT_CS 10  //Waveshare
#define TFT_BL  9  //Waveshare backlight
// For touchscreen
#define TIRQ_PIN  3
// MOSI=11, MISO=12, SCK=13
*/


// colours
#define BLACK         0x0000
#define WHITE         0xFFFF
#define RED           0xF800
#define GREEN         0x07E0
#define YELLOW        0xFFE0
#define LIGHT_PINK    0xF2B8FC

// instantiate the screen
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_BL);
// instantiate the souchscreen
XPT2046_Touchscreen ts(CS_PIN);

// Card Reader

#define B0            0 // TX as DigitalPin
#define B1            1 // RX as DigitalPin
#define B2            2
#define B3            3
#define B4            4
#define B5            5
#define B6            6

/*
// for Uno
#define B0            A0 // TX as DigitalPin
#define B1            A1 // RX as DigitalPin
#define B2            A2
#define B3            A3
#define B4            A4
#define B5            A5
#define B6            A6
*/
// Selector Buttons
/*
#define BUTTON_MENU   7
#define BUTTON_SELECT 14
#define BUTTON_FWD    16
#define BUTTON_BACK   A0
*/

// Internal Parameters
#define TOUCH_RAW     4095
#define Y_MAX         240
#define Y_FONT_SIZE   14 // used for incrementing int the Y axis
#define X_MAX         340
#define BRIGHT_MAX    255
#define LIGHT_MAX     255
#define PAGE_LAST     1 // 2 for testing
#define MAX_TASKS     38 // 38 items rn!
#define LTOP          0 // quadrant for Quadrant
#define RTOP          1
#define LBOT          2
#define RBOT          3

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
enum States {WAIT, TASKS, MENU, TIME_SET, BRIGHTNESS, ABOUT, INCREMENT};

// Primary pages 
enum Pages  {TIME_P, BALANCE_P, INSERTED_P, MENU_P, TIME_SET_P, BRIGHTNESS_P, ABOUT_P};

/*
  Global Variables
*/
int prevPage = 0;
int pageNumber = 0;
int screenBrightness = 127;
int lightBrightness = 127;
int pointerX = 0;
int pointerY = 0;

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  Serial.println("GABOBANK Test!");
  tft.init(240, 320, SPI_MODE0); // screen init
  ts.begin(); // touchscreen init
  tft.setRotation(1);
  while (!Serial && (millis() <= 1000));
  
  
  pinMode(B0, INPUT_PULLUP);
  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP);
  pinMode(B4, INPUT_PULLUP);
  pinMode(B5, INPUT_PULLUP);
  pinMode(B6, INPUT_PULLUP);

  WaitPage(true);

}

// flip flops the touch condition
boolean wasTouched = true;
States state = WAIT;
States prevState = state;

void loop() {
  // put your main code here, to run repeatedly:
  
  if(DEBUG)
    if(millis() % 500 == 0)
      Serial.println(state);
  
  boolean isTouched = ts.touched();

  switch (state)
  {
    
    case WAIT:
      if(wasTouched)
      {
        wasTouched = false;
        WaitPage(true);
      }
      if(isTouched)
      {
        wasTouched = true;
        Serial.println("Touched Wait");
        
        if(Quadrant() == RBOT || Quadrant() == LBOT)
        {
          state = TASKS;
          Tasks(true);
        }
        if(Quadrant() == LTOP)
        {
          state == ABOUT;
        }
        if(Quadrant() == RTOP)
        {
          prevState = WAIT;
          state = INCREMENT;
        }
      }
      
      //if(millis() % 1000000 == 0)
      break;

    case TASKS:
      if(wasTouched)
      {
        wasTouched = false;
      }
      if(isTouched)
      {
        wasTouched = true;

        if(Quadrant() == RTOP)
        {
          prevState = TASKS;
          state = INCREMENT;
        }
        if(Quadrant() == LTOP)
          state = WAIT;
        if(Quadrant() == RBOT)
        {
          ChangePage(0, 1);
          Tasks(true);
        }
        else if(Quadrant() == LBOT)
        {
          ChangePage(1, 0);
          Tasks(true);
        }
      }
      break;
    case INCREMENT:
        delay(50); // tap debounce
        long int cards = ReadCard(digitalRead(B0), digitalRead(B1), digitalRead(B2), digitalRead(B3), digitalRead(B4), digitalRead(B5), digitalRead(B6));
        CardInserted(cards);
        if(DEBUG)
          Serial.println(taskNames[cards]);
        if(DEBUG)
          Serial.println(cards);
        if(DEBUG)
          Serial.println("Card Inserted!");
        state = prevState;
      break;
    case MENU:
      /*if(isTouched)
      {
        Menu(true);
      }
      else
        state = MENU; // keep staying in this state    */
      break;
    case BRIGHTNESS:
      break;
    case ABOUT:
      break;
    default:
      break;
  }

}



/*
  Reads the addresss based on the state of the reader, which depends on the card inserted.

  Input: buttons [6:0] boolean state, representing 7-bit binary integer

  Output: decimal integer taskIndex
*/
long int ReadCard(int b0, int b1, int b2, int b3, int b4, int b5, int b6)
{
  // b0 is the LSB, b6 is the MSB
  // buttons are active low, so need to flip in order to use for calculation
  
  return (!b6*64) + (!b5*32) + (!b4*16) + (!b3*8) + (!b2*4) + (!b1*2) + (!b0*1);
}

/*
  Returns which corner was tapped by the touchscreen.

  Input: none

  Output: int quadrant, defined above
*/
int Quadrant()
{
  TS_Point pointer = ts.getPoint();

  if(pointer.y < TOUCH_RAW/2)
      if(pointer.x > TOUCH_RAW/2)
        return RBOT;
      else
        return LBOT;
    else // top half
      if(pointer.x > TOUCH_RAW/2) // if the touch point was on the right side of the screen
          return RTOP;
    
    return LTOP;

}

/*
  Displays the card inserted once incremented with some graphics.
  Called when a card is deposited into the machine (ie: top right is tapped).

  Input: int taskIndex

  Output: void, writes directly to screen
*/
void CardInserted(int taskIndex)
{
  TaskIncrement(taskIndex);

  tft.fillScreen(LIGHT_PINK);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);

  tft.setCursor(4,4);
  tft.print("Lovely has used . . .");

  tft.setTextSize(4);
  tft.setCursor(X_MAX/8, Y_MAX/3+14);
  tft.print(taskNames[taskIndex]);

  tft.setTextSize(2);
  tft.setCursor(10, Y_MAX-(Y_FONT_SIZE*3)+14);
  tft.print("^-^ <3");
  delay(1500); // delay for 1.5 seconds
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
  if(taskIndex > MAX_TASKS)
    return 0;
  else
  {
    uint16_t tempValue = EEPROM.read(taskIndex+128); // read the MSB
    tempValue <<= 8; // shift bits by 8
    tempValue += EEPROM.read(taskIndex); // add the LSB

    return (long int)tempValue;
  }
}

/*
  Builds a string of the task and its count to the display at a specified coordinate.

  Input: xcoord, ycoord, taskIndex
  Output: void, writes to display directly
*/
void DisplayTask(int xCoord, int yCoord, int taskIndex)
{
  // first black out the area to be written in, which is a 170 wide x 14 high box
  tft.fillRect(xCoord, yCoord, X_MAX/2, Y_FONT_SIZE, BLACK);

  if(taskIndex < MAX_TASKS)
  {
    String stringBuilder = taskNames[taskIndex] + ": " + GetTaskCount(taskIndex);
    tft.setTextColor(YELLOW);
    //tft.setFont(Arial_48);
    tft.setTextSize(1);
    tft.setCursor(xCoord, yCoord);

    // displays the text, expected: "HUG: 999"
    tft.print(stringBuilder);
  }
}

/*
  Displays all the task for a page and refreshes for more tasks whenever the page number is changed.
  Uses DisplayTask, amountToDisplay must be adjusted to fit on the physical screen!
  Note: the screen is going to be sideways, so ensure that the coordinates are rotated

  Input: int start of the index to look at, int of the amount of tasks to display on the page.
          ** Amount to Display is limited to 34!

  Output: void, writes directly to screen
*/
void DisplayTaskPage(int indexStart, int amountToDisplay)
{
  int xCoord = 0;
  int yCoord = 0;

  tft.fillScreen(BLACK);

  for(int i = 0; i < amountToDisplay; i++)
  {
    DisplayTask(xCoord, yCoord, indexStart+i);
    if(yCoord < Y_MAX - Y_FONT_SIZE*2)
      yCoord += Y_FONT_SIZE; // font size is 7 for textsize 1
    else
    {
      yCoord = 0; // back to first line
      xCoord += (X_MAX / 2); // move to the second half of the screen
    }
  }
}

/*
  This module acts as the main datapath for task page things.

    Calls: DisplayTaskPage, and ChangePage



    Input: page number

    Output: boolean finish, indicates ready state

*/
void Tasks(bool start)
{      
  DisplayTaskPage(34*(pageNumber), 34*(pageNumber+1));
}

/*
  Incremenet/Decrement the task pages based on button inputs 

  Input: button state left, button state right

  Output: void, writes directly to main display moduleasdasdasdasd
*/
void ChangePage(int left, int right)
{
  if(left == 1)
    if(pageNumber == 0)
      pageNumber = PAGE_LAST;
    else
      pageNumber -= 1;
  if(right == 1)
    if(pageNumber == PAGE_LAST)
      pageNumber = 0;
    else
      pageNumber += 1;
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

/*
  Sets up the main menu.

  Input: bool start init
  Output: void, writes directly to display
*/
void Menu(bool start)
{
  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);

  // top left
  tft.setCursor(4,4);
  tft.print("Adjust");
  tft.setCursor(4,20);
  tft.print("Brightness");

  // bot left
  tft.setCursor(4, Y_MAX/2);
  tft.print("<<< Back");

  // bot right
  tft.setCursor(X_MAX/2, Y_MAX/2);
  tft.print("About");

}

/*
  Page for waiting for an input.
*/
void WaitPage(bool start)
{
  tft.fillScreen(GREEN);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  
  tft.setCursor(4,4);
  tft.print("Welcome to . . .");

  tft.setTextSize(3);
  tft.setCursor(X_MAX/2-70, Y_MAX/2-20);
  tft.print("LOVELY");
  tft.setCursor(40, Y_MAX/2+4);
  tft.print("ACTIVITIES ^-^");

  tft.setTextSize(2);
  tft.setCursor(50, Y_MAX-30);
  tft.print("<< Tap to Begin >>");

}










































































