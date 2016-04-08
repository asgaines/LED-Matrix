using namespace std;
#include <LedControl.h>
#include <utility.h>
//#include <system_configuration.h>
#include <StandardCplusplus.h>
//#include <unwind-cxx.h>
#include <vector>
// alphabet file holds all characters for the scrolling text
#include "alphabet.h"

// dataPin is connected to the DATA IN-pin of the first MAX7219
int dataPin = 12;
// clockPin is connected to the CLK-pin of both MAX7219s
int clockPin = 11;
// csPin is connected to the LOAD(/CS)-pin of both MAX7219s
int csPin = 10;

// Number of MAX7219/7221s used (each 8x8)
const int numDevices = 2;
// Potentiometer used to control the intensity of the LEDs. Optional
int trimPot = A0;
// North and South grids, currently connected in series. Optional
int AdditionalLights = 5;
// Declare instance of the LedControl object
LedControl lc = LedControl(dataPin, clockPin, csPin, numDevices);

// Used to calculate a delay for the acceleration of rows/columns
int delayFactor = 10;
// Used to control a beat
int delayTime = 25;
int intensity = 15;
int readIntensity = 7; // This variable reads the new setting of the
// potentiometer. If different from intensity, update the setting 
// on the matrix
//int message[23] = {41, 0, 76, 79, 86, 69, 0, 43, 69, 76, 76, 69, 89, 0, 46, 69, 85, 77, 65, 78, 78, 1, 0};
int counter;
// Default value if nothing read from Serial port. Change to whatever you'd like
String serialString = "Acressity"; 
vector<int> convertedMessage(serialString.length() * 6);
// Toggles between functions
// 0 = message, 1 = picture, 2 = starry night
int functionID = 0;
int picture[16] = {0, 0, 60, 90, 189, 24, 90, 255, 255, 90, 24, 189, 90, 60, 0, 0};

// Variables for snake game
int numVertebrae = 5; // Starting length of snake
const int numLEDs = numDevices * 64; // Total number of possible vertebrae of snake, one for each LED before victory
int snake[numLEDs][2];
char snakeDirection;
const char UP = 'w';
const char LEFT = 'a';
const char DOWN = 's';
const char RIGHT = 'd';
const char PAUSE = 'p';
int nextSnakeSpot[2];
int snakeDelay = 500;
int snakeFruit[2];

void setup() {
  // LedControl library handles all INPUT/OUTPUT declarations for lc
  for (int address = 0; address < numDevices; address++) {
    // Wake up display #1 (by un-shutting it down--
    // powersaving mode in beginning)
    lc.shutdown(address, false);
    // Set intensity for display #1 (from 0-15)
    lc.setIntensity(address, intensity);
    lc.clearDisplay(address); // Reset display
  }
  pinMode(trimPot, INPUT);
  pinMode(AdditionalLights, OUTPUT);
  Serial.begin(9600);
  // Initialize random numbers, reading noise from Arduino input pins
  randomSeed(analogRead(A6)); // Read from unused pin!
  // Initialize the message
  //convertMessage(serialString);
  //displayMessage();
}

void loop() {
  nightSky();
  // playSnake();
  // meditate();
}

void playSnake() {
  initializeSnake();
  
  while (!victory() && !gameOver()) {
    displaySnake();
    displayFruit();
    
    // Retrieve direction to move from serial port
    if (Serial.available() > 0) {
      snakeDirection = Serial.read();
    }
    Serial.println(Serial.available());

    // Move head to new location based on direction
    switch (snakeDirection) {
      case UP:
        nextSnakeSpot[0] = snake[numVertebrae - 1][0] - 1;
        nextSnakeSpot[1] = snake[numVertebrae - 1][1];
        if (overlap(nextSnakeSpot, snakeFruit)) {
          snakeEatFruit();
        } else {
          moveVertebraeForward();
          snake[numVertebrae - 1][0] = nextSnakeSpot[0];
        }
        break;
      case DOWN:
        nextSnakeSpot[0] = snake[numVertebrae - 1][0] + 1;
        nextSnakeSpot[1] = snake[numVertebrae - 1][1];
        if (overlap(nextSnakeSpot, snakeFruit)) {
          snakeEatFruit();
        } else {
          moveVertebraeForward();
          snake[numVertebrae - 1][0] = nextSnakeSpot[0];
        }
        break;
      case LEFT:
        nextSnakeSpot[0] = snake[numVertebrae - 1][0];
        nextSnakeSpot[1] = snake[numVertebrae - 1][1] - 1;
        if (overlap(nextSnakeSpot, snakeFruit)) {
          snakeEatFruit();
        } else {
          moveVertebraeForward();
          snake[numVertebrae - 1][1] = nextSnakeSpot[1];
        }
        break;
      case RIGHT:
        nextSnakeSpot[0] = snake[numVertebrae - 1][0];
        nextSnakeSpot[1] = snake[numVertebrae - 1][1] + 1;
        if (overlap(nextSnakeSpot, snakeFruit)) {
          snakeEatFruit();
        } else {
          moveVertebraeForward();
          snake[numVertebrae - 1][1] = nextSnakeSpot[1];
        }
        break;
      default:
        // Any other key pauses the gameplay
        continue;
    }

    delay(snakeDelay - 2 * numVertebrae); // Slight speed increase after each feeding
    
    lc.clearDisplay(0);
    lc.clearDisplay(1);
  }
}

