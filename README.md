#Arcade Basketball

The following project was programmed in C with the Atmel Studio 6.2 IDE.

####Notable Components Used

- 16X2 LCD Display: Controls instructions to player. Also displays score.
- 7 Segment Displays: Used for countdown time and for game logic (daisy chained together for saving PINS on the microcontroller).
- NEMA-17 Stepper Motor: Used for gameplay.
- EEPROM Memory: Built into the microcontroller this memory is used for storing the High Score in the first game mode.
- Adafruit TB6612 1.2A DC/Stepper Motor Driver Breakout Board: Drives the Stepper Motor and allows usage of 12V power supply to drive the motor.
- Reflective IR Sensor: Detects made baskets.
- 74HC595 Shift Registers: Used to maximize PIN usage on the micro controller.
- ATmega1284 Micro Controller: Controls the entire project.  

####Project Description

In this project the single ATmega1284 is used to do all the logic needed. It controls all the components with the use of shift registers to maximize PIN output. The components do as listed above and working together with the help of a provided Task Scheduler coding outline provided in the Lab portion of the course. Descriptions of the game modes are stated below.
 
Game mode 1 (Time Attack): Classic arcade mode where the user tries to break the pre-set high score. The game lasts a total of 40 seconds; displayed on the 7-segment displays, with the first 25 seconds having the basket stationary and the shots are worth 2 points. The last 15 seconds the NEMA-17 stepper motor moves the basket from left to right and those baskets in that time frame are worth a total of 3 points each. If the user beats the current high score they get a winning prompt and their high score is saves to the EEPROM memory on the ATmega 1284 chip. If the user does not beat the high score they get a simple “try again” message. Both outcomes will loop to the main menu afterwards.
 
Game mode 2 (Horse version “Bit”): Here the user is playing a friend in whom the classic game of “Horse” meets its CS120B rendition “Bit”. Here buttons A and B are used to move the basket to where the desired shooter wants it (A goes left, B goes right). Once the shooter gets the basket to where they want to press the C button which locks in their position. They now get 5 seconds to make their basket. If they make it the next player has to make theirs in order to avoid getting a letter. The second player shoots directly after the first player if the first player makes the shot. If the first player misses, the second player now has control over where they want to move the basket and they repeat the above steps. The game is played until one player loses, meaning they spell out the word “Bit”. The system then loops back to the main menu.
 
In both games modes a user can push the 4th button “R” which is a soft reset. Here everything is reset and the basket moves to the original position with all the scores being reset.

####Video Link
Here is a YouTube link to see how the project actually works.

https://www.youtube.com/watch?v=twQz8b-qCdU&feature=youtu.be

####Original Work Statement

I acknowlegde all content contained herin, excluding template or example code, is my own original work.
