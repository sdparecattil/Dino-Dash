///////////////////////////////////////////////////////////////////////////////////////////////
// Group Members: Sebastian Parecattil, John Finch
// Description: T-Rex Run game on a SSD1306 OLED Display
//				Configures joystick to trigger jumping and ducking animations
//				Spawn obstacles randomly on the screen
//				Endlessly scrolls the screen until the T-Rex has hit an object
//				When object is hit the game ends
//				Touch sensor is used to reset the game to start again
// Resources: Peter Fleury's i2c library
//			  http://www.peterfleury.epizy.com/index.html?i=2
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef F_CPU
#define F_CPU 16000000UL    // 16 MHz clock speed.
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "avr/sfr_defs.h"
#include <stdio.h>
#include "i2cmaster.h"
#include <time.h>

#include <inttypes.h>
#include <compat/twi.h>

void sendOneCommandByte(unsigned char cmd);
void sendTwoCommandByte(unsigned char cmdOne, unsigned char cmdTwo);
void sendData(unsigned char data);
void oled_init();
void position(unsigned char x, unsigned char y);
void clearDisplay();
void clearTopTwoPages();
void drawRex();
void drawCactus();
void drawPterodactyl();
void background();
void scrollLeft();
void preventScrollBack();
void jumpingRex();
void jumpingOne(unsigned char top, unsigned char bottom);
void jumpingTwo(unsigned char top, unsigned char bottom);
void jumpingThree(unsigned char top, unsigned char middle, unsigned char bottom);
void jumpingFour(unsigned char top, unsigned char middle, unsigned char bottom);
void jumpingFive(unsigned char top, unsigned char middle, unsigned char bottom);
void jumpingSix(unsigned char top, unsigned char middle, unsigned char bottom);
void jumpingSeven(unsigned char top, unsigned char middle, unsigned char bottom);
void jumpingEight(unsigned char top, unsigned char middle, unsigned char bottom);
void fallingClear(unsigned char page);
void duckingRex();
void unduckingRex();
void duckingOne();
void duckingTwo();
void duckingThree();
void duckingFour();
void generateRandomEnemy();
void checkPterodactyl();
void checkCactus();
void activeCounter();
uint8_t max(uint8_t one, uint8_t two);
void collisionCheck();
void stopDisplay();
void resetCactus();
void resetPterodactyl();
void gameLoop();
void buzzerToggle();
void buzzerOn();
void buzzerOff();
void gameStart();
void letterDisplay(uint8_t x, uint8_t index);
void stickPress();
void gameEnd();
void displayScore();
void drawScore();
void displayNumber(int number, int x);
void displayFinalScore();
void letterDisplay(uint8_t x, uint8_t index);

void Timer0Settings();
void _delay_5ms();
void _delay_10ms();

void LEDOn(void);
void LEDOff(void);
void ADCint(void);
unsigned int read_adc(unsigned char adc_input);
void convertADCToVoltage(void);

// T-Rex Bytes
const unsigned char Rex[][15] PROGMEM = {
	{ 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xE0, 0xF8, 0xFC, 0x74, 0x5C, 0x5C, 0x18 },
	{ 0x03, 0x07, 0x07, 0x0F, 0xFF, 0xBF, 0x1F, 0x0F, 0x1F, 0xFF, 0x87, 0x01, 0x03, 0x00, 0x00 }
};

// Cactus Bytes
const unsigned char Cactus[][6] PROGMEM = {
	{ 0x00, 0x00, 0xE0, 0xE0, 0x00, 0x00 },
	{ 0x0F, 0x08, 0xFF, 0xFF, 0x08, 0x0F }
};

// Pterodactyl Bytes
const unsigned char Pterodactyl[11] PROGMEM = {
	0x04, 0x06, 0x07, 0x0C, 0xFC, 0x7C, 0x1C, 0x1C, 0x14, 0x14, 0x04 
};

