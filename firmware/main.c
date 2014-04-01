/********************************************************************
CHANGELOG
AUTHOR			DATE		COMMENTS
ANDERS NELSON	2009.11.08	FILE CREATED
ANDERS NELSON	2009.11.10	BEGAN INSERTING PIC24 CALLS
ANDERS NELSON	2009.11.17	BEGAN ADDING RTCC FRAMEWORK CALLS
ANDERS NELSON	2009.12.01	BEGAN ADDING WATCH STATE MACHINERY
ANDERS NELSON	2009.12.09	ADDED SETTIME()
ANDERS NELSON	2009.12.10	ADDED SHOWTIME()
ANDERS NELSON	2009.12.14	BEGAN ADDING POWER-SAVING MEASURES,
                                ENABLED RTCC, ADDED TIME ENTRY DELAY
ANDERS NELSON	2009.12.15	CHANGED REFRESH TIMER TO TIMER2
ANDERS NELSON	2009.12.22	ADDED SETMODE(), DIFFERENT SHOWTIME()
                                DISPLAYS
ANDERS NELSON	2009.12.23	COMPLETED SHOWTIME() SECONDS REFESH
ANDERS NELSON	2009.12.24	FIXED SECONDS REFRESH ISSUE, FIXED
                                INCORRECT OVERLAY OF SECONDS UNTO
                                DATE/TIME DISPLAY, ADDED SWITCH DEBOUNCE
                                , ADDED DEEP SLEEP WAKEUP PROVISIONS
ANDERS NELSON	2009.12.25	FIXED CNIF CLEARING IN MAIN()
********************************************************************/
//INCLUDES
#include <p24Fxxxx.h>
#define FCY 16000000
#include <libpic30.h>
#include <Rtcc.h>
#include "7segment.h"

//CONFIG BITS
// Setup configuration bits
// JTAG/Code Protect/Write Protect
// Watchdog Timer/ICD pins select
_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_OFF & ICS_PGx3)

// Enable CLK switch and disable CLK monitor, OSCO as RC15,
// primary oscillator block off, FRC oscillator, Low Power Secondary Oscillator
_CONFIG2(FCKSM_CSECMD & OSCIOFNC_ON &
POSCMOD_NONE & FNOSC_FRC & SOSCSEL_LPSOSC)

//write protect page 0, low-power SOSC drive
//fast VREG wakeup timer, segment write protection disabled,
//write protect config page disabled, write protect from page 0
_CONFIG3(WPFP_WPFP1 & SOSCSEL_LPSOSC &
WUTSEL_FST & WPDIS_WPDIS &
WPCFG_WPCFGDIS & WPEND_WPSTARTMEM)

//deep sleep WDT prescale at 1:2048, dswdt osc as LPRC,
//RTCC ref as secondary osc, ds BOR off, dswdt off
_CONFIG4(DSWDTPS_DSWDTPS5 & DSWDTOSC_LPRC &
RTCOSC_SOSC & DSBOREN_OFF & DSWDTEN_OFF)

//PROTOTYPES
void init(void);
void setTime(void);
void setOptions(void);
void showTime(void);
unsigned char hex2bcd(unsigned char);
unsigned char bcd2hex(unsigned char);

//DEFINES
#define SW1_TRIS TRISCbits.TRISC8
#define SW2_TRIS TRISCbits.TRISC9
#define SW3_TRIS TRISBbits.TRISB7

#define SW1 PORTCbits.RC8
#define SW2 PORTCbits.RC9
#define SW3 PORTBbits.RB7

#define SW_DEBOUNCE 10

//TODO: enumerate these states
#define state_idle 0
#define state_setTime 1
#define state_showTime 2

#define state_setTime_idle 0
#define state_setTime_year 1
#define state_setTime_month 2
#define state_setTime_day 3
#define state_setTime_hour 4
#define state_setTime_minute 5
#define state_setTime_second 6

#define state_showTime_idle 0
#define state_showTime_splash 1
#define state_showTime_time 2
#define state_showTime_date 3

//GLOBAL VARIABLES
volatile unsigned int currentState, timer1Ticks, timeEntryWaiting, alarmFlag;
unsigned char old_sw1, old_sw2, old_sw3;

