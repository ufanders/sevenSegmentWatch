/***********************************************************************
CHANGELOG
AUTHOR			DATE		COMMENTS
ANDERS NELSON	2009.09.30	FILE CREATED, ADDED DISPLAY2DIGHEX
ANDERS NELSON	2009.10.01	CORRECTED DISPLAY2DIGDEC()
ANDERS NELSON	2009.10.02	ADDED DISPLAYDP()
ANDERS NELSON	2009.10.07	ADDED PROPER DIGIT INIT
ANDERS NELSON	2009.10.17	
ANDERS NELSON	2009.12.09	UPDATED TO CONFORM TO 16BIT MACHINE,
							USING DIGIT ARRAY
***********************************************************************/
#include <p24Fxxxx.h>
#include "7segment.h"

// variable declarations
volatile unsigned char currentDigit, digits[NUM_DIGITS];

void init7Segment()
{
	unsigned char iTemp;
	
	clear7Segment();
	
	//INIT GPIO
	//SEG_PORT = SEG_PORT_OFF; //ALL SEGMENTS OFF
	//SEG_TRIS = 0x00; //ALL SEGMENT PINS AS OUTPUT
	//COMMON_PORT = COMMON_PORT_OFF;
	//COMMON_TRIS = 0x00; //COMMON PINS AS OUTPUT
	
	AD1PCFG = 0x0000; //ALL ADC REFS OFF, PINS AS DIGITAL
	
	//ALL SEGMENTS OFF
	SEGA = SEG_OFF;
	SEGB = SEG_OFF;
	SEGC = SEG_OFF;
	SEGD = SEG_OFF;
	SEGE = SEG_OFF;
	SEGF = SEG_OFF;
	SEGG = SEG_OFF;
	SEGDP = SEG_OFF;
	
	//ALL SEGMENT PINS AS OUTPUT
	SEGA_TRIS = 0;
	SEGB_TRIS = 0;
	SEGC_TRIS = 0;
	SEGD_TRIS = 0;
	SEGE_TRIS = 0;
	SEGF_TRIS = 0;
	SEGG_TRIS = 0;
	SEGDP_TRIS = 0;
	
	//ALL COMMONS OFF
	COMMON1 = COMMON_OFF;
	COMMON2 = COMMON_OFF;
	COMMON3 = COMMON_OFF;
	COMMON4 = COMMON_OFF;
	COMMON5 = COMMON_OFF;
	COMMON6 = COMMON_OFF;
	COMMON7 = COMMON_OFF;
	COMMON8 = COMMON_OFF;
	COMMON9 = COMMON_OFF;
	COMMON10 = COMMON_OFF;
	COMMON11 = COMMON_OFF;
	COMMON12 = COMMON_OFF;
	COMMON13 = COMMON_OFF;
	COMMON14 = COMMON_OFF;
	
	//COMMON PINS AS OUTPUT
	COMMON1_TRIS = 0;
	COMMON2_TRIS = 0;
	COMMON3_TRIS = 0;
	COMMON4_TRIS = 0;
	COMMON5_TRIS = 0;
	COMMON6_TRIS = 0;
	COMMON7_TRIS = 0;
	COMMON8_TRIS = 0;
	COMMON9_TRIS = 0;
	COMMON10_TRIS = 0;
	COMMON11_TRIS = 0;
	COMMON12_TRIS = 0;
	COMMON13_TRIS = 0;
	COMMON14_TRIS = 0;
	
	//INIT DIGITS
	for(iTemp = 0; iTemp<NUM_DIGITS; iTemp++)
	{
		digits[iTemp] = 0;
	}	
	currentDigit = 0;
}