// Two Page Letter Bytes
const unsigned char Letters[][14] PROGMEM = {
	{ 0xFF, 0x41, 0x41, 0x41, 0x41, 0x3E, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //P
	{ 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x00 }, //U
	{ 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x00 }, //S
	{ 0xFF, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00 }, //H
	{ 0x01, 0x01, 0x01, 0xFF, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00 }, //T
	{ 0xFF, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x00, 0xFF, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x00 }, //O
	{ 0x01, 0x01, 0x01, 0xFF, 0x01, 0x01, 0x01, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80 }, //I
	{ 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00 }, //C
	{ 0xFF, 0x80, 0x40, 0x30, 0x0C, 0x03, 0x00, 0xFF, 0x01, 0x02, 0x0C, 0x30, 0xC0, 0x00 }, //K
	{ 0xFF, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00 }, //A
	{ 0xFF, 0xC1, 0x41, 0x41, 0x41, 0x3E, 0x00, 0xFF, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00 }, //R
	{ 0xFF, 0x81, 0x81, 0x81, 0x81, 0x01, 0x00, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00 }, //E
	{ 0xFF, 0x0F, 0xF0, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x0F, 0xF0, 0xFF, 0x00 } //N
};

// One Page Letter Bytes
const unsigned char scoreLetters[][5] PROGMEM = {
	{ 0x8F, 0x89, 0x89, 0x89, 0xF9 }, //S
	{ 0xFF, 0x81, 0x81, 0x81, 0x81 }, //C
	{ 0xFF, 0x81, 0x81, 0x81, 0xFF }, //O
	{ 0xFF, 0x19, 0x29, 0x49, 0x86 }, //R
	{ 0xFF, 0x89, 0x89, 0x89, 0x81 } //E
};

// One Page Number Bytes
const unsigned char numbers[][4] PROGMEM = {
	{ 0xFF, 0x81, 0x81, 0xFF }, //0
	{ 0x00, 0xFF, 0x00, 0x00 }, //1
	{ 0xF9, 0x89, 0x89, 0x8F }, //2
	{ 0x89, 0x89, 0x89, 0xFF }, //3
	{ 0x0F, 0x08, 0x08, 0xFF }, //4
	{ 0x8F, 0x89, 0x89, 0xF9 }, //5
	{ 0xFF, 0x89, 0x89, 0xF9 }, //6
	{ 0x01, 0x01, 0x01, 0xFF }, //7
	{ 0xFF, 0x89, 0x89, 0xFF }, //8
	{ 0x0F, 0x09, 0x09, 0xFF }, //9
};

uint8_t scrollCount = 0;
uint8_t cactusOne = 0;
uint8_t cactusTwo = 0;
uint8_t pteroOne = 0;
uint8_t pteroTwo = 0;
uint8_t rexMode = 0;
uint8_t stop = 0;
uint8_t resetCount = 0;
uint8_t pressCondition = 1;
float score = 0;
int lastOnes = 0;
int lastTens = 0;
int lastHundreds = 0;
int lastThousands = 0;

int main (void) {
	DDRC = 0x04; // Reset Toggle output
	PORTC |= 0x04; // Setting Reset to logic 1
    i2c_init(); // Initializing the OLED
	DDRD = 0x90;	// Sets PD5 to an output for the LED
	ADCint(); // Initializing the ADC
	oled_init(); // Initializing the OLED
	Timer0Settings(); // Timer 0 Settings
	
	
	// External Interrupt Control Register
	// ISC11 - 1, ISC10 - 1
	// Rising edge of INT1 generates an interrupt request
	EICRA |= (1 << ISC11) | (1 << ISC10);
	// External Interrupt Mask Register
	// INT1 - 1
	// External Interrupt Request 1 Enable
	EIMSK |= (1 << INT1);
	sei();
	
	gameStart(); // Display the start message to the screen
	background(); // Displays the background
	drawRex(); // Displays a T-Rex
	stickPress(); // Waits to start game until button has been pressed
	gameLoop(); // Loop while game is running

}

// Loops until the joystick is pressed down
void stickPress() {
	while(pressCondition) {
		if ((PIND & 0x40) == 0) {
			clearTopTwoPages(); // Clears the message from the top of screen
			while ((PIND & 0x40) == 0) {
				
			}
			drawScore(); // Draw the letter for score
			// Displays 0 0 0 0 on the screen
			displayNumber(0, 38);
			displayNumber(0, 44);
			displayNumber(0, 50);
			displayNumber(0, 56);
			pressCondition = 0;
		}
	}
}

// Loop for the game
void gameLoop() {
	while(1) {
		preventScrollBack(); // Clears the first eight vertical lines on the 6th page
		drawRex(); // Displays a T-Rex on the screen
		scrollLeft(); // Shifts the content on the OLED one pixel to the left
		unsigned int adcReading = read_adc((unsigned char)0x00); // Reading the ADC output
		// Checking if the joystick is tilted up
		if (adcReading < 300) {
			LEDOn(); // Turn LED on
			buzzerToggle(); // Sounds the buzzer shortly
			jumpingRex(); // Jumping animation
		}
		//Checking if the joystick is tilted down
		else if (adcReading > 650) {
			duckingRex(); // Ducking animation
			// Checking when the joystick returns to rest position
			while(adcReading > 650) {
				duckingFour(); // Keeps the T-Rex in ducking mode as long as the stick is tilted down
				adcReading = read_adc((unsigned char)0x00); // Reading the ADC output
				preventScrollBack(); // Clears the first eight vertical lines on the 6th page
				scrollLeft(); // Shifts the content on the OLED one pixel to the left
				_delay_5ms();
			}
			unduckingRex(); // Animation that returns the T-Rex to original position when the joystick is return to rest position
		}
		else {
			LEDOff(); // LED is off when joystick is in rest position
		}
		
		_delay_5ms();
	}
}

// Checks if the T-Rex has collided with an active object
void collisionCheck() {
	uint8_t tempMax = max(max(cactusOne, cactusTwo), max(pteroOne, pteroTwo)); // Checks which active object is closest to collision point
	// Checks which object is active
	if (tempMax != 0) {
		// First Cactus
		if (cactusOne == tempMax) {
			// Checks if the T-Rex will collide with the cactus one pixel before entering collision zone
			if ((cactusOne == 98) && !((rexMode >= 10) && (rexMode <= 24))) {
				stopDisplay();
			}
			// Checks if the T-Rex will collide with the cactus while in collision zone
			else if ((cactusOne > 98) && !((rexMode >= 11) && (rexMode <= 24))) {
				stopDisplay();
			}
		}
		// Second Cactus
		else if (cactusTwo == tempMax) {
			// Checks if the T-Rex will collide with the cactus one pixel before entering collision zone
			if ((cactusTwo == 98) && !((rexMode >= 10) && (rexMode <= 24))) {
				stopDisplay();
			}
			// Checks if the T-Rex will collide with the cactus while in collision zone
			else if ((cactusTwo > 98) && !((rexMode >= 11) && (rexMode <= 24))) {
				stopDisplay();
			}
		}
		// First pterodactyl
		else if (pteroOne == tempMax) {
			// Checks if the T-Rex will collide with the pterodactyl one pixel before entering collision zone
			if ((pteroOne == 92) && !((rexMode >= 10) && (rexMode <= 24)) && !(rexMode >= 27)) {
				stopDisplay();
			}
			// Checks if the T-Rex will collide with the pterodactyl while in collision zone
			else if ((pteroOne > 92) && !((rexMode >= 11) && (rexMode <= 24)) && !(rexMode == 28)) {
				stopDisplay();
			}
		}
		// Second pterodactyl
		else if (pteroTwo == tempMax) {
			// Checks if the T-Rex will collide with the pterodactyl one pixel before entering collision zone
			if ((pteroTwo == 92) && !((rexMode >= 10) && (rexMode <= 24)) && !(rexMode >= 27)) {
				stopDisplay();
			}
			// Checks if the T-Rex will collide with the pterodactyl while in collision zone
			else if ((pteroTwo > 92) && !((rexMode >= 11) && (rexMode <= 24)) && !(rexMode == 28)) {
				stopDisplay();
			}
		}
		
	}
}

// Deactivates a cactus once it is cleared
void resetCactus() {
	if (cactusOne >= 119) {
		cactusOne = 0;
		score++;
	}
	else if (cactusTwo >= 119) {
		cactusTwo = 0;
		score++;
	}
	displayScore();
}

// Deactivates a pterodactyl once it is cleared
void resetPterodactyl() {
	if (pteroOne >= 119) {
		pteroOne = 0;
		score++;
	}
	else if (pteroTwo >= 119) {
		pteroTwo = 0;
		score++;
	}
	displayScore();
}

// Stop the display when a collision has occurred
void stopDisplay() {
	sendOneCommandByte(0x2E); // Stops the screen from scrolling
	clearTopTwoPages(); // Clears the score from the screen
	gameEnd(); // Displays the message to reset the screen
	if (resetCount == 0) {
		displayFinalScore(); // Displays the final score on a lower part of the screen
		buzzerOn(); // Sounds the buzzer when the game has ended
		_delay_ms(2000);
		buzzerOff(); // Turns the buzzer off
		resetCount++;
	}
}

// Finds the max between two 8 bit values
uint8_t max(uint8_t one, uint8_t two) {
	if (one > two) {
		return one;
	}
	else {
		return two;
	}
}

// Counts for each active object every time the screen is shifted one pixel
void activeCounter() {
	if (cactusOne > 0) {
		cactusOne++;
	}
	if (cactusTwo > 0) {
		cactusTwo++;
	}
	if (pteroOne > 0) {
		pteroOne++;
	}
	if (pteroTwo > 0) {
		pteroTwo++;
	}
}

// Makes an cactus active when it is displayed on the screen
void checkCactus() {
	if (cactusOne == 0) {
		cactusOne++;
	}
	else if (cactusTwo == 0) {
		cactusTwo++;
	}
}

// Makes an pterodactyl active when it is displayed on the screen
void checkPterodactyl() {
	if (pteroOne == 0) {
		pteroOne++;
	}
	else if (pteroTwo == 0) {
		pteroTwo++;
	}
}

// Generates a random cactus or pterodactyl every 128 shifts
void generateRandomEnemy() {
	scrollCount++; // Increments every time the screen is shifted left
	if (scrollCount == 128) {
		int random_num = rand() % 2;
		double scaled_num = random_num + 1; // Generates a random number either 1 or 2
		// If random number is 1 draw a cactus on the screen and make it active
		if (scaled_num == 1) {
			drawCactus();
			checkCactus();
		}
		// If random number is 2 draw a pterodactyl on the screen and make it active
		else {
			drawPterodactyl();
			checkPterodactyl();
		}
		scrollCount = 0;
	}
}

// Sets all the settings needed for Timer 0
void Timer0Settings() {
	TCNT0 = 0x00; // Timer/Counter Register for Timer 0, Setting to 0
	// TCCR0B - Timer/Counter Control Register for Timer 0
	// CS02 - 0, CS01 - 0, CS00 - 1
	// Prescale Timer by 1024
	TCCR0B |= (1 << CS02) | (0 << CS01) | (1 << CS00);
	TIMSK0 |= (1 << TOIE0);
}

// Triggered when the touch sensor is pressed
ISR(INT1_vect) {
	// Checks if the game is in its end state
	if (resetCount > 0) {
		// Toggles port to trigger reset
		PORTC &= ~(0x04);
		_delay_10ms();
		PORTC |= 0x04;
	}
}

// Checks if the T-Rex has collided with any active objects constantly
ISR(TIMER0_OVF_vect) {
	collisionCheck();
}


// Creates a delay of 5 ms
void _delay_5ms() {
	unsigned char counter3 = 0;
	while (counter3 != 5) {
		_delay_loop_2(4000);
		counter3++;
	}
}

// Creates a delay of 10 ms
void _delay_10ms() {
	unsigned char counter3 = 0;
	while (counter3 != 5) {
		_delay_loop_2(8000);
		counter3++;
	}
}

// Initializes OLED display
void oled_init() {
	_delay_ms(100);
	sendOneCommandByte(0xAE);
	
	sendTwoCommandByte(0xD5,0x80);
	sendTwoCommandByte(0xA8,0x3F);
	sendTwoCommandByte(0xD3,0x00);
	sendOneCommandByte(0x40);
	sendOneCommandByte(0xA1);
	sendOneCommandByte(0xC8);
	sendTwoCommandByte(0xDA,0x12);
	sendTwoCommandByte(0x81,0x66);
	sendTwoCommandByte(0xD9,0xF1);
	sendTwoCommandByte(0xD8,0x30);
	sendOneCommandByte(0xA4);
	sendOneCommandByte(0xA6);
	
	sendTwoCommandByte(0x8D,0x14);
	sendOneCommandByte(0xAF);
	_delay_ms(100);
	
	sendTwoCommandByte(0x20,0x10);
	
	clearDisplay();
	_delay_ms(1000);
	
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Ducking animation for the T-Rex
void duckingRex() {
	duckingOne();
	_delay_10ms();
	duckingTwo();
	_delay_10ms();
	duckingThree();
	_delay_10ms();
	position(8,5);
	for (int i = 0; i < 18; i++) {
		sendData(0x00);
	}
	duckingFour();
}

// Animation to return T-Rex back to original state
// Calls ducking animation backward with additional clearing
void unduckingRex() {
	position(25,6);
	sendData(0x00);
	duckingThree();
	_delay_10ms();
	position(24,5);
	sendData(0x00);
	position(24,6);
	sendData(0x00);
	duckingTwo();
	_delay_10ms();
	position(23,5);
	sendData(0x00);
	duckingOne();
	_delay_10ms();
	position(22,5);
	sendData(0x00);
	rexMode = 0;
	drawRex();
}

// First frame of the ducking animation
void duckingOne() {
	rexMode = 25;
	position(8,5);
	sendData(0x80);
	sendData(0x80);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x80);
	sendData(0xC0);
	sendData(0xC0);
	sendData(0xC0);
	sendData(0xF0);
	sendData(0xF8);
	sendData(0xE8);
	sendData(0xB8);
	sendData(0xB8);
	sendData(0x30);
	
	position(8,6);
	sendData(0x03);
	sendData(0x03);
	sendData(0x07);
	sendData(0x07);
	sendData(0xFF);
	sendData(0xBF);
	sendData(0x1F);
	sendData(0x0F);
	sendData(0x1F);
	sendData(0xFF);
	sendData(0x87);
	sendData(0x02);
	sendData(0x06);
	sendData(0x00);
	sendData(0x00);
}

// Second frame of the ducking animation
void duckingTwo() {
	rexMode = 26;
	position(8,5);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x80);
	sendData(0x80);
	sendData(0x80);
	sendData(0x80);
	sendData(0xE0);
	sendData(0xF0);
	sendData(0xD0);
	sendData(0x70);
	sendData(0x70);
	sendData(0x60);
	
	position(8,6);
	sendData(0x0F);
	sendData(0x0F);
	sendData(0x0F);
	sendData(0x0F);
	sendData(0xFF);
	sendData(0xBF);
	sendData(0x1F);
	sendData(0x0F);
	sendData(0x1F);
	sendData(0xFF);
	sendData(0x87);
	sendData(0x05);
	sendData(0x0D);
	sendData(0x01);
	sendData(0x01);
	sendData(0x00);
}