struct tagCONF1bits {
  unsigned hr24:1; //12 or 24 hour display (military time)
  unsigned showDateWithTime:1; //show date and time at once
  unsigned showSeconds:1; //show seconds elapsing (only useable with showDateWithTime turned off)
  unsigned rsvd:5; //reserved
} CONF1bits;

rtccTimeDate currentTimeDate;

//INTERRUPTS
void _ISR _CNInterrupt(void) //INTERRUPT-ON-CHANGE ISR (BUTTONS)
{	
	if(SW3 == 0) //SW3 ISR (SHOW TIME BUTTON)
	{
		switch(currentState)
		{
			case state_setTime:
				//currentState = state_showTime;
			break;
			
			case state_showTime:
				//currentState = state_showTime;
			break;
			
			case state_idle:
				currentState = state_showTime;
			break;
			
			default:
			break;
		}	
	}
	
	if((SW2 == 0) && !timeEntryWaiting) //SW2 ISR (ADJUST + BUTTON)
	{
		if(currentState == state_idle)
		{
			timeEntryWaiting = 1;
			
			//start timer counting up to 2 seconds
			TMR1 = 0; //reload timer
			T1CON = 0b1000000000000010; //SOSC, 1:1	, TIMER1 ON
			IEC0bits.T1IE = 1; //TIMER1 interrupt enabled
		}
	}
	
	//only the buttons above are used to wakeup the system from sleep
	/*
	if(PORTAbits.RA0 == 0) //SW3 ISR (ADJUST - BUTTON)
	{
		
	}
	*/		

	//reading the port has cleared the mismatch condition already
	IFS1bits.CNIF = 0; //clear interrupt flag
}

void _ISR _T1Interrupt(void) //TIMER1 ISR (INTERFACE TIMER, SET-TIME ENTRY TIMER)
{	
	if(currentState == state_idle) //when using timer1 as a time-entry countdown
	{
		if(SW2 == 0) currentState = state_setTime; //if timer has expired and SW2 is still held down, enter time-set mode 
		
		timeEntryWaiting = 0; //clear timeEntryWaiting flag
		
		//cancel timer
		T1CON = 0b0000000000000000; //FOSC/2, 1:1, TIMER1 OFF
	}
	else
	{
		TMR1 = 0x8000; //reload timer at one second
		//TMR1 = TIMER2PRELOADVALUE; //reload timer
		
		timer1Ticks++;
	}
	
	IFS0bits.T1IF = 0; //clear interrupt flag
}

void _ISR _T2Interrupt(void) //TIMER2 ISR (DISPLAY MULTIPLEXING)
{	
	//TMR2 = 0xED66; //reload timer
	TMR2 = TIMER1PRELOADVALUE; //reload timer

	draw7Segment();
	
	IFS0bits.T2IF = 0; //clear interrupt flag
}

void _ISR _RTCCInterrupt(void) //RTCC ISR (EVERY ONE SECOND)
{
	alarmFlag = 1;
	IFS3bits.RTCIF = 0; //clear interrupt flag
}

void _ISR _INT0Interrupt(void) //INT0 ISR (FOR WAKEUP FROM DEEP SLEEP)
{
	IFS0bits.INT0IF = 0; //clear interrupt flag
}

int main(void)
{	
	init();
	
	while(1)
	{	
		switch(currentState)
		{
			case state_idle:
			break;
			
			case state_setTime:
			IEC1bits.CNIE = 0;		// disable CN
			setTime();
			setOptions();
			showTime();
			currentState = state_idle;
			break;
			
			case state_showTime:
			IEC1bits.CNIE = 0;		// disable CN
			showTime();
			currentState = state_idle;
			break;
			
			default:
			break;
		}
		
		IEC1bits.CNIE = 1;		// enable CN
		
		/*
		//TODO: go into deep sleep
		//save any critical state info in the two deep sleep registers if necessary.
		//save watch configuration using deep sleep register(s)
		DSGPR0 = CONF1bits.hr24;
		DSGPR0 |= (CONF1bits.showDateWithTime << 1);
		DSGPR0 |= (CONF1bits.showSeconds << 2);
		
		IFS0bits.INT0IF = 0; //clear INT0 interrupt flag
		IEC0bits.INT0IE = 1; //INT0 interrupt enabled
		
		DSCONbits.DSEN = 1; //set DSEN bit
		Nop(); //requisite three nop's for proper deep sleep synchronization using INT0 to wake up
		Nop();
		Nop();
		*/
		
		Sleep();
		Nop();
	}	
}