void initializeSnake() {
  snake[0][0] = 3; // Tail row
  snake[0][1] = 0; // Tail column
  snake[1][0] = 3;
  snake[1][1] = 1;
  snake[2][0] = 3;
  snake[2][1] = 2;
  snake[3][0] = 3;
  snake[3][1] = 3;
  snake[4][0] = 3; // Head row
  snake[4][1] = 4; // Head column
  numVertebrae = 5;
  snakeDirection = RIGHT;
  snakePositionFruit();
  Serial.println("NEW GAME");
  delay(100);
}

void moveVertebraeForward() {
  // Move all vertebrae forward
  for (int vertebra = 1; vertebra < numVertebrae; vertebra++) {
    snake[vertebra - 1][0] = snake[vertebra][0];
    snake[vertebra - 1][1] = snake[vertebra][1];
  }
}

bool overlap(int spot1[], int spot2[]) {
  return (spot1[0] == spot2[0] && spot1[1] == spot2[1]);
}

void snakeEatFruit() {
  // Grow the length of the snake
  snake[numVertebrae][0] = snakeFruit[0];
  snake[numVertebrae][1] = snakeFruit[1];
  numVertebrae++;

  // Put a new piece of fruit out there for the hungry fella
  snakePositionFruit();
}

void snakePositionFruit() {
  // Create array of all free spaces
  int freeSpaces[numLEDs - numVertebrae][2];
  int counter = 0;
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < numDevices * 8; column++) {
      int spot[2] = {row, column};
      int spotFree = true;
      for (int vertebra = 0; vertebra < numVertebrae; vertebra++) {
        int vert[2] = {snake[vertebra][0], snake[vertebra][1]};
        if (overlap(spot, vert)) {
          spotFree = false;
          break; 
        }
      }
      if (spotFree) {
        freeSpaces[counter][0] = spot[0];
        freeSpaces[counter][1] = spot[1];
        counter++;
      }
    }
  }

  int randomChoice = random(numLEDs - numVertebrae);
  snakeFruit[0] = freeSpaces[randomChoice][0];
  snakeFruit[1] = freeSpaces[randomChoice][1];
}

void displaySnake() {
  for (int vertebra = 0; vertebra < numVertebrae; vertebra++) {
    lc.setLed(snake[vertebra][1] / 8, snake[vertebra][0], snake[vertebra][1] % 8, true);
  }
}

void displayFruit() {
  lc.setLed(snakeFruit[1] / 8, snakeFruit[0], snakeFruit[1] % 8, true);
}

bool victory() {
  if (numVertebrae == numLEDs) {
    Serial.println("VICTORY!");
    return true;
  }
  return false;
}

bool gameOver() {
  int snakeHead[2] = {snake[numVertebrae - 1][0], snake[numVertebrae - 1][1]};
  int gameOver = false;
  // Check if snake has run into wall
  if (snakeHead[0] >= 8 || snakeHead[0] < 0 || snakeHead[1] >= (numDevices * 8) || snakeHead[1] < 0) {
    gameOver = true;
  }

  // Check if snake has bitten itself
  for (int vertebra = 0; vertebra < numVertebrae - 1; vertebra++) {
    if (overlap(snakeHead, snake[vertebra])) {
      gameOver = true;
    }
  }

  if (gameOver) {
    Serial.println("GAME OVER");
    Serial.print("Score: ");
    Serial.println(numVertebrae - 5);
    Serial.println();
  }
  
  return gameOver;
}

