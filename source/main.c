/*
 * GccApplication22.c
 *
 * Created: 7/22/2019 3:24:17 PM
 * Author : njmar
 */ 


#include <avr/io.h>
#include <avr/eeprom.h>
#include "scheduler.h"
#include "bit.h"
#include "io.c"
#include "io.h"
#include "timer.h"



static task task1, task2,task3, task4, task5;
task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
	
char upperMap[57] = {0,1,0,0, 2,0,0,1, 0,0,0,0, 3,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,3, 0,0,0,0, 1,0,0,0, 0,0,0,0, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,};
char lowerMap[57] = {0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,0, 0,0,2,0, 0,0,0,0, 0,0,0,0, 0,0,0,1, 0,0,0,0, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,};
char upperMapReset[57] = {0,1,0,0, 2,0,0,1, 0,0,0,0, 3,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,3, 0,0,0,0, 1,0,0,0, 0,0,0,0, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,};
char lowerMapReset[57] = {0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,0, 0,0,2,0, 0,0,0,0, 0,0,0,0, 0,0,0,1, 0,0,0,0, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,};
	

unsigned char playerPos = 0;
unsigned char mapScrollCount = 0;
unsigned char score =0;
unsigned char previousScore=0;
unsigned char mapStart = 0;
unsigned char button = 0x00;
unsigned char i = 0;


void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

enum GameStates {start, startWait, startPress, win, lose};
int gameTick(int state)
{
	
	int previousState = state;
	switch(state)
	{
		case start:
		state = startWait;
		break;
		
		case startWait:
		state = (button)? startPress: startWait; break;
		break;
		
		case startPress:
		if(button){
			state = startWait;
		}else{
			if(mapStart >= 40){
				state = win;
			}else{
					if(playerPos == 1 && upperMap[mapStart]==1){
						state = lose;
					}
					
					if(playerPos == 17 && lowerMap[mapStart]==1){
						state = lose;
					}
			}
		}
		break;
		
		case win:
		state = (button)? startWait: win; break;
		
		case lose:
		state = (button)? startWait: lose; break;
		default:
		break;
	}
	switch(state)
	{
		case start:
		LCD_ClearScreen();
		break;
		
		case startWait:
		score=0;
		playerPos = 17;
		memcpy (upperMap, upperMapReset, sizeof (upperMap));
		memcpy (lowerMap, lowerMapReset, sizeof (lowerMap));
		if(previousState != startWait){
			LCD_DisplayString(1, "START           Press to play!");	
		}
		break;
		
		case startPress:
		break;
		
		case win:
		if(previousState != win){
			
			previousScore= eeprom_read_byte((uint8_t*)1);
			
			LCD_DisplayString(1, "*:  WIN!  PREV*:");
			LCD_Cursor(17);
			LCD_WriteData(score/10+'0');
			LCD_WriteData(score%10+'0');
			
			LCD_Cursor(31);
			LCD_WriteData(previousScore/10+'0');
			LCD_WriteData(previousScore%10+'0');
			
			eeprom_write_byte((uint8_t*)1,score);
		}
		break;
		
		case lose:
		if(previousState != lose ){
			previousScore= eeprom_read_byte((uint8_t*)1);
			
			LCD_DisplayString(1, "*:  LOSE  PREV*:");
			LCD_Cursor(17);
			LCD_WriteData(score/10+'0');
			LCD_WriteData(score%10+'0');
			
			LCD_Cursor(31);
			LCD_WriteData(previousScore/10+'0');
			LCD_WriteData(previousScore%10+'0');
			
			eeprom_write_byte((uint8_t*)1,score);	
		}
		break;
	}
	return state;
}


void cursorAt(unsigned char row, unsigned char col)
{
	if(row == 1){
		LCD_Cursor(col+1);
		
	}
	else if(row == 2){
		LCD_Cursor(col+17);
	}
}


enum mapScrollStates { mapWait, mapMove};

int mapTick(int state)
{
	unsigned char i = 0;
	switch(state)
	{
		case mapWait:
			state = (task1.state == startPress )? mapMove: mapWait; break;
			
		case mapMove:
			state = (task1.state == startPress )? mapMove:mapWait;break;
			
		default:
			break;
			
	}
	switch(state)
	{
		case mapWait:
			mapStart=0;
			score = 0;
			break;
		case mapMove:
			LCD_ClearScreen();
			
			mapStart++;
			score++;
			
			for(i = mapStart; i < mapStart+16; i++)
			{
				if(upperMap[i]==1){
					cursorAt(1, (i-mapStart));
					LCD_WriteData(1);
				}
				
				if(lowerMap[i]==1){
					cursorAt(2, (i-mapStart));
					LCD_WriteData(1);
				}
				
				if(upperMap[i]==2){
					cursorAt(1, (i-mapStart));
					LCD_WriteData(2);
				}
				
				if(lowerMap[i]==2){
					cursorAt(2, (i-mapStart));
					LCD_WriteData(2);
				}
				
				if(upperMap[i]==3){
					cursorAt(1, (i-mapStart));
					LCD_WriteData(3);
				}
				
				if(lowerMap[i]==3){
					cursorAt(2, (i-mapStart));
					LCD_WriteData(3);
				}
			}
			
			break;
		default:
			break;
	}
	return state;
}