void init(void)
{
	unsigned char Time_Input_out_of_range, initializationNeeded;
	
	if(RCONbits.DPSLP == 1) //check to see if we have woken up from deep sleep
	{
		//if(DSCONbits.DSBOR == 1)
		
		initializationNeeded = 0;
		
		//restore watch configuration using deep sleep register(s)
		CONF1bits.hr24 = (DSGPR0 & 0b00000001);
		CONF1bits.showDateWithTime = (DSGPR0 & 0b00000010);
		CONF1bits.showSeconds = (DSGPR0 & 0b00000100);
		
		RCONbits.DPSLP = 0; //user must clear the deep sleep status bit
		DSCONbits.RELEASE = 0; //release GPIO to current state (still POR state at this point)
	}
	else initializationNeeded = 1;
	
	//init clock(s)
	OSCCON = 0b0000000000000000; //FRC, FRC, not locked, not locked, pri osc dis in sleep, SOSC enabled
	//CLKDIV = (0b0000010100000000; //250kHz CPU clock
	
	//init gpio
	//pins as digital
	AD1PCFGL = 0x0000;
	//AD1PCFGH = 0x0000;
	
	//inputs
	SW1_TRIS = 1;
	SW2_TRIS = 1;
	SW3_TRIS = 1;
	
	//pullups
	CNPU2bits.CN20PUE = 1;
	CNPU2bits.CN19PUE = 1;
	CNPU2bits.CN23PUE = 1;
	
	//outputs
	
	//init interrupt sources
	//change notification
	CNEN2bits.CN19IE = 1;
	CNEN2bits.CN23IE = 1;
	
	IFS1bits.CNIF = 0;		// clear interrupt flag
	//IPC4bits.CNIP = 7;		// set IP as 7
	//IEC1bits.CNIE = 1;		// enable CN
	
	//timer1
	IFS0bits.T1IF = 0; //clear interrupt flag
	IEC0bits.T1IE = 1; //TIMER1 interrupt enabled
	
	//timer2
	IFS0bits.T2IF = 0; //clear interrupt flag
	IEC0bits.T2IE = 1; //TIMER2 interrupt enabled
	
	//int0
	INTCON2bits.INT0EP = 1; //interrupt on negative edge (pulled up internally)
	IFS0bits.INT0IF = 0; //clear interrupt flag
	//IEC0bits.INT0IE = 1; //INT0 interrupt enabled
	
	old_sw1 = old_sw2 = old_sw3 = 0;
	
	timeEntryWaiting = 0;

	//init LEDs
	init7Segment();
	
	//init timers
	//RTCC
	if(initializationNeeded)
	{
		RtccInitClock();       //turn on clock source
		
		RtccWrOn(); //enable writes to the rtcc
		RCFGCALbits.RTCEN = 1; //RTCC ON
		
		currentTimeDate.f.year = 0x00; //year 2000
		currentTimeDate.f.mday = 0x01; //first of the month
		currentTimeDate.f.mon = 0x01; //January
		currentTimeDate.f.hour = 0x00; //00:xx:xx
		currentTimeDate.f.wday = 0x01; //Monday
		currentTimeDate.f.sec = 0x00; //xx:xx:00
		currentTimeDate.f.min = 0x00; //xx:00:xx
		
		Time_Input_out_of_range = RtccWriteTimeDate(&currentTimeDate, TRUE);
		
		if(Time_Input_out_of_range == 0)
		{
			 //object has values out of range
			 while(1); //panic, we're going to die.
	 	}
	 	
	 	//TODO: establish a one-second alarm
	 	
	 	RtccWrOn(); //enable writes to the rtcc
	 	RtccWriteAlrmTimeDate(&currentTimeDate);
		RtccSetAlarmRpt(RTCC_RPT_SEC, 0);
		RtccSetAlarmRptCount(0xFF, 0);
	 	RtccSetChimeEnable(1, 0); //allow indefinite alarm repeats until alarm is disabled
	 	IFS3bits.RTCIF = 0;		//clear interrupt flag
	 	//IEC3bits.RTCIE = 1;
	 	
	 	CONF1bits.hr24 = 0;
		CONF1bits.showDateWithTime = 1;
		CONF1bits.showSeconds = 0;
		CONF1bits.rsvd = 0;
	 	
 		currentState = state_setTime;
 		
	}else currentState = state_showTime;
}

