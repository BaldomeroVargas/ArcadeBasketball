/* 
	Baldomero Vargas
	*EE/CS 120B Custom Lab Project
	*Arcade Basketball
*/

#include <avr/io.h>
#include <avr/eeprom.h>
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\scheduler.h"
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\timer.h"
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\shift_registers.h"
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\io.c"
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\seven_seg.h"
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\bit.h"
#include "C:\Users\Baldomero\Documents\Atmel Studio\6.2\Project_Headers\states.h"

int main(void)
{
	Init_HS();
	//setting up PORTS as inputs/outputs
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0X00;
	
	//Period for the tasks
	unsigned long int Input_calc = 50;
	unsigned long int LCD_calc = 100;
	unsigned long int IR_calc = 50;
	unsigned long int Seven_calc = 1000;
	unsigned long int Motor_calc = 2;
	
	//calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(Input_calc,LCD_calc);
	tmpGCD = findGCD(tmpGCD, IR_calc);
	tmpGCD = findGCD(tmpGCD, Seven_calc);
	tmpGCD = findGCD(tmpGCD, Motor_calc);
	
	//greatest common divisor for all tasks
	unsigned long int GCD = tmpGCD;
	
	//Recalculating GCD periods for scheduler
	unsigned long int Input_period = Input_calc/GCD;
	unsigned long int LCD_period = LCD_calc/GCD;
	unsigned long int IR_period = IR_calc/GCD;
	unsigned long int Seven_period = Seven_calc/GCD;
	unsigned long int Motor_period = Motor_calc/GCD;
		
	//declare an array of tasks
	static task Input_Task, LCD_Task, IR_Task, Seven_Task, Motor_Task;
	task *tasks[] = {&Input_Task, &LCD_Task, &IR_Task,&Seven_Task, &Motor_Task};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	//input task
	Input_Task.state = 0;
	Input_Task.period = Input_period;
	Input_Task.elapsedTime = Input_period;
	Input_Task.TickFct = &InputTick;
	
	//LCD task
	LCD_Task.state = 0;
	LCD_Task.period = LCD_period;
	LCD_Task.elapsedTime = LCD_period;
	LCD_Task.TickFct = &LCD_Tick;
	
	//IR task
	IR_Task.state = 0;
	IR_Task.period = IR_period;
	IR_Task.elapsedTime = IR_period;
	IR_Task.TickFct = &IR_Tick;
	
	//Seven Seg task
	Seven_Task.state = 0;
	Seven_Task.period = Seven_period;
	Seven_Task.elapsedTime = Seven_period;
	Seven_Task.TickFct = &Seven_Tick;
	
	//motor task
	Motor_Task.state = 0;
	Motor_Task.period = Motor_period;
	Motor_Task.elapsedTime = Motor_period;
	Motor_Task.TickFct = &Motor_Tick;
	
	//Set timer and turn it on
	TimerSet(GCD);
	TimerOn();
	ADC_init();
	LCD_init();
	
	unsigned short i; //Scheduler for-loop iterator
	
	while(1) {
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
		
	}
	// Error: Program should not exit!
	return 0;
}