enum playerStates {playerUp,playerDown};
int playerTick(int state){
	switch(state){
		case playerUp:
		if(ADC >= 650){
			state = playerUp;
			playerPos=1;
		}else if(ADC <= 250){
			state = playerDown;
			playerPos=17;
		}
		break;
		
		case playerDown:
		if(ADC >= 650){
			state = playerUp;
			playerPos=1;
		}else if(ADC <= 250){
			state = playerDown;
			playerPos=17;
		}
		 break;
			
		
		default:
			break;
	}
	return state;
}


enum playerPrintStates {printWait, printPlayer};
int playerPrintTick(int state)
{
	switch(state)
	{
		case printWait:
		state = (task1.state == startPress )? printPlayer: printWait; break;
		
		case printPlayer:
		state = (task1.state == startPress )? printPlayer: printWait; break;
		
		default:
		break;
	}
	switch(state)
	{
		case printWait:
		break;
		
		case printPlayer:
		LCD_Cursor(playerPos);
		LCD_WriteCommand(0x0C);
		LCD_WriteData(0);
		
		break;
		
		default:
		break;
	}
	return state;
}


enum lightStates {lightWait, led1, led2, led3, led4, led5};
int lightTick(int state)
{
	switch(state)
	{
		case lightWait:
		state = (task1.state == startPress)? led1: lightWait; break;
		
		case led1:
		state = (task1.state == startPress)? led2: lightWait; break;
		
		case led2:
		state = (task1.state == startPress)? led3: lightWait; break;
		
		case led3:
		state = (task1.state == startPress)? led4: lightWait; break;
		
		case led4:
		state = (task1.state == startPress)? led5: lightWait; break;
		
		case led5:
		state = (task1.state == startPress)? led1: lightWait; break;
		break;
		
		default:
		break;
	}
	switch(state)
	{
		case lightWait:
		PORTB = 0x1F;	
		break;
		
		case led1:
		PORTB = 0x01;
		break;
		
		case led2:
		PORTB = 0x02;
		break;
		
		case led3:
		PORTB = 0x04;
		break;
		
		case led4:
		PORTB = 0x08;
		break;
		
		case led5:
		PORTB = 0x10;
		break;
		
		default:
		break;
	}
	return state;
}

int main(void) {

	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	
	
	unsigned char Character1[8] = { 0x0E, 0x0A, 0x0E, 0x04, 0x1F, 0x04, 0x0A, 0x11 };
	unsigned char Character2[8] = { 0x1F, 0x11, 0x1B, 0x15, 0x15, 0x1B, 0x11, 0x1F };
	unsigned char Character3[8] = {	0x0E, 0x1F, 0x13, 0x1B, 0x1B, 0x11, 0x1F, 0x0E };
	unsigned char Character4[8] = {	0x00, 0x00, 0x0A, 0x0A, 0x00, 0x11, 0x0E, 0x00 };	
	LCD_CustomChar(0, Character1);
	LCD_CustomChar(1, Character2);
	LCD_CustomChar(2, Character3);
	LCD_CustomChar(3, Character4);
	
	LCD_init();
	LCD_ClearScreen();
	
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
 
    task1.state =  start;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &gameTick;
    
    
    task2.state = mapWait;
    task2.period =100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &mapTick;
	
	task3.state = playerDown;
	task3.period =10;
	task3.elapsedTime = task3.period;
	task3.TickFct = &playerTick;
	
	task4.state = printPlayer;
	task4.period =10;
	task4.elapsedTime = task4.period;
	task4.TickFct = &playerPrintTick;
	
	task5.state = lightWait;
	task5.period =100;
	task5.elapsedTime = task5.period;
	task5.TickFct = &lightTick;
	
	unsigned long GCD = tasks[0]->period;
	for(int i = 1;i <numTasks; i++){
		GCD = findGCD(GCD,tasks[i]->period);
	}
    TimerSet(GCD);
    TimerOn();
	
	
    
    unsigned short i;
    while (1) {
		ADC_init();
		button = ~PINA & 0x02;
		for(i = 0; i < numTasks; i++){
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state= tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}else tasks[i]->elapsedTime += GCD; 
			
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}



