/* 
 *  SNAKE ON A LED MATRIX
 *  Creator : Thomas Cibils
 *  Last update : 02.02.2019
 *  Code original source : http://forum.arduino.cc/index.php?topic=8280.0
 *  FastLED tuto : https://github.com/FastLED/FastLED/wiki/Basic-usage - used for WS2812B 5050 RGB LED Strip 5M 150 300 Leds 144 60LED/M Individual Addressable 5V
 *  */

#include <Bounce2.h>
#include <TimerOne.h>
#include "FastLED.h"

const unsigned int numberOfRows = 3;                          // Number of rows
const unsigned int numberOfColumns = 3;                       // Number of coumns
const unsigned int NUM_LEDS = numberOfRows * numberOfColumns; // Number of LEDs

CRGB leds[NUM_LEDS]; // Defining leds table for FastLed

// Pin used from the arduino
const unsigned int leftButton = A1;       // Input pin for button 1
const unsigned int rightButton = A2;      // Input pin for button 2
#define DATA_PIN 6                        // Output pin for FastLed

struct pointOnMatrix {
  int lineCoordinate;
  int columnCoordinate;
};

// Original colours for leds.
const unsigned int empty = 0;
const unsigned int red = 1;
const unsigned int green = 2;
const unsigned int orange = 3;

// Directions for the snake head
const unsigned int directionUp = 0;
const unsigned int directionRight = 1;
const unsigned int directionDown = 2;
const unsigned int directionLeft = 3;

// Direction variable, initial set-up to "right"
const int directionInitial = directionRight;
int static snakeDirection = directionInitial;

// LED Matrix
// top column is from 0 to 7, bottom one from 56 to 63 (for a 8x8 matrix)
byte LEDMatrix[numberOfRows][numberOfColumns];

// Snake presence on the matrix.
// At index 0 will be the head position on the matrix, and the following indexes will be the current position of each body part of the snake
// Matrix is as follows : couting 0,0 on top left; 0, numberOfColumns-1 on top right; and numberOfRows-1, numberOfColumns-1 on bottom right
// We will put -1 if no part of the snake currently exist at this index.
// For instance, snake[5] = (4,3) (on a 6x6 matrix) means that the 6th body part of the snake is on led line 4, column 3 (counting the columns from 1)
// snake[7] = (-1,-1) means that the snake is shorter than 7 body parts.
pointOnMatrix snake[numberOfRows * numberOfColumns];

// New snake is used to make snake move forward
pointOnMatrix newsnake[numberOfRows * numberOfColumns];

pointOnMatrix startingPoint = {1, 0};     // Game starting point, line 1, column 0.
pointOnMatrix snakehead = {0, 0};       // Starting point for the snake head

unsigned int applecaught = 0;
pointOnMatrix apple = {0, 0};         // Sets where the apple is on the matrix

const int moveSpeed = 1000;

unsigned long lastMillis = 0;

Bounce bouncer1 = Bounce(leftButton, 10);
Bounce bouncer2 = Bounce(rightButton, 10);

int leftButtonValue = LOW;
int rightButtonValue = LOW;
int lastLeftButtonValue = LOW;
int lastRightButtonValue = LOW;

void setup() {

  Serial.begin(9600);
  // WTF is that
  // randomSeed(analogRead(5));

  // Set matrix pins to output
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  // Set button pins to input
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  // Enable internal pullup resistors (woot?)
  // digitalWrite(leftButton, HIGH);
  // digitalWrite(rightButton, HIGH);

  // We start the game.
  resetSnake();
}

void loop() {
  // We first check if the button have been pressed, using a "debouncer"

    leftButtonValue = analogRead(leftButton);
  Serial.print("leftButtonValue : ");
  Serial.print(leftButtonValue);
  Serial.print("\n");
    if (leftButtonValue == 0 && lastLeftButtonValue > 800) {
//     // If the button 1 has been pressed, we go left
        prevDirection();
   }
  lastLeftButtonValue = leftButtonValue; // And we update what we read just after
   
    rightButtonValue = analogRead(rightButton);
 // Serial.print("rightButtonValue : ");
 // Serial.print(rightButtonValue);
 // Serial.print("\n");
    if (rightButtonValue == 0 && lastRightButtonValue > 800) { 
     // If the button 2 has been pressed, we go right
        nextDirection();
    }
    lastRightButtonValue = rightButtonValue; // And we update what we read just after
  
  // We make the snake digitalized values move
  // This function includes a call to "updateLEDMatrix", so the matrix contains the correct values

//  Serial.print("We're on the loop, and going to move the snake. \n");
  moveSnake();
//  Serial.print("Snake moved, we will print the following on the matrix : \n");
//  digitalOutputDisplay();

  // We then light up the physical display of the led matrix, according to the values stored
//  Serial.print("We're on the loop, and going to update the physical display. \n");
  outputDisplay();

  // To avoid going too fast, we include 1 millisecond of delay in the loop
  delay(1);

 // Serial.print("End of the loop, we're starting it again. \n");
}