void draw7Segment(void)
{
	unsigned char tempSegments;

	COMMON1 = COMMON_OFF; //TURN OFF ANODE 1
	COMMON2 = COMMON_OFF; //TURN OFF ANODE 2
	COMMON3 = COMMON_OFF; //TURN OFF ANODE 3
	COMMON4 = COMMON_OFF; //TURN OFF ANODE 4
	COMMON5 = COMMON_OFF; //TURN OFF ANODE 5
	COMMON6 = COMMON_OFF; //TURN OFF ANODE 6
	COMMON7 = COMMON_OFF; //TURN OFF ANODE 7
	COMMON8 = COMMON_OFF; //TURN OFF ANODE 8
	COMMON9 = COMMON_OFF; //TURN OFF ANODE 9
	COMMON10 = COMMON_OFF; //TURN OFF ANODE 10
	COMMON11 = COMMON_OFF; //TURN OFF ANODE 11
	COMMON12 = COMMON_OFF; //TURN OFF ANODE 12
	COMMON13 = COMMON_OFF; //TURN OFF ANODE 13
	COMMON14 = COMMON_OFF; //TURN OFF ANODE 14
	
	//COMMON_PORT = COMMON_PORT_OFF;
		
	tempSegments = digits[currentDigit];
	
	if((tempSegments & 0b10000000) > 0)
	{
		SEGDP = SEG_ON;
	}
	else SEGDP = SEG_OFF;
	
	if((tempSegments & 0b01000000) > 0)
	{
		SEGG = SEG_ON;
	}
	else SEGG = SEG_OFF;
	
	if((tempSegments & 0b00100000) > 0)
	{
		SEGF = SEG_ON;
	}
	else SEGF = SEG_OFF;
	
	if((tempSegments & 0b00010000) > 0)
	{
		SEGE = SEG_ON;
	}
	else SEGE = SEG_OFF;
	
	if((tempSegments & 0b00001000) > 0)
	{
		SEGD = SEG_ON;
	}
	else SEGD = SEG_OFF;
	
	if((tempSegments & 0b00000100) > 0)
	{
		SEGC = SEG_ON;
	}
	else SEGC = SEG_OFF;
	
	if((tempSegments & 0b00000010) > 0)
	{
		SEGB = SEG_ON;
	}
	else SEGB = SEG_OFF;
	
	if((tempSegments & 0b00000001) > 0)
	{
		SEGA = SEG_ON;
	}
	else SEGA = SEG_OFF;
	
	
	switch(currentDigit)
	{	
		case 0:
		COMMON1 = COMMON_ON; //TURN ON ANODE 1
		break;
		
		case 1:
		COMMON2 = COMMON_ON; //TURN ON ANODE 2
		break;
		
		case 2:
		COMMON3 = COMMON_ON; //TURN ON ANODE 3
		break;
		
		case 3:
		COMMON4 = COMMON_ON; //TURN ON ANODE 4
		break;
		
		case 4:
		COMMON5 = COMMON_ON; //TURN ON ANODE 5
		break;
		
		case 5:
		COMMON6 = COMMON_ON; //TURN ON ANODE 6
		break;
		
		case 6:
		COMMON7 = COMMON_ON; //TURN ON ANODE 7
		break;
		
		case 7:
		COMMON8 = COMMON_ON; //TURN ON ANODE 8
		break;
		
		case 8:
		COMMON9 = COMMON_ON; //TURN ON ANODE 9
		break;
		
		case 9:
		COMMON10 = COMMON_ON; //TURN ON ANODE 10
		break;
		
		case 10:
		COMMON11 = COMMON_ON; //TURN ON ANODE 11
		break;
		
		case 11:
		COMMON12 = COMMON_ON; //TURN ON ANODE 12
		break;
		
		case 12:
		COMMON13 = COMMON_ON; //TURN ON ANODE 13
		break;
		
		case 13:
		COMMON14 = COMMON_ON; //TURN ON ANODE 14
		break;
	}
	
	currentDigit++;
	if(currentDigit > (NUM_DIGITS-1))
	{
		currentDigit = 0;
	}	
	
	//COMMON_PORT = (COMMON_ON << (currentDigit-1));
}	

void clear7Segment()
{
	unsigned char iTemp;
	
	COMMON1 = COMMON_OFF; //TURN OFF ANODE 1
	COMMON2 = COMMON_OFF; //TURN OFF ANODE 2
	COMMON3 = COMMON_OFF; //TURN OFF ANODE 3
	COMMON4 = COMMON_OFF; //TURN OFF ANODE 4
	COMMON5 = COMMON_OFF; //TURN OFF ANODE 5
	COMMON6 = COMMON_OFF; //TURN OFF ANODE 6
	COMMON7 = COMMON_OFF; //TURN OFF ANODE 7
	COMMON8 = COMMON_OFF; //TURN OFF ANODE 8
	COMMON9 = COMMON_OFF; //TURN OFF ANODE 9
	COMMON10 = COMMON_OFF; //TURN OFF ANODE 10
	COMMON11 = COMMON_OFF; //TURN OFF ANODE 11
	COMMON12 = COMMON_OFF; //TURN OFF ANODE 12
	COMMON13 = COMMON_OFF; //TURN OFF ANODE 13
	COMMON14 = COMMON_OFF; //TURN OFF ANODE 14
	
	for(iTemp = 0; iTemp<NUM_DIGITS; iTemp++)
	{
		digits[iTemp] = 0;
	}	
}