void showMessage(String message) {
  convertMessage(message);
  displayMessage();
}

void twoSettings() {
  // Depending on the brightness setting, run two different patterns
  updateIntensity();
  if (intensity > 7) {
    freakOut();
  } else {
    nightLight();
  }
}

void readyForReception() {
  int numBytes = 16;
  // Send a signal via the Serial port announcing the Arduino is
  // ready to receive data
  // Wait for the number of bytes expected, keep sending
  // until this point
  while (Serial.available() < numBytes) {
    Serial.println('A');
    delay(300); // Don't flood the Serial with signals
  }
}

void freakOut() {
  // Gets some random columns and rows flashing to the beat
  // with the NS Grids
  if (counter % 2 == 0) {
    onAdditionalLights();
  }
  randomRow();
  if (counter % 2 == 1) {
    offAdditionalLights();
  }
  randomColumn();
}

void nightLight() {
  // night sky and a fade in the NS grids
  brightenAdditionalLights();
  nightSky();
  fadeAdditionalLights();
  nightSky();
}

void updateIntensity() {
  // Map the read intensity to 0-16 based on value of poteniometer
  // NOTE: intensity levels range from 0 to 15, -1 here used
  // for turning off the matrix LEDs
  readIntensity = map(analogRead(trimPot), 0, 1023, -1, 15);
  
  // Only update the setting on the board if
  // the intensity is different
  if (readIntensity != intensity) {
    // Load to matrix
    for (int address = 0; address < numDevices; address++) {
      if (readIntensity == -1) {
        lc.shutdown(address, true);
      } else {
        // If it was off (from old intensity setting), turn back on
        if (intensity == -1) {
          lc.shutdown(address, false);
        } else {
          lc.setIntensity(address, readIntensity);
        }
      }
    }
    // Now update the new setting so they match
    intensity = readIntensity;
  }
}

void flashAdditionalLights(int beatDelay) {
  onAdditionalLights();
  delay(beatDelay);
  offAdditionalLights();
  delay(beatDelay);
}

void brightenAdditionalLights() {
  for (int value = 0; value < map(intensity, -1, 15, 0, 255); value++) {
    analogWrite(AdditionalLights, value);
    // The lower the light setting, the slower they fade/brighten,
    // keeping the NS grids from flashing at low intensity
    // + 1 to keep from dividing by 0
    delay(50. / (intensity + 1)); 
  }
}

void fadeAdditionalLights() {
  for (int value = map(intensity, -1, 15, 0, 255); value >= 0; value--) {
    analogWrite(AdditionalLights, value);
    // The lower the light setting, the slower they fade/brighten,
    // keeping the NS grids from flashing at low intensity
    // + 1 to keep from dividing by 0
    delay(50. / (intensity + 1));
  }
}

void onAdditionalLights() {
  analogWrite(AdditionalLights, map(intensity, -1, 15, 0, 255));
}

void offAdditionalLights() {
  analogWrite(AdditionalLights, 0);
}

void randomRow() {
  int randomRow = random(0, 8);
  for (int address = 0; address < numDevices; address++) {
    lc.setRow(address, randomRow, 255);
  }
  delay(delayTime);
  for (int address = 0; address < numDevices; address++) {
    lc.setRow(address, randomRow, 0);
  }
  delay(delayTime);
}

void randomColumn() {
  int randomColumn = random(0, 16);
  lc.setColumn(randomColumn / 8, randomColumn % 8, 255);
  delay(delayTime);
  lc.setColumn(randomColumn / 8, randomColumn % 8, 0);
  delay(delayTime);
}


void rows() {
  for (int top_of_arc = 0; top_of_arc < 8; top_of_arc++) {
    for (int row = top_of_arc; row < 8; row++) {
      lc.setRow(0, row, 255);
      lc.setRow(1, row, 255);
      delay(200 - (delayFactor * row * 2 + 30));
      lc.setRow(0, row, 0);
      lc.setRow(1, row, 0);
    }
    for (int row = 7; row >= top_of_arc; row--) {
      lc.setRow(0, row, 255);
      lc.setRow(1, row, 255);
      delay(200 - (delayFactor * row * 2 + 30));
      lc.setRow(0, row, 0);
      lc.setRow(1, row, 0);
    }
  }
  for (int top_of_arc = 7; top_of_arc >= 0; top_of_arc--) {
    for (int row = top_of_arc; row < 8; row++) {
      lc.setRow(0, row, 255);
      lc.setRow(1, row, 255);
      delay(200 - (delayFactor * row * 2 + 30));
      lc.setRow(0, row, 0);
      lc.setRow(1, row, 0);
    }
    for (int row = 7; row >= top_of_arc; row--) {
      lc.setRow(0, row, 255);
      lc.setRow(1, row, 255);
      delay(200 - (delayFactor * row * 2 + 30));
      lc.setRow(0, row, 0);
      lc.setRow(1, row, 0);
    }
  }
}