// Third frame of the ducking animation
void duckingThree() {
	rexMode = 27;
	position(8,5);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x80);
	sendData(0x80);
	sendData(0x80);
	sendData(0x80);
	sendData(0x00);
	sendData(0x00);
	sendData(0x80);
	sendData(0xC0);
	sendData(0x40);
	sendData(0xC0);
	sendData(0xC0);
	sendData(0x80);
	
	position(8,6);
	sendData(0x1E);
	sendData(0x1E);
	sendData(0x0F);
	sendData(0x0F);
	sendData(0xFF);
	sendData(0xBF);
	sendData(0x1F);
	sendData(0x0F);
	sendData(0x1F);
	sendData(0xFF);
	sendData(0x87);
	sendData(0x1F);
	sendData(0x17);
	sendData(0x07);
	sendData(0x05);
	sendData(0x05);
	sendData(0x01);
}

// Fourth frame of the ducking animation
void duckingFour() {
	rexMode = 28;
	
	position(8,6);
	sendData(0xF8);
	sendData(0x7C);
	sendData(0x3C);
	sendData(0x1E);
	sendData(0xFF);
	sendData(0xBF);
	sendData(0x1F);
	sendData(0x0F);
	sendData(0x1F);
	sendData(0xFF);
	sendData(0x8E);
	sendData(0x3E);
	sendData(0x2F);
	sendData(0x0F);
	sendData(0x1D);
	sendData(0x17);
	sendData(0x17);
	sendData(0x06);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Jumping and falling animation for the T-Rex
// Continues the scrolling of the screen while in animation
// Sets different mode for each frame it is in
// First Eighth frames are repeated until the T-Rex has gone up 3 pages where it start to fall
// Falling animation is done by calling the jumping animation backwards
void jumpingRex() {
	
	jumpingOne(5,6);
	rexMode = 1;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingTwo(5,6);
	rexMode = 2;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingThree(4,5,6);
	rexMode = 3;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFour(4,5,6);
	rexMode = 4;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFive(4,5,6);
	rexMode = 5;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSix(4,5,6);
	rexMode = 6;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSeven(4,5,6);
	rexMode = 7;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingEight(4,5,6);
	rexMode = 8;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	
	jumpingOne(4,5);
	rexMode = 9;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingTwo(4,5);
	rexMode = 10;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingThree(3,4,5);
	rexMode = 11;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFour(3,4,5);
	rexMode = 12;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFive(3,4,5);
	rexMode = 13;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSix(3,4,5);
	rexMode = 14;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSeven(3,4,5);
	rexMode = 15;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingEight(3,4,5);
	rexMode = 16;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	
	jumpingOne(3,4);
	rexMode = 17;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingTwo(3,4);
	rexMode = 18;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingThree(2,3,4);
	rexMode = 19;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFour(2,3,4);
	rexMode = 20;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFive(2,3,4);
	rexMode = 21;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSix(2,3,4);
	rexMode = 22;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSeven(2,3,4);
	rexMode = 23;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingEight(2,3,4);
	rexMode = 24;
	//Falling
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSeven(2,3,4);
	rexMode = 23;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSix(2,3,4);
	rexMode = 22;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFive(2,3,4);
	rexMode = 21;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFour(2,3,4);
	rexMode = 20;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingThree(2,3,4);
	rexMode = 19;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	fallingClear(2);
	jumpingTwo(3,4);
	rexMode = 18;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingOne(3,4);
	rexMode = 17;
	
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingEight(3,4,5);
	rexMode = 16;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSeven(3,4,5);
	rexMode = 15;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSix(3,4,5);
	rexMode = 14;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFive(3,4,5);
	rexMode = 13;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFour(3,4,5);
	rexMode = 12;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingThree(3,4,5);
	rexMode = 11;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	fallingClear(3);
	jumpingTwo(4,5);
	rexMode = 10;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingOne(4,5);
	rexMode = 9;
	
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingEight(4,5,6);
	rexMode = 8;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSeven(4,5,6);
	rexMode = 7;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingSix(4,5,6);
	rexMode = 6;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFive(4,5,6);
	rexMode = 5;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingFour(4,5,6);
	rexMode = 4;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingThree(4,5,6);
	rexMode = 3;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	fallingClear(4);
	jumpingTwo(5,6);
	rexMode = 2;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	jumpingOne(5,6);
	rexMode = 1;
	_delay_5ms();
	scrollLeft();
	preventScrollBack();
	
	drawRex();
	rexMode = 0;
}

// First frame of the jumping animation
void jumpingOne(unsigned char top, unsigned char bottom) {
	position(8,top);
	//position(8,5);
	for (int j = 0; j < 15; j++) {
		if (j < 13) {
			sendData((pgm_read_byte(&Rex[0][j]) >> 1) | 0x80);
		}
		else {
			sendData((pgm_read_byte(&Rex[0][j]) >> 1));
		}
	}
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 1);
	}
}