void displayDigDec(unsigned long number)
{
	unsigned long divisor;
	unsigned char tempNumber, iTemp, index;
	
	//if(digits > 14) return;
	
	//no need to truncate, a maxed out 32 bit number in decimal is 4,294,967,295 (ten digits)
	divisor = 1000000000;
	//divisor = pow(10, NUM_DIGITS);
	
	for(iTemp = 0; iTemp<(NUM_DIGITS-1); iTemp++) //for all but the last digit
	{	
		index = iTemp;
		tempNumber = number/divisor;
		digits[index] = getSegments(tempNumber);
		number = number % (tempNumber * divisor);
		divisor /= 10;
	}
	
	digits[(NUM_DIGITS-1)] = getSegments(number); //for the last digit
}

void display2digHex(unsigned int number)
{
	unsigned char tempNumber;
	
	if(number > 0xFF)
	{
		number = 0xFF; //truncate to 0xFF
	}
	
	tempNumber = number / 0x10;
	digits[1] = getSegments(tempNumber);
	number = number - tempNumber * 0x10;
	
	digits[0] = getSegments(number);
}

unsigned char getSegments(unsigned char number)
{	
	unsigned char tempNumber = 0;
	
	switch(number)
	{
		case 0:
			//SEGA = SEGB = SEGC = SEGD = SEGE = SEGF = 1;
			//tempNumber = 0b11000000;
			tempNumber = 0b00111111;
			break;
		case 1:
			//SEGB = SEGC = 1;
			//tempNumber = 0b11111001;
			tempNumber = 0b00000110;
			break;
		case 2:
			//SEGA = SEGB = SEGD = SEGE = SEGG = 1;
			//tempNumber = 0b10100100;
			tempNumber = 0b01011011;
			break;
		case 3:
			//SEGA = SEGB = SEGC = SEGD = SEGG = 1;
			//tempNumber = 0b10110000;
			tempNumber = 0b01001111;
			break;
		case 4:
			//SEGB = SEGC = SEGF = SEGG = 1;
			//tempNumber = 0b10011001;
			tempNumber = 0b01100110;
			break;
		case 5:
			//SEGA = SEGC = SEGD = SEGF = SEGG = 1;
			//tempNumber = 0b10010010;
			tempNumber = 0b01101101;
			break;
		case 6:
			//SEGA = SEGC = SEGD = SEGE = SEGF = SEGG = 1;
			//tempNumber = 0b10000010;
			tempNumber = 0b01111101;
			break;
		case 7:
			//SEGA = SEGB = SEGC = 1;
			//tempNumber = 0b11111000;
			tempNumber = 0b00000111;
			break;
		case 8:
			//SEGA = SEGB = SEGC = SEGD = SEGE = SEGF = SEGG = 1;
			//tempNumber = 0b10000000;
			tempNumber = 0b01111111;
			break;
		case 9:
			//SEGA = SEGB = SEGC = SEGD = SEGF = SEGG = 1;
			//tempNumber = 0b10010000;
			tempNumber = 0b01101111;
			break;
		case 0x0A:
			//SEGA = SEGB = SEGC = SEGE = SEGF = SEGG = 1;
			//tempNumber = 0b10001000;
			tempNumber = 0b01110111;
			break;
		case 0x0B:
			//SEGC = SEGD = SEGE = SEGF = SEGG =  1;
			//tempNumber = 0b10000011;
			tempNumber = 0b01111100;
			break;
		case 0x0C:
			//SEGA = SEGC = SEGD = SEGE = SEGF = 1;
			//tempNumber = 0b11000010;
			tempNumber = 0b00111101;
			break;
		case 0x0D:
			//SEGB = SEGC = SEGD = SEGE = SEGG = 1;
			//tempNumber = 0b10100001;
			tempNumber = 0b01011110;
			break;
		case 0x0E:
			//SEGA = SEGD = SEGE = SEGF = SEGG = 1;
			//tempNumber = 0b10000110;
			tempNumber = 0b01111001;
			break;
		case 0x0F:
			//SEGA = SEGE = SEGF = SEGG = 1;
			//tempNumber = 0b10001110;
			tempNumber = 0b01110001;
			break;
		default:
			break;
	}
	
	return tempNumber;
}

void displayDp(unsigned char dp, signed char digit)
{		
	if(digit < 0) return;
	
	//append decimal point to digit 1 variable
	if((dp & 0b00000001) == 1) digits[digit] |= 0b10000000; //set dp bit (turns on)
	else digits[digit] &= 0b01111111; //clear dp bit (turns off)
}

void setSegments(unsigned char segments, unsigned char digit)
{	
	digits[digit] = segments;	
}