void nextDirection() {

  Serial.print("\n Click on right button detected, changing direction. It is now ");

  snakeDirection++;
  if (snakeDirection > 3) {
    snakeDirection = 0;
  }

  Serial.print(snakeDirection);
  Serial.print(". \n");
}

void prevDirection() {
  Serial.print("\n Click on left button detected, changing direction. It is now ");
  snakeDirection--;
  if (snakeDirection < 0) {
    snakeDirection = 3;
  }
  Serial.print(snakeDirection);
  Serial.print(". \n");
}

// We reset the game, create a 1-dot snake, and an apple.
void resetSnake() {

  Serial.print("\n We've entered the reset function. \n");
  // Create empty snake

  Serial.print("Snake values created : ");
  for (int i = 0; i < numberOfRows * numberOfColumns; i++)  {
    snake[i].lineCoordinate = -1;
    snake[i].columnCoordinate = -1;
    newsnake[i].lineCoordinate = -1;
    newsnake[i].columnCoordinate = -1;

    Serial.print("{");
    Serial.print(snake[i].lineCoordinate);
    Serial.print(",");
    Serial.print(snake[i].columnCoordinate);
    if (i < numberOfColumns * numberOfRows - 1) {
      Serial.print("}, ");
    }
    else {
      Serial.print("}. \n");
    }
  }


  // Whole LEDMatrix = 0, it's now empty.
  Serial.print("\n We now clear the Matrix. \n");
  clearLEDMatrix();

  Serial.print("We define the snake head on the starting point : ");
  LEDMatrix[startingPoint.lineCoordinate][startingPoint.columnCoordinate] = orange;  // Add starting dot
  snakehead = startingPoint;                         					   		   // We put the snake head on the starting point.
  snake[0].lineCoordinate = snakehead.lineCoordinate;            				    // We set the head of the snake.
  snake[0].columnCoordinate = snakehead.columnCoordinate;
  Serial.print("{");
  Serial.print(snake[0].lineCoordinate);
  Serial.print(",");
  Serial.print(snake[0].columnCoordinate);
  Serial.print("}. \n");

  generateApple();    								                             // We define a spot for the apple.
  Serial.print("The apple will be placed on (seen from resetSnake) : ");
  LEDMatrix[apple.lineCoordinate][apple.columnCoordinate] = green;         // We put the apple in the matrix, as the spot at which it has been generated.

  Serial.print("{");
  Serial.print(apple.lineCoordinate);
  Serial.print(",");
  Serial.print(apple.columnCoordinate);
  Serial.print("}. \n");

  Serial.print("The snake will go to the direction ");
  snakeDirection = directionInitial;												// We also initalize the direction
  Serial.print(snakeDirection);
  Serial.print("\n");
}

// Makes the whole "LEDMatrix" equals to 0, i.e. each LED is off
void clearLEDMatrix() {
  /*
    Serial.print("\n We've entered the Matrix Clearing. \n");
    Serial.print("Matrix values (from ");
    Serial.print(numberOfRows);
    Serial.print(" lines to ");
    Serial.print(numberOfColumns);
    Serial.print(" columns) : \n");
  */
  for (int i = 0; i < numberOfRows; i++)  {
    for (int j = 0; j < numberOfColumns; j++) {

      LEDMatrix[i][j] = empty;
      //Serial.print(LEDMatrix[i][j]);
    //  if (j < numberOfColumns - 1) {
        // Serial.print(", ");
    //  }
    //  if (j == numberOfColumns - 1) {
        //Serial.print("\n");

    //  }
    }
  }

  //  Serial.print("\n");
}