// Second frame of the jumping animation
void jumpingTwo(unsigned char top, unsigned char bottom) {
	position(8,top);
	//position(8,5);
	for (int j = 0; j < 15; j++) {
		if ((j < 11) || (j == 12)) {
			sendData((pgm_read_byte(&Rex[0][j]) >> 2) | 0xC0);
		}
		else if (j == 11) {
			sendData((pgm_read_byte(&Rex[0][j]) >> 2) | 0x40);
		}
		else {
			sendData((pgm_read_byte(&Rex[0][j]) >> 2));
		}
	}
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 2);
	}
}

// Third frame of the jumping animation
void jumpingThree(unsigned char top, unsigned char middle, unsigned char bottom) {
	position(8,top);
	for (int i = 0; i < 10; i++) {
		sendData(0x00);
	} 
	//position(18,top);
	//position(18,4);
	for (int i = 0; i < 4; i++) {
		sendData(0x80);
	}
	sendData(0x00);
	
	position(8,middle);
	//position(8,5);
	sendData(0x7C);
	sendData(0xF8);
	sendData(0xF0);
	sendData(0xE0);
	sendData(0xE0);
	sendData(0xF0);
	sendData(0xF8);
	sendData(0xF8);
	sendData(0xFC);
	sendData(0xFF);
	sendData(0xFF);
	sendData(0x2E);
	sendData(0x6B);
	sendData(0x0B);
	sendData(0x03);
	
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 3);
	}
}