void columns() {
  for (int top_of_arc = 15; top_of_arc >=0; top_of_arc--) {
    for (int column = 0; column <= top_of_arc; column++) {
      lc.setColumn(column / 8, column % 8, 255);
      delay((190 - (delayFactor * column + 30)) / 2);
      lc.setColumn(column / 8, column % 8, 0);
    }
    for (int column = top_of_arc; column >= 0; column--) {
      lc.setColumn(column / 8, column % 8, 255);
      delay((190 - (delayFactor * column + 30)) / 2);
      lc.setColumn(column / 8, column % 8, 0);
    }
  }
  for (int top_of_arc = 0; top_of_arc <= 15; top_of_arc++) {
    for (int column = 0; column <= top_of_arc; column++) {
      lc.setColumn(column / 8, column % 8, 255);
      delay((190 - (delayFactor * column + 30)) / 2);
      lc.setColumn(column / 8, column % 8, 0);
    }
    for (int column = top_of_arc; column >= 0; column--) {
      lc.setColumn(column / 8, column % 8, 255);
      delay((190 - (delayFactor * column + 30)) / 2);
      lc.setColumn(column / 8, column % 8, 0);
    }
  }
}

void nightSky() {
  int iterations = 1;
  for (int turnon = 0; turnon < iterations; turnon++) {
    lc.setLed(random(0, 2), random(0, 8), random(0, 8), true);
    //delay(75);
  }
  // Perform the iteration 4x as many times on the turning off
  // of the LEDs to have fewer on at any one time (1/4 lit)
  for (int turnoff = 0; turnoff < iterations * 4; turnoff++) {
    lc.setLed(random(0, 2), random(0, 8), random(0, 8), false);
    //delay(75);
  }
}

// Section for decoding Serial data -> message to be displayed
void serialMessage() {
  if (Serial.available() > 0) {
//    if (Serial.peek() == 254) {
//      displayPicture();
//      return;
//    }
    // if there's data, pull it in
    serialString = Serial.readString();
    //}
    //Serial.println(serialString);
    convertMessage(serialString);
  } else {
    displayMessage();
  }
}

void displayMessage() {
  for (int i = 0; i < convertedMessage.size() - 16; i++) {
    for (int col = 0; col < 16; col++) {
      lc.setColumn(col / 8, col % 8, convertedMessage[i + col]);
    }
    if (i == 0){
      delay(1000);
    }
    updateIntensity();
    if (Serial.available() > 0) {
      return;
    }
  }
  delay(600);
  for (int address = 0; address < numDevices; address++) {
    for (int col = 0; col < 8; col++) {
      lc.setColumn(address, col, 0);
    }
  }
  delay(400);
}


void convertMessage(String serialMsg) {
  // Declare the message array (will be 6x length of incoming array--5 for each char
  // and a space between each--and on the end)
  
  int index = 0; // counter for the return array
  int ASCIIValue = 0; // Initialized, will be changed for each iteration
  
  // redeclare new serialString size
  convertedMessage.resize(serialString.length() * 6);
//  convertedMessage = newConvertedMessage;
  
  // go through each char in the array
  for (int charIndex = 0; charIndex < serialMsg.length(); charIndex++) {
    for (int charCol = 0; charCol < 5; charCol++) { // each char in array has 5 cols
      ASCIIValue = (char)serialMsg[charIndex] - 32;
      convertedMessage[index] = alphabets[ASCIIValue][charCol];
      index++;
    }
    // Add the space after each character
    convertedMessage[index] = 0;
    index++;
  }
}


void serialPicture() {
  if (Serial.available() > 0) {
    for (int i = 0; i < 16; i++) {
      while (Serial.available() == 0){}
      picture[i] = Serial.read();
    }
  }
  displayPicture();
}

void displayPicture() {
  // Serial.read(); Serial.read();// Throw out the signalling byte
  // Receive the other 16 values
  for (int i = 0; i < 16; i++) {
    lc.setColumn(i / 8, i % 8, picture[i]);
  }
  delay(1000);
}