// We update the physical display of the LED matrix
void outputDisplay() {
   
  for(int rowIndex = 0; rowIndex < numberOfRows; rowIndex++) {
    for(int columnIndex = 0; columnIndex < numberOfColumns; columnIndex++) {
      if(LEDMatrix[rowIndex][columnIndex] == empty) {leds[rowIndex*numberOfRows + columnIndex] = CRGB::Black;}
      if(LEDMatrix[rowIndex][columnIndex] == red) {leds[rowIndex*numberOfRows + columnIndex] = CRGB::Red;}
      if(LEDMatrix[rowIndex][columnIndex] == green) {leds[rowIndex*numberOfRows + columnIndex] = CRGB::Green;}
      if(LEDMatrix[rowIndex][columnIndex] == orange) {leds[rowIndex*numberOfRows + columnIndex] = CRGB::Orange;}
    }
  }
  
  // Display the matrix physically
  FastLED.show(); 
}

// We update the digital display of the LED matrix
void digitalOutputDisplay() {
  Serial.print("\n We print digitally the current theoritical state of the LED Matrix : \n");
  for (int i = 0; i < numberOfRows; i++) {
    for (int j = 0; j < numberOfColumns; j++) {
      Serial.print(LEDMatrix[i][j]);
      if (j < numberOfColumns - 1) {
        Serial.print(", ");
      }
      else {
        Serial.print("\n");
      }
    }
  }
}

// The game - the real one.
void moveSnake() {

  // We start by moving the snake's head in function of the current direction

  if (millis() - lastMillis < moveSpeed) {
 //   Serial.print("Time interval too short. Snake not moved \n");
  }


  // If enough time has passed,
  if (millis() - lastMillis >= moveSpeed) {

    // In function of the direction,
    switch (snakeDirection) {
      // if we're going up,
      case directionUp:
        // and if we're already at the top of the matrix,
        if (snakehead.lineCoordinate == 0) {
          // then we go to the bottom of the matrix.
          snakehead.lineCoordinate = numberOfRows - 1;
        }
        // and if we're not at the top of the matrix,
        else {
          // we simply get the line number one rank up.
          snakehead.lineCoordinate -= 1;
        }
        break;

      // Now if we're going right,
      case directionRight:
        // If we're not already at the right end of the matrix,
        if (snakehead.columnCoordinate < numberOfColumns - 1) {
          // we simply avance the head of one column, hence to the right.
          snakehead.columnCoordinate += 1;
        }
        // But if we're on the last column on the right end of the matrix,
        else {
          // we set the column coordinate of the head of the snake to the first colum (on the left end) of the matrix.
          snakehead.columnCoordinate = 0;
        }
        break;

      // Same kind of business if we go down.
      case directionDown:
        // If we're on the bottom line of the matrix,
        if (snakehead.lineCoordinate == numberOfRows - 1) {
          // we set the line coordinate to 0, to go to the top row of the matrix.
          snakehead.lineCoordinate = 0;
        }
        // But if we're on the "middle" of the matrix
        else {
          // we simply increase by one the line coordinate of the snake's head.
          snakehead.lineCoordinate += 1;
        }
        break;

      // And finally for the left direction,
      case directionLeft:
        // If we're on the left column of the matrix,
        if (snakehead.columnCoordinate == 0) {
          // We go the the full right column.
          snakehead.columnCoordinate = (numberOfColumns - 1);
        }
        // But if we're on the "fat" of the matrix
        else {
          // We decrease the column coordinate of the snake head by one.
          snakehead.columnCoordinate -= 1;
        }
        break;

      default:
        // wtf?
        break;
    }

    // Check if we have the apple
    if (snakehead.lineCoordinate == apple.lineCoordinate && snakehead.columnCoordinate == apple.columnCoordinate) {
      // We encode the fact that the apple is caught using "applecaught",
      applecaught = 1;

      // and we generate a new apple.
      // generateApple(); // To avoid redundance
    }

    // We check if we've hit ourselves. Loop starts to one, as we don't want the head hiting itself.
    for (int i = 1; i < numberOfRows * numberOfColumns; i++) {
      // If the head of the snake is on the same position as one of the body parts, we end the game.
      if (snakehead.lineCoordinate == snake[i].lineCoordinate && snakehead.columnCoordinate == snake[i].columnCoordinate) {
        endGame();
      }
    }

    // We start moving the snake
    // The define the head as the start of the new snake.
    newsnake[0] = snakehead;

    // We now move the snake.
    // We parse the snake table to see where it ends
    for (int n = 0; n < numberOfColumns * numberOfRows - 1; n++) {
      // If we've reached the end of the snake,
      if (snake[n].lineCoordinate == -1 || snake[n].columnCoordinate == -1) {
        // we define the "snake tail" point as this last point reached,
        //  snaketail = snake[n-1]; // (which is useless so I comment it)
        // if we didn't caught an apple,
        if (applecaught == 0) {
          // We destroy the last bit of the newsnake to let it fade out
          newsnake[n].lineCoordinate = -1;
          newsnake[n].columnCoordinate = -1;
          // In the contrary case, we will keep the snake to be one unit longer.
        }
        // This takes us out of the loop, I think ?
        // break;
      }
      // In all cases, we copy the old snake in the new one with one unit of lag
      newsnake[n + 1] = snake[n];
    }

    // We copy the new snake in the actual snake, in order to display it.
    for (int m = 0; m < numberOfColumns * numberOfRows; m++) {
      snake[m] = newsnake[m];
    }

    // Now if the apple has been caught,
    if (applecaught == 1) {
      // We generate a new apple
      // Redundance avoided
      generateApple();
      // And we set the parameter to 0, waiting for the player to catch one more apple to set it back to one.
      applecaught = 0;
    }

    // We update the variable LEDMatrix to reflect those changes, so we can later display it
    updateLEDMatrix();

    // And we update the timer.
    lastMillis = millis();
  }
}