// Fourth frame of the jumping animation
void jumpingFour(unsigned char top, unsigned char middle, unsigned char bottom) {
	position(8,top);
	for (int i = 0; i < 9; i++) {
		sendData(0x00);
	}
	//position(17,top);
	//position(17,4);
	sendData(0x80);
	sendData(0xC0);
	sendData(0x40);
	sendData(0xC0);
	sendData(0xC0);
	sendData(0x80);
	
	position(8,middle);
	//position(8,5);
	sendData(0x3E);
	sendData(0x7C);
	sendData(0x78);
	sendData(0xF0);
	sendData(0xF0);
	sendData(0xF8);
	sendData(0xFC);
	sendData(0xFC);
	sendData(0xFE);
	sendData(0xFF);
	sendData(0x7F);
	sendData(0x17);
	sendData(0x35);
	sendData(0x05);
	sendData(0x01);
	
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 4);
	}
}

// Fifth frame of the jumping animation
void jumpingFive(unsigned char top, unsigned char middle, unsigned char bottom) {
	position(8,top);
	for (int i = 0; i < 9; i++) {
		sendData(0x00);
	}
	//position(17,top);
	//position(17,4);
	sendData(0xC0);
	sendData(0xE0);
	sendData(0xA0);
	sendData(0xE0);
	sendData(0xE0);
	sendData(0xC0);
	
	position(8,middle);
	//position(8,5);
	sendData(0x1F);
	sendData(0x3E);
	sendData(0x3C);
	sendData(0x78);
	sendData(0xF8);
	sendData(0xFC);
	sendData(0xFE);
	sendData(0x7E);
	sendData(0xFF);
	sendData(0xFF);
	sendData(0x3F);
	sendData(0x0B);
	sendData(0x1A);
	sendData(0x02);
	sendData(0x00);
	
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 5);
	}
}