BOOL sw1IsPressed(void)
{
    if(SW1 != old_sw1)
    {
        __delay_ms(SW_DEBOUNCE); //10ms delay to debounce switch
	    
        old_sw1 = SW1;                  // Save new value
        if(SW1 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end sw1IsPressed

BOOL sw2IsPressed(void)
{
    if(SW2 != old_sw2)
    {
        __delay_ms(SW_DEBOUNCE); //10ms delay to debounce switch
	    
        old_sw2 = SW2;                  // Save new value
        if(SW2 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end sw1IsPressed

BOOL sw3IsPressed(void)
{
    if(SW3 != old_sw3)
    {
        __delay_ms(SW_DEBOUNCE); //10ms delay to debounce switch
	    
        old_sw3 = SW3;                  // Save new value
        if(SW3 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end sw1IsPressed

void setTime(void)
{
	//unsigned char keysInUse = 1;
	unsigned char changesMadeFlag, Time_Input_out_of_range;
	signed int currentFieldValue, tempValue, tempValue2, tempValue3;
	unsigned int currentFieldSelected;
	
	currentFieldValue = currentFieldSelected = 0;
	changesMadeFlag = 1;
	
	//grab rtccTimeDate
	RtccReadTimeDate(&currentTimeDate);
	
	//convert fields to hex
	tempValue = bcd2hex(currentTimeDate.f.year);
	currentTimeDate.f.year = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.mon);
	currentTimeDate.f.mon = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.mday);
	currentTimeDate.f.mday = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.hour);
	currentTimeDate.f.hour = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.min);
	currentTimeDate.f.min = tempValue;
	/*
	tempValue = bcd2hex(currentTimeDate.f.sec);
	currentTimeDate.f.sec = tempValue;
	*/
	
	//turn on display refresh interrupt
	T2CON = 0b1000000000000000; //FOSC/2, 1:1, TIMER2 ON
	IEC0bits.T2IE = 1; //TIMER2 interrupt enabled
	
	TMR1 = 0; //reload timer with 2-second interval
	//TMR1 = TIMER2PRELOADVALUE; //reload timer
	T1CON = 0b1000000000000010; //SOSC, 1:1, TIMER1 ON
	IEC0bits.T1IE = 1; //TIMER1 interrupt enabled
	
	//scan keys
	while(1 /*keysInUse*/)
	{		
		if(timer1Ticks > 10) break; //if enough one-second intervals have passed, exit time-set mode and discard changes
		
		if(sw1IsPressed()) //if adj - key pressed, increment/decrement value and clamp value
		{
			currentFieldValue--;
			changesMadeFlag = 1;
		}
		else if(sw2IsPressed()) //if adj + key pressed, increment/decrement value and clamp value
		{
			currentFieldValue++;
			changesMadeFlag = 1;
		}
		else if(sw3IsPressed()) //if display button pressed, increment field.
		{
			currentFieldSelected++;
			changesMadeFlag = 1;
		}
		
		if(changesMadeFlag) //apply changes
		{
			switch(currentFieldSelected)
			{
				case 0: //year field
					tempValue = currentTimeDate.f.year;
					tempValue += currentFieldValue;
					
					//clamp to 99
					if(tempValue < 0) tempValue = 0;
					else if(tempValue > 99) tempValue = 99;
					currentTimeDate.f.year = tempValue;
				break;
				
				case 1: //month field
					tempValue = currentTimeDate.f.mon;
					tempValue += currentFieldValue;
					
					//clamp to 12
					if(tempValue < 1) tempValue = 1;
					else if(tempValue > 12) tempValue = 12;
					currentTimeDate.f.mon = tempValue;
				break;
				
				case 2: //day field
					tempValue = currentTimeDate.f.mday;
					tempValue += currentFieldValue;
					
					if(tempValue < 1) tempValue = 1;
					else
					{
						//clamp to appropriate days in month
						tempValue2 = currentTimeDate.f.mon;
						if(tempValue2 == 2) //February
						{
							tempValue3 = currentTimeDate.f.year;
							if((tempValue3 % 4) == 0) //on a leap year
							{
								if(tempValue > 29)
								{
									tempValue = 29;
								}
								
							}
							else if(tempValue > 28)
							{
								tempValue = 28;
							}
						}
						else if( (tempValue2 == 9) || (tempValue2 == 4) || (tempValue2 == 6) || (tempValue2 == 11))
						{
							if(tempValue > 30)
							{
								tempValue = 30;
							}
						}
						else
						{
							if(tempValue > 31)
							{
								tempValue = 31;
							}
						}
					}
					currentTimeDate.f.mday = tempValue;
				break;
				
				case 3: //hour field
					tempValue = currentTimeDate.f.hour;
					tempValue += currentFieldValue;
					
					//clamp to 23
					if(tempValue < 0) tempValue = 0;
					if(tempValue > 23) tempValue = 23;
					currentTimeDate.f.hour = tempValue;
				break;
				
				case 4: //minute field
					tempValue = currentTimeDate.f.min;
					tempValue += currentFieldValue;
					
					//clamp to 59
					if(tempValue < 0) tempValue = 0;
					if(tempValue > 59) tempValue = 59;
					currentTimeDate.f.min = tempValue;
				break;
				
				/*
				case 5: //second field
					tempValue = currentTimeDate.f.sec;
					tempValue += currentFieldValue;
					
					//clamp to 59
					if(tempValue < 0) tempValue = 0;
					if(tempValue > 59) tempValue = 59;
					currentTimeDate.f.sec = tempValue;
				break;
				*/
				
				default:
				break;
			}
			
			//set display digits, establish field selection
			digits[0] = getSegments(2);
			digits[1] = getSegments(0);
			digits[2] = getSegments((currentTimeDate.f.year / 10));
			digits[3] = getSegments((currentTimeDate.f.year % 10));
			
			digits[4] = getSegments((currentTimeDate.f.mon / 10));
			digits[5] = getSegments((currentTimeDate.f.mon % 10));
			
			digits[6] = getSegments((currentTimeDate.f.mday / 10));
			digits[7] = getSegments((currentTimeDate.f.mday % 10));
			
			//leave a blank space between date and time
			
			digits[10] = getSegments((currentTimeDate.f.hour / 10));
			digits[11] = getSegments((currentTimeDate.f.hour % 10));
			
			digits[12] = getSegments((currentTimeDate.f.min / 10));
			digits[13] = getSegments((currentTimeDate.f.min % 10));
			
			//digits[12] = getSegments(0 /*(currentTimeDate.f.hour / 10)*/); 
			//digits[13] = getSegments(0 /*(currentTimeDate.f.hour / 10)*/);
			
			//flash digit pair using RTCC half second interrupt OR use decimal points to show current field
			switch(currentFieldSelected)
			{
				case 0:
					//set decimal points within year field
					displayDp(1,0);
					displayDp(1,1);
					displayDp(1,2);
					displayDp(1,3);
				break;
				
				case 1:
					//set decimal points within month field
					displayDp(1,4);
					displayDp(1,5);
				break;
				
				case 2:
					//set decimal points within day field
					displayDp(1,6);
					displayDp(1,7);
				break;
				
				case 3:
					//set decimal points within hour field
					displayDp(1,10);
					displayDp(1,11);
				break;
				
				case 4:
					//set decimal points within minute field
					displayDp(1,12);
					displayDp(1,13);
				break;
				
				/*
				case 5:
					//set decimal points within second field
					displayDp(1,12);
					displayDp(1,13);
				break;
				*/
				
				default:
				break;
			}	
			
			currentFieldValue = 0;
			timer1Ticks = 0; //reset inactivity timeout counter
			changesMadeFlag = 0;
		}
		
		if(currentFieldSelected > 4 /*5*/) //If all fields have been cycled through, save and exit time-set mode.
		{
			//convert fields back to BCD
			tempValue = hex2bcd(currentTimeDate.f.year);
			currentTimeDate.f.year = tempValue;
			tempValue = hex2bcd(currentTimeDate.f.mon);
			currentTimeDate.f.mon = tempValue;
			tempValue = hex2bcd(currentTimeDate.f.mday);
			currentTimeDate.f.mday = tempValue;
			tempValue = hex2bcd(currentTimeDate.f.hour);
			currentTimeDate.f.hour = tempValue;
			tempValue = hex2bcd(currentTimeDate.f.min);
			currentTimeDate.f.min = tempValue;
			/*
			tempValue = hex2bcd(currentTimeDate.f.sec);
			currentTimeDate.f.sec = tempValue;
			*/
			currentTimeDate.f.sec = 0;
			
			Time_Input_out_of_range = RtccWriteTimeDate(&currentTimeDate, TRUE); //save settings to RTCC
	
			if(Time_Input_out_of_range == 0)
			{
				 //object has values out of range
				 while(1); //panic, we're going to die.
		 	}
			
			break; 
		}
	}	
	
	//clear screen and turn off display refresh
	T2CON = 0; //TIMER2 OFF
	IEC0bits.T2IE = 0; //TIMER2 interrupt disabled
	
	clear7Segment();
	
	T1CONbits.TON = 0; //TIMER1 OFF
	timer1Ticks = 0; //reset inactivity timeout counter
	
}

void setOptions(void)
{
	//unsigned char keysInUse = 1;
	unsigned char changesMadeFlag;
	signed int currentFieldValue, tempValue;
	unsigned int currentFieldSelected;
	
	currentFieldValue = currentFieldSelected = 0;
	changesMadeFlag = 1;
	
	//display arrangement is: SEt 0 .1.
	
	//TODO: display "SEt"
	digits[0] = getSegments(5); //'S'
	setSegments(0b01111001, 1); //'E'
	setSegments(0b01111000, 2); //'t'
	
	//set decimal points within setting field
	displayDp(1,0);
	displayDp(1,1);
	
	//turn on display refresh interrupt
	T2CON = 0b1000000000000000; //FOSC/2, 1:1, TIMER2 ON
	IEC0bits.T2IE = 1; //TIMER2 interrupt enabled
	
	TMR1 = 0; //reload timer with 2-second interval
	//TMR1 = TIMER2PRELOADVALUE; //reload timer
	T1CON = 0b1000000000000010; //SOSC, 1:1, TIMER1 ON
	IEC0bits.T1IE = 1; //TIMER1 interrupt enabled
	
	//scan keys
	while(1 /*keysInUse*/)
	{		
		if(timer1Ticks > 10) break; //if enough one-second intervals have passed, exit time-set mode and discard changes
		
		if(sw1IsPressed()) //if adj - key pressed, increment/decrement value and clamp value
		{
			currentFieldValue--;
			changesMadeFlag = 1;
		}
		else if(sw2IsPressed()) //if adj + key pressed, increment/decrement value and clamp value
		{
			currentFieldValue++;
			changesMadeFlag = 1;
		}
		else if(sw3IsPressed()) //if display button pressed, increment field.
		{
			currentFieldSelected++;
			changesMadeFlag = 1;
		}
		
		if(changesMadeFlag) //apply changes
		{
			switch(currentFieldSelected)
			{
				case 0: //12-24 hour display field
					tempValue = CONF1bits.hr24;
					tempValue += currentFieldValue;
					
					//clamp to 0 or 1
					if(tempValue < 0) tempValue = 1;
					else if(tempValue > 1) tempValue = 0;
					CONF1bits.hr24 = tempValue;
				break;
				
				case 1: //showDateWithTime field
					tempValue = CONF1bits.showDateWithTime;
					tempValue += currentFieldValue;
					
					//clamp to 0 or 1
					if(tempValue < 0) tempValue = 1;
					else if(tempValue > 1) tempValue = 0;
					CONF1bits.showDateWithTime = tempValue;
				break;
				
				case 2: //showSeconds field
					tempValue = CONF1bits.showSeconds;
					tempValue += currentFieldValue;
					
					//clamp to 0 or 1
					if(tempValue < 0) tempValue = 1;
					else if(tempValue > 1) tempValue = 0;
					CONF1bits.showSeconds = tempValue;
				break;
				
				default:
				break;
			}
			
			//update display
			digits[4] = getSegments(currentFieldSelected); //current setting to be adjusted
			digits[6] = getSegments(tempValue); //current setting value
			
			currentFieldValue = 0;
			timer1Ticks = 0; //reset inactivity timeout counter
			changesMadeFlag = 0;
		}
		
		if(currentFieldSelected > 2) //If all fields have been cycled through, exit set mode.
		{
			break; 
		}
	}	
	
	//clear screen and turn off display refresh
	T2CON = 0; //TIMER2 OFF
	IEC0bits.T2IE = 0; //TIMER2 interrupt disabled
	
	clear7Segment();
	
	T1CONbits.TON = 0; //TIMER1 OFF
	timer1Ticks = 0; //reset inactivity timeout counter
}	

void showTime(void)
{
	unsigned char tempValue;
	
	//grab rtccTimeDate
	RtccReadTimeDate(&currentTimeDate);
	
	//convert fields to hex
	tempValue = bcd2hex(currentTimeDate.f.year);
	currentTimeDate.f.year = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.mon);
	currentTimeDate.f.mon = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.mday);
	currentTimeDate.f.mday = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.hour);
	currentTimeDate.f.hour = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.min);
	currentTimeDate.f.min = tempValue;
	tempValue = bcd2hex(currentTimeDate.f.sec);
	currentTimeDate.f.sec = tempValue;
	
	//set display digits
	if(CONF1bits.showDateWithTime == 1) //showing date with time
	{
		digits[0] = getSegments(2);
		digits[1] = getSegments(0);
		digits[2] = getSegments((currentTimeDate.f.year / 10));
		digits[3] = getSegments((currentTimeDate.f.year % 10));
		
		digits[4] = getSegments((currentTimeDate.f.mon / 10));
		digits[5] = getSegments((currentTimeDate.f.mon % 10));
		
		digits[6] = getSegments((currentTimeDate.f.mday / 10));
		digits[7] = getSegments((currentTimeDate.f.mday % 10));
		
		//leave a blank space between date and time
		
		if(CONF1bits.hr24 == 1) //using 00 to 23 hours
		{	
			digits[10] = getSegments((currentTimeDate.f.hour / 10));
			digits[11] = getSegments((currentTimeDate.f.hour % 10));
			
			digits[12] = getSegments((currentTimeDate.f.min / 10));
			digits[13] = getSegments((currentTimeDate.f.min % 10));	
			
			//insert field separators
			displayDp(1,3); //year
			displayDp(1,5); //month
			//displayDp(1,7); //day
			displayDp(1,11); //hour
			//displayDp(1,13); //minute
		}
		else //using AM and PM
		{
			if(currentTimeDate.f.hour == 0) //for 12am
			{
				currentTimeDate.f.hour += 12;
				setSegments(0b01110111, 13); //'A'
			}
			else if(currentTimeDate.f.hour < 12) //for 1am through 11am
			{
				setSegments(0b01110111, 13); //'A' 
			}
			else if(currentTimeDate.f.hour == 12) //for 12pm
			{
				setSegments(0b01110011, 13); //'P'
			}
			else if(currentTimeDate.f.hour > 12) //for 1pm through 11pm
			{
				currentTimeDate.f.hour -= 12;
				setSegments(0b01110011, 13); //'P'
			}
			
			digits[9] = getSegments((currentTimeDate.f.hour / 10));
			digits[10] = getSegments((currentTimeDate.f.hour % 10));
			
			digits[11] = getSegments((currentTimeDate.f.min / 10));
			digits[12] = getSegments((currentTimeDate.f.min % 10));	
			
			//insert field separators
			displayDp(1,3); //year
			displayDp(1,5); //month
			//displayDp(1,7); //day
			displayDp(1,10); //hour
			//displayDp(1,13); //minute
		}
	}
	else  //showing the time only
	{
		//TODO: center time in display
		
		if(CONF1bits.showSeconds == 1)
		{
			if(CONF1bits.hr24 == 1) //using 00 to 23 hours
			{
				
			}	
			else //using AM and PM
			{
				if(currentTimeDate.f.hour == 0) //for 12am
				{
					currentTimeDate.f.hour += 12;
					setSegments(0b01110111, 6); //'A'
				}
				else if(currentTimeDate.f.hour < 12) //for 1am through 11am
				{
					setSegments(0b01110111, 6); //'A' 
				}
				else if(currentTimeDate.f.hour == 12) //for 12pm
				{
					setSegments(0b01110011, 6); //'P'
				}
				else if(currentTimeDate.f.hour > 12) //for 1pm through 11pm
				{
					currentTimeDate.f.hour -= 12;
					setSegments(0b01110011, 6); //'P'
				}
			}
			
			digits[0] = getSegments((currentTimeDate.f.hour / 10));
			digits[1] = getSegments((currentTimeDate.f.hour % 10));
			
			digits[2] = getSegments((currentTimeDate.f.min / 10));
			digits[3] = getSegments((currentTimeDate.f.min % 10));
			
			digits[4] = getSegments((currentTimeDate.f.sec / 10)); 
			digits[5] = getSegments((currentTimeDate.f.sec % 10));

			//insert field separators
			displayDp(1,1); //hour
			displayDp(1,3); //minute
		}
		else
		{			
			if(CONF1bits.hr24 == 1) //using 00 to 23 hours
			{
				
			}	
			else //using AM and PM
			{
				if(currentTimeDate.f.hour == 0) //for 12am
				{
					currentTimeDate.f.hour += 12;
					setSegments(0b01110111, 4); //'A'
				}
				else if(currentTimeDate.f.hour < 12) //for 1am through 11am
				{
					setSegments(0b01110111, 4); //'A' 
				}
				else if(currentTimeDate.f.hour == 12) //for 12pm
				{
					setSegments(0b01110011, 4); //'P'
				}
				else if(currentTimeDate.f.hour > 12) //for 1pm through 11pm
				{
					currentTimeDate.f.hour -= 12;
					setSegments(0b01110011, 4); //'P'
				}
			}	
			
			digits[0] = getSegments((currentTimeDate.f.hour / 10));
			digits[1] = getSegments((currentTimeDate.f.hour % 10));
			
			digits[2] = getSegments((currentTimeDate.f.min / 10));
			digits[3] = getSegments((currentTimeDate.f.min % 10));
			
			//insert field separators
			displayDp(1,1); //hour
		}
	}	
		
	//turn on display refresh interrupt
	TMR2 = 0xED66; //reload timer
	T2CON = 0b1000000000000000; //FOSC/2, 1:1, TIMER2 ON
	IEC0bits.T2IE = 1; //TIMER2 interrupt enabled
	
	//wait for user to read time
	TMR1 = 0; //reload timer with 2-second interval
	//TMR1 = TIMER1PRELOADVALUE; //reload timer
	T1CON = 0b1000000000000010; //SOSC, 1:1, TIMER1 ON
	IEC0bits.T1IE = 1; //TIMER1 interrupt enabled
		
	if( (CONF1bits.showSeconds == 1) && (CONF1bits.showDateWithTime == 0) )
	{
		alarmFlag = 0; //reset alarm flag
		ALCFGRPTbits.ALRMEN = 1; //enable one-second alarm
		IEC3bits.RTCIE = 1;
	}	
	
	//TODO: wait for timer to expire, preferably sleeping in some way.
	timer1Ticks = 0;
	while(timer1Ticks < 3)
	{
		if( (CONF1bits.showSeconds == 1) && (CONF1bits.showDateWithTime == 0) )
		{
			if(alarmFlag == 1) //if a second has passed, refresh seconds field in display
			{
				//grab rtccTimeDate
				RtccReadTimeDate(&currentTimeDate);
				
				//convert fields to hex
				tempValue = bcd2hex(currentTimeDate.f.min);
				currentTimeDate.f.min = tempValue;
				
				digits[2] = getSegments((currentTimeDate.f.min / 10)); 
				digits[3] = getSegments((currentTimeDate.f.min % 10));
				
				displayDp(1,3); //minute field separator
				
				tempValue = bcd2hex(currentTimeDate.f.sec);
				currentTimeDate.f.sec = tempValue;
				
				digits[4] = getSegments((currentTimeDate.f.sec / 10)); 
				digits[5] = getSegments((currentTimeDate.f.sec % 10));
				
				alarmFlag = 0; //reset alarm flag
			}
		}	
		
		//Sleep();
		asm( "PWRSAV #1" ); //enter IDLE mode
		Nop();
	}
	T1CONbits.TON = 0; //TIMER1 OFF
	
	//clear screen and turn off display refresh
	T2CONbits.TON = 0; //TIMER1 OFF
	//IEC0bits.T2IE = 0; //TIMER1 interrupt disabled
	
	if( (CONF1bits.showSeconds == 1) && (CONF1bits.showDateWithTime == 0) )
	{
		ALCFGRPTbits.ALRMEN = 0; //disable one-second interrupt
		IEC3bits.RTCIE = 0;
		alarmFlag = 0; //reset alarm flag
	}
	
	clear7Segment();
}	

unsigned char bcd2hex(unsigned char x)
{
    unsigned char y;
    y = (x >> 4) * 10;
    y += (x & 0b00001111);
    return (y);
}

unsigned char hex2bcd(unsigned char x)
{
    unsigned char y;
    y = (x / 10) << 4;
    y = y | (x % 10);
    return (y);
}