// Updating the LED Matrix after each step
void updateLEDMatrix() {

 // Serial.print("\n We will now update the LED Matrix. \n");
  // We first empty it
  // Serial.print("We first clear it. \n");
  clearLEDMatrix();

  //Serial.print("We put the snake on it. \n");
  //Serial.print("We find that the snake is on the following places : ");
  // Then we light up in red the whole snake - except the head, hence the loop starts from 1
  for (int i = 1; i < numberOfRows * numberOfColumns; i++) {

    // We scan the matrix, and we light up the dots where the snake is.
    if (snake[i].lineCoordinate > -1 && snake[i].columnCoordinate > -1) {
      /*Serial.print("{");
      Serial.print(snake[i].lineCoordinate);
      Serial.print(", ");
      Serial.print(snake[i].columnCoordinate);
      Serial.print("}, ");*/
      LEDMatrix[snake[i].lineCoordinate][snake[i].columnCoordinate] = red; // red = 1, so the dot will light up.
    }
  }
/*
  Serial.print("We moreover print in orange the snake's head, which is in {");
  Serial.print(snakehead.lineCoordinate);
  Serial.print(", ");
  Serial.print(snakehead.columnCoordinate);
  Serial.print("}. \n");*/
  LEDMatrix[snakehead.lineCoordinate][snakehead.columnCoordinate] = orange; // orange = 3, so the dot will light up.

/*
  Serial.print("And in green the apple, which is in {");
  Serial.print(apple.lineCoordinate);
  Serial.print(", ");
  Serial.print(apple.columnCoordinate);
  Serial.print("}. \n");*/
  // And we light up the apple, using its coordinates
  LEDMatrix[apple.lineCoordinate][apple.columnCoordinate] = green; // green = 2, so the dot will light up.
}

void generateApple() {
  Serial.print("\n We are now creating an apple. \n");
  // We draw a number for the X and the Y coordinates
  // TO BE CLARIFIED : HOW DOES RANDOM WORK
  const int appleLineNumber = random(numberOfRows);
  const int appleColumnNumber = random (numberOfColumns);
  Serial.print("The apple will be in ");
  Serial.print("{");
  Serial.print(appleLineNumber);
  Serial.print(", ");
  Serial.print(appleColumnNumber);
  Serial.print("}. \n");

  // And we set the apple to the coordinates created
  apple = {appleLineNumber, appleColumnNumber};

  // We check that the apple didn't appear on the snake.
  // We scan the whole grid
  for (int i = 0; i < numberOfRows * numberOfColumns; i++) {
    // and if the apple drops on the snake
    if (apple.lineCoordinate == snake[i].lineCoordinate && apple.columnCoordinate == snake[i].columnCoordinate) {
      // We re-create a new apple, hopefully somewhere else - in the worst case, we will again generate a new one.
      generateApple();
      Serial.print("But the snake was already there, so we re-generate an apple. \n");

    }
  }
}





// We end the game with a curtain of light
void endGame() {

  Serial.print("\n GAME OVER \n");
  clearLEDMatrix();
  // We light up the rows one by one, with .2 sec of delay between each
  for (int i = 0; i < numberOfRows; i++) {
    for (int j = 0; j < numberOfColumns; j++) {
      LEDMatrix[i][j] = green;
      updateLEDMatrix();
      outputDisplay();
    }
    
    delay(200);
  }
  // It stays on for 4 seconds
  delay(4000);

  Serial.print("We reset the game. \n");
  // And we restart the game.
  resetSnake();
}