// Sixth frame of the jumping animation
void jumpingSix(unsigned char top, unsigned char middle, unsigned char bottom) {
	position(8,top);
	//position(8,4);
	sendData(0x80);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	//position(16,top);
	//position(16,4);
	sendData(0x80);
	sendData(0xE0);
	sendData(0xF0);
	sendData(0xD0);
	sendData(0x70);
	sendData(0x70);
	sendData(0x60);
	
	position(8,middle);
	//position(8,5);
	sendData(0x0F);
	sendData(0x1F);
	sendData(0x1E);
	sendData(0x3C);
	sendData(0xFC);
	sendData(0xFE);
	sendData(0x7F);
	sendData(0x3F);
	sendData(0x7F);
	sendData(0xFF);
	sendData(0x1F);
	sendData(0x05);
	sendData(0x0D);
	sendData(0x01);
	sendData(0x00);
	
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 6);
	}
}

// Seventh frame of the jumping animation
void jumpingSeven(unsigned char top, unsigned char middle, unsigned char bottom) {
	position(8,top);
	//position(8,4);
	sendData(0xC0);
	sendData(0x80);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	//position(14,top);
	//position(14,4);
	sendData(0x80);
	sendData(0x80);
	sendData(0xC0);
	sendData(0xF0);
	sendData(0xF8);
	sendData(0xE8);
	sendData(0xB8);
	sendData(0xB8);
	sendData(0x30);
	
	position(8,middle);
	//position(8,5);
	sendData(0x07);
	sendData(0x0F);
	sendData(0x0F);
	sendData(0x1E);
	sendData(0xFE);
	sendData(0x7F);
	sendData(0x3F);
	sendData(0x1F);
	sendData(0x3F);
	sendData(0xFF);
	sendData(0x0F);
	sendData(0x02);
	sendData(0x06);
	sendData(0x00);
	sendData(0x00);
	
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 7);
	}
}

// Eighth frame of the jumping animation
void jumpingEight(unsigned char top, unsigned char middle, unsigned char bottom) {
	position(8,top);
	//position(8,4);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[0][j]));
	}
	position(8,middle);
	//position(8,5);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]));
	}
	
	
	position(8,bottom);
	//position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]) >> 8);
	}
	
}

// Clears the top of the head of the T-Rex when it is falling
void fallingClear(unsigned char page) {
	position(18,page);
	for (int i = 0; i < 4; i++) {
		sendData(0x00);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Shifts the screen one pixel to the left
void scrollLeft() {
	// Turns the scroll on the OLED on
	sendOneCommandByte(0x2E);
	sendOneCommandByte(0x27);
	sendOneCommandByte(0x00);
	sendOneCommandByte(0x05);
	sendOneCommandByte(0x00); // Change scroll rate 0x04, 0x07
	sendOneCommandByte(0x07);
	sendOneCommandByte(0x00);
	sendOneCommandByte(0xFF);
	sendOneCommandByte(0x2F);
	
	_delay_ms(30);
	sendOneCommandByte(0x2E); // Turns the scroll off
	
	resetCactus(); // Checks if the cactus has been cleared
	resetPterodactyl(); // Checks if the pterodactyl has been cleared
	activeCounter(); // Increments any active objects
	generateRandomEnemy(); // Generates a random enemy every 128 shifts
}

// Clears the first eight columns of the 5th and 6th page
void preventScrollBack() {
	position(0,5);
	for (int i = 0; i < 8; i++) {
		sendData(0x00);
	}
	position(0,6);
	for (int i = 0; i < 8; i++) {
		sendData(0x00);
	}
}

// Displays a pterodactyl on the screen
void drawPterodactyl() {
	// Sends all the bytes required for a pterodactyl
	position(115,5);
	for (int j = 0; j < 11; j++) {
		sendData(pgm_read_byte(&Pterodactyl[j]));
	}
}

// Displays a cactus on the screen
void drawCactus() {
	// Sends all the bytes required for a cactus
	position(121,5);
	for (int j = 0; j < 6; j++) {
		sendData(pgm_read_byte(&Cactus[0][j]));
	}
	position(121,6);
	for (int j = 0; j < 6; j++) {
		sendData(pgm_read_byte(&Cactus[1][j]));
	}
}

// Displays a T-Rex on the screen
void drawRex() {
	// Sends all the bytes required for a T-Rex
	position(8,5);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[0][j]));
	}
	position(8,6);
	for (int j = 0; j < 15; j++) {
		sendData(pgm_read_byte(&Rex[1][j]));
	}
}

