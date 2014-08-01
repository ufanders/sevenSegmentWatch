/********************************************************************
CHANGELOG

AUTHOR			DATE		COMMENTS
ANDERS NELSON	2009.11.17	FILE CREATED
ANDERS NELSON	2009.12.01	ACCOUNTED FOR RECENT SCHEMATIC CHANGES
********************************************************************/
#define SEG_PORT				LATC //DISPLAY PORT
#define SEG_TRIS				TRISC //DISPLAY PORT TRIS

#define COMMON_PORT				LATB //COMMON (ANODE or CATHODE) PORT
#define COMMON_TRIS				TRISB //COMMON (ANODE or CATHODE) PORT TRIS

#define SEGA_TRIS				TRISCbits.TRISC0 //DISPLAY SEGMENT A
#define SEGB_TRIS				TRISCbits.TRISC1 //DISPLAY SEGMENT B
#define SEGC_TRIS				TRISCbits.TRISC2 //DISPLAY SEGMENT C
#define SEGD_TRIS				TRISCbits.TRISC3 //DISPLAY SEGMENT D
#define SEGE_TRIS				TRISCbits.TRISC4 //DISPLAY SEGMENT E
#define SEGF_TRIS				TRISCbits.TRISC5 //DISPLAY SEGMENT F
#define SEGG_TRIS				TRISCbits.TRISC6 //DISPLAY SEGMENT G
#define SEGDP_TRIS				TRISCbits.TRISC7 //DISPLAY SEGMENT DP

#define SEGA					LATCbits.LATC0 //DISPLAY SEGMENT A
#define SEGB					LATCbits.LATC1 //DISPLAY SEGMENT B
#define SEGC					LATCbits.LATC2 //DISPLAY SEGMENT C
#define SEGD					LATCbits.LATC3 //DISPLAY SEGMENT D
#define SEGE					LATCbits.LATC4 //DISPLAY SEGMENT E
#define SEGF					LATCbits.LATC5 //DISPLAY SEGMENT F
#define SEGG					LATCbits.LATC6 //DISPLAY SEGMENT G
#define SEGDP					LATCbits.LATC7 //DISPLAY SEGMENT DP

#define COMMON1_TRIS					TRISBbits.TRISB0 //DISPLAY COMMON 1
#define COMMON2_TRIS					TRISBbits.TRISB1 //DISPLAY COMMON 2
#define COMMON3_TRIS					TRISBbits.TRISB2 //DISPLAY COMMON 3
#define COMMON4_TRIS					TRISBbits.TRISB3 //DISPLAY COMMON 4
#define COMMON5_TRIS					TRISBbits.TRISB8 //DISPLAY COMMON 5
#define COMMON6_TRIS					TRISBbits.TRISB9 //DISPLAY COMMON 6
#define COMMON7_TRIS					TRISBbits.TRISB10 //DISPLAY COMMON 7
#define COMMON8_TRIS					TRISBbits.TRISB11 //DISPLAY COMMON 8
#define COMMON9_TRIS					TRISBbits.TRISB12 //DISPLAY COMMON 9
#define COMMON10_TRIS					TRISBbits.TRISB13 //DISPLAY COMMON 10
#define COMMON11_TRIS					TRISBbits.TRISB14 //DISPLAY COMMON 11
#define COMMON12_TRIS					TRISBbits.TRISB15 //DISPLAY COMMON 12
#define COMMON13_TRIS					TRISAbits.TRISA0 //DISPLAY COMMON 13
#define COMMON14_TRIS					TRISAbits.TRISA1 //DISPLAY COMMON 14

#define COMMON1						LATBbits.LATB0 //DISPLAY COMMON 1
#define COMMON2						LATBbits.LATB1 //DISPLAY COMMON 2
#define COMMON3						LATBbits.LATB2 //DISPLAY COMMON 3
#define COMMON4						LATBbits.LATB3 //DISPLAY COMMON 4
#define COMMON5						LATBbits.LATB8 //DISPLAY COMMON 5
#define COMMON6						LATBbits.LATB9 //DISPLAY COMMON 6
#define COMMON7						LATBbits.LATB10 //DISPLAY COMMON 7
#define COMMON8						LATBbits.LATB11 //DISPLAY COMMON 8
#define COMMON9						LATBbits.LATB12 //DISPLAY COMMON 9
#define COMMON10					LATBbits.LATB13 //DISPLAY COMMON 10
#define COMMON11					LATBbits.LATB14 //DISPLAY COMMON 11
#define COMMON12					LATBbits.LATB15 //DISPLAY COMMON 12
#define COMMON13					LATAbits.LATA0 //DISPLAY COMMON 13
#define COMMON14					LATAbits.LATA1 //DISPLAY COMMON 14

/***************************************************************************
The display refresh routine for 14 digits takes ~105 instructions. That
said, we need (105 instructions)*(14 digits)*(60 refreshes per second)*
(2 clock cycles per instruction) = 176,400Hz oscillator. Let's use
the CLKDIV register to drive the CPU at 250kHz for minimal power draw.

***************************************************************************/

#define FOSC 8000000 //250000
#define NUM_DIGITS 14
#define INST_FREQ (FOSC/2)
#define REFRESHRATE (60*NUM_DIGITS) //REFRESH RATE IN FRAMES PER SECOND (60FPS for each digit)
#define TIMER1PRESCALE 1
#define TIMER1PRELOADVALUE (0xFFFF - (INST_FREQ/TIMER1PRESCALE/REFRESHRATE))

#define SEG_ON 1
#define SEG_OFF 0
#define SEG_PORT_ON (0xFF * SEG_ON)
#define SEG_PORT_OFF (0xFF * SEG_OFF)

#define COMMON_ON ~SEG_ON
#define COMMON_OFF ~SEG_OFF
#define COMMON_PORT_ON (0xFF * COMMON_ON)
#define COMMON_PORT_OFF (0xFF * COMMON_OFF)

extern volatile unsigned char currentDigit, digits[NUM_DIGITS];

//FUNCTION PROTOTYPES
void init7Segment(void); 		//Initialization Routine
void clear7Segment(void);
void draw7Segment(void);
void displayNumber(unsigned char, unsigned char);
unsigned char getSegments(unsigned char);
void displayDp(unsigned char, signed char);
void displayDigDec(unsigned long);//, unsigned char digits);
void setSegments(unsigned char, unsigned char);