// Displays the background / floor on the screen
void background() {
	position(0,7);
	// Pattern for background repeated 16 times to fill the entirety of the 7th page
	for (int i = 0; i < 16; i++) {
		sendData(0xFE);
		sendData(0xFD);
		sendData(0xF7);
		sendData(0xBF);
		sendData(0xEF);
		sendData(0xFB);
		sendData(0x7F);
		sendData(0xDF);
	}	
}

// Displays the start message asking user to press down on the joystick
void gameStart() {
	letterDisplay(0, 0); // P
	letterDisplay(7, 1); // U
	letterDisplay(14, 2); // S
	letterDisplay(21, 3); // H
	
	letterDisplay(31, 2); // S
	letterDisplay(38, 4); // T
	letterDisplay(46, 6); // I
	letterDisplay(54, 7); // C
	letterDisplay(61, 8); // K
	
	letterDisplay(71, 4); // T
	letterDisplay(79, 5); // O
	
	letterDisplay(89, 2); // S
	letterDisplay(96, 4); // T
	letterDisplay(104, 9); // A
	letterDisplay(111, 10); // R
	letterDisplay(118, 4); // T
}

// Displays the end message asking user to press down on touch sensor to reset the screen
void gameEnd() {
	letterDisplay(0, 0); // P
	letterDisplay(7, 1); // U
	letterDisplay(14, 2); // S
	letterDisplay(21, 3); // H
	
	letterDisplay(30, 2); // S
	letterDisplay(37, 11); // E
	letterDisplay(44, 12); // N
	letterDisplay(51, 2); // S
	letterDisplay(58, 5); // O
	letterDisplay(65, 10); // R
	
	letterDisplay(74, 4); // T
	letterDisplay(82, 5); // O
	
	letterDisplay(92, 10); // R
	letterDisplay(99, 11); // E
	letterDisplay(106, 2); // S
	letterDisplay(113, 11); // E
	letterDisplay(120, 4); // T
	
}

// Displays the final score the middle right of the screen when the game has ended
void displayFinalScore() {
	position(64,3);
	// S
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[0][i]));
	}
	position(71,3);
	// C
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[1][i]));
	}
	position(78,3);
	// O
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[2][i]));
	}
	position(85,3);
	// R
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[3][i]));
	}
	position(92,3);
	// E
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[4][i]));
	}
	
	position(99,3);
	sendData(0x24); //colon
	
	int thousands = (int)score / 1000;
	int temp = (int)score % 1000;
	int hundreds = temp / 100;
	temp = temp % 100;
	int tens = temp / 10;
	temp = temp % 10;
	int ones = temp;
	
	position(102,3);
	// Thousands number of the score
	for (int i = 0; i < 4; i++) {
		sendData(pgm_read_byte(&numbers[thousands][i]));
	}
	
	position(108,3);
	// Hundreds number of the score
	for (int i = 0; i < 4; i++) {
		sendData(pgm_read_byte(&numbers[hundreds][i]));
	}
	
	position(114,3);
	// Tens number of the score
	for (int i = 0; i < 4; i++) {
		sendData(pgm_read_byte(&numbers[tens][i]));
	}
	
	position(120,3);
	// Ones number of the score
	for (int i = 0; i < 4; i++) {
		sendData(pgm_read_byte(&numbers[ones][i]));
	}
	
}

// Displays the current score to the screen
void displayScore() {
	// Grabs the current score from the score count
	// Extracts the Thousands, Hundreds, Tens, and Ones place
	int thousands = (int)score / 1000;
	int temp = (int)score % 1000;
	int hundreds = temp / 100;
	temp = temp % 100;
	int tens = temp / 10;
	temp = temp % 10;
	int ones = temp;
	
	// Displays any numbers that have changed to the screen
	if (lastThousands != thousands) {
		displayNumber(thousands, 38);
		lastThousands = thousands;
	}
	if (lastHundreds != hundreds) {
		displayNumber(hundreds, 44);
		lastHundreds = hundreds;
	}
	if (lastTens != tens) {
		displayNumber(tens, 50);
		lastTens = tens;
	}
	if (lastOnes != ones) {
		displayNumber(ones, 56);
		lastOnes = ones;
	}
	
}

// Displays a number to the screen at a certain position
void displayNumber(int number, int x) {
	position(x,0);
	for (int i = 0; i < 4; i++) {
		sendData(pgm_read_byte(&numbers[number][i]));
	}
}

// Displays the score text to the screen
void drawScore() {
	position(0,0);
	// S
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[0][i]));
	}
	position(7,0);
	// R
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[1][i]));
	}
	position(14,0);
	// O
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[2][i]));
	}
	position(21,0);
	// R
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[3][i]));
	}
	position(28,0);
	// E
	for (int i = 0; i < 5; i++) {
		sendData(pgm_read_byte(&scoreLetters[4][i]));
	}
	
	position(35,0);
	sendData(0x24); //colon
}

// Displays a two page letter to the screen a certain location on the first two pages 
void letterDisplay(uint8_t x, uint8_t index) {
	position(x,0);
	for (int i = 0; i < 7; i++) {
		sendData(pgm_read_byte(&Letters[index][i]));
	}
	position(x,1);
	for (int i = 7; i < 14; i++) {
		sendData(pgm_read_byte(&Letters[index][i]));
	}
}

// Sends one command byte to the screen
void sendOneCommandByte(unsigned char cmd) {
	i2c_start((unsigned char)0x78 + I2C_WRITE);
	i2c_write(0x00);
	i2c_write(cmd);
	i2c_stop(); 
}

// Sends two command bytes to the screen
void sendTwoCommandByte(unsigned char cmdOne, unsigned char cmdTwo) {
	i2c_start((unsigned char)0x78 + I2C_WRITE);
	i2c_write(0x00);
	i2c_write(cmdOne);
	i2c_write(cmdTwo);
	i2c_stop();
}

// Sends one data byte to the screen
// Logic 1 turns a pixel on the display on
void sendData(unsigned char data) {
	i2c_start((unsigned char)0x78 + I2C_WRITE);
	i2c_write(0x40);
	i2c_write(data);
	i2c_stop();
}

// Sets the position of the cursor on the display
void position(unsigned char x, unsigned char y) {
	sendOneCommandByte(0x00 + (x & 0x0F));
	sendOneCommandByte(0x10 + ((x >> 4) & 0x0F));
	sendOneCommandByte(0xB0 + y);
}

// Clears the top two pages of the display
void clearTopTwoPages() {
	position(0,0);
	for (int j = 0; j < 128; j++) {
		sendData(0x00);
	}
	position(0,1);
	for (int j = 0; j < 128; j++) {
		sendData(0x00);
	}
}

// Clears the entire display
void clearDisplay() {
	position(0,0);
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 128; j++) {
			sendData(0x00);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Initializes the ADC
void ADCint(void) {
	// ADC Clock frequency: >200.000 kHz.
	// ADC Voltage Reference: AVCC pin.
	// ADC Auto Trigger Source: Free Running.
	#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR));
	ADMUX = ADC_VREF_TYPE;
	ADCSRA = (1<<ADEN)|(0<<ADSC)|(1<<ADATE)|(0<<ADIF);
	ADCSRA = ADCSRA |(0<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //scaling factor 128 so freq = 50-200kHz
	ADCSRB=(0<<ADTS2)|(0<<ADTS1)|(0<<ADTS0);
}

// Creates a delay of 10 microseconds
void _delay_10us(void) {
	_delay_loop_2(40);
}

// Creates a delay of 1 second
void _delay_1s(void) {
	unsigned char counter = 0;
	while (counter != 80) {
		_delay_loop_2(50000);
		counter++;
	}
}

// Reads the ADC output
unsigned int read_adc(unsigned char adc_input) {
	ADMUX = adc_input|ADC_VREF_TYPE;
	_delay_10us(); // For stabilization of ADC input voltage.
	ADCSRA|=(1<<ADSC); // Start the AD conversion.
	while((ADCSRA&(1<<ADIF))==0) // Wait for AD conversion to complete.
	;
	ADCSRA|=(1<<ADIF);
	return ADCW;
}

// Turns the LED on
void LEDOn(void) {
	PORTD |= 0x10;
}

// Turns LED off
void LEDOff(void) {
	PORTD &= ~(0x10);
}

// Toggles the buzzer for jump animation
void buzzerToggle() {
	PORTD |= 0x80;
	_delay_10ms();
	PORTD &= ~(0x80);
}

// Turns the buzzer on
void buzzerOn() {
	PORTD |= 0x80;
}

// Turns the buzzer off
void buzzerOff() {
	PORTD &= ~(0x80);
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// IC2 Related Are Below ////////////////////////////

//#include <inttypes.h>
//#include <compat/twi.h>
//#include <i2cmaster.h>

// define CPU frequency in Here here if not defined in Makefile  or above 
/*
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
*/

/* I2C clock in Hz */
#define SCL_CLOCK  100000L


/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void)
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  
  TWSR = 0;                         /* no prescaler */
  TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */

}/* i2c_init */


/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address)
{
    uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

}/* i2c_start */


/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address)
{
    uint8_t   twst;


    while ( 1 )
    {
	    // send START condition
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    
    	// wait until transmission completed
    	while(!(TWCR & (1<<TWINT)));
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    
    	// send device address
    	TWDR = address;
    	TWCR = (1<<TWINT) | (1<<TWEN);
    
    	// wail until transmission completed
    	while(!(TWCR & (1<<TWINT)));
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
    	{    	    
    	    /* device busy, send stop condition to terminate write operation */
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	        
	        // wait until stop condition is executed and bus released
	        while(TWCR & (1<<TWSTO));
	        
    	    continue;
    	}
    	//if( twst != TW_MT_SLA_ACK) return 1;
    	break;
     }

}/* i2c_start_wait */


/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction 

 Input:   address and transfer direction of I2C device
 
 Return:  0 device accessible
          1 failed to access device
*************************************************************************/
unsigned char i2c_rep_start(unsigned char address)
{
    return i2c_start( address );

}/* i2c_rep_start */


/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void)
{
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

}/* i2c_stop */


/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed
*************************************************************************/
unsigned char i2c_write( unsigned char data )
{	
    uint8_t   twst;
    
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}/* i2c_write */


/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    

    return TWDR;

}/* i2c_readAck */


/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
    return TWDR;

}/* i2c_readNak */







