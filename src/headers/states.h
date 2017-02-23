#ifndef STATES_H
#define STATES_H
//////////////////////////////////////////////////
/*Shared Variables*/
	
/*Motor Logic*/
const unsigned char CCW[] = {0x06, 0x02, 0x0A, 0x08, 0x09, 0x01, 0x05, 0x04};
const unsigned char CW[] = {0x04, 0x05, 0x01, 0x09, 0x08, 0x0A, 0x02, 0x06};
	
/*Game mode 2 motor Logic*/
//7 stations to choose from
const unsigned short small_move = 256;
unsigned char mottor_position = 3;

//current steps counter
unsigned short M1steps = 0;
unsigned short M2steps = 0;
	
/*IR Reading*/
const unsigned short IRclear = 0x0370;

/*Seven Segs*/
signed char TimerAttack = 40;

/*Inputs*/
unsigned char tmpA;

/*outputs*/
unsigned char tmpD = 0;

/*for soft reset*/
// (-768 <-> +768)
signed short motor_reset = 0;
//sets to 1 if reset flag stayed on
unsigned char reset_flag = 0;

/*Time attack game mode*/
uint8_t hs_temp;
unsigned short highScore = 0;
unsigned short playerScore = 0;

/*"Bit" Horse game mode*/
const unsigned char letters[] = {'B','I','T'};
unsigned char lettersP1[3] = {' ',' ',' '};
unsigned char lettersP2[3] = {' ',' ',' '};
unsigned char P1_letter_count = 0;
unsigned char P2_letter_count = 0;
unsigned char P1_Cursor = 5;
unsigned char P2_Cursor = 21;
	
/*LCD Prompts*/
//menu
const unsigned char * menuTop = "Time Attack (A)";
const unsigned char * menuBot = "Bit (B)";
//Game 1
const unsigned char * HSDisplay = "High Score: ";
const unsigned char * PlayerDisplay = "Your Score: ";
const unsigned char * NewHS = "New High Score!";
const unsigned char * TryAgain = "Try Again!";
//Game 2
const unsigned char * P1Win = "Player 1 Wins!";
const unsigned char * P2Win = "Player 2 Wins!";
//score arrarys
unsigned char hs_array[3] = {0,0,0};
unsigned char yours_array[3] = {0,0,0};

/*Shared & flags*/
unsigned char GameMode1 = 0;
unsigned char GameMode2 = 0;
unsigned char madeBasket = 0;
unsigned char firstNum;
unsigned char secondNum;
unsigned short total;

/*Gamemode 2 specific*/
unsigned char current_player = 1;
unsigned char need_to_make = 0;
unsigned char made_shot = 0;
signed char countdown_to_make = 5;
unsigned char M2winner = 0;
unsigned char locked_location = 0;
unsigned char leftMove = 0;
unsigned char rightMove = 0;
unsigned char lock_in_flag = 0;

/*Function Declarations*/
void ADC_init() {
	ADCSRA = 0xE7;
	//ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADPS2) | (1 << ADPS1);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	// in Free Running Mode, a new conversion will trigger whenever
	// the previous conversion completes.
}

//read from mem location 75
void Init_HS(){
	unsigned char hundreds = 0;
	hs_temp = eeprom_read_byte((uint8_t*)75);
	highScore = hs_temp;
	hs_array[0] = highScore / 100;

	while(highScore >= 100){
		highScore -= 100;
		hundreds++;
	}
	highScore = highScore + (hundreds * 100);
	if(highScore >= 100){
		hs_array[1] = (highScore - (hundreds * 100)) / 10;
		hs_array[2] = (highScore - (hundreds * 100)) % 10;
	}
	else{
		hs_array[1] = highScore / 10;
		hs_array[2] = highScore % 10;		
	}
	
	firstNum = TimerAttack % 10;
	secondNum = TimerAttack / 10;
	firstNum = Write7Seg(firstNum);
	secondNum = Write7Seg(secondNum);
	total = (short)secondNum << 8;
	total |= (short)firstNum;
	transmit_chain_upper(total);
}

//////////////////////////////////////////////////
enum Input_States {toggle};
	
int InputTick(int state) {
	
	switch(state){
		case toggle: 
			tmpA = ~PINA;
			state = toggle;
			break;
		
		default:
		state = toggle;
			break;
	}
	return state;
}

//////////////////////////////////////////////////
enum LCD_States {LCDstart, menu, GM1Beg, GM1End, GM2Beg, GM2End };

int LCD_Tick(int state) {
		
	static unsigned char hun_count = 0;
	static unsigned short waitTime = 0;
	//transitions
	switch (state){
		case LCDstart:	
			hun_count = 0;
			LCD_ClearScreen();
			yours_array[2] = 0;
			yours_array[1] = 0;
			yours_array[0] = 0;
			GameMode1 = GameMode2 = 0;
			LCD_DisplayString(1,menuTop);
			LCD_DisplayString(17,menuBot);
			firstNum = Write7Seg(0);
			secondNum = Write7Seg(0);
			total = (short)secondNum << 8;
			total |= (short)firstNum;
			transmit_chain_upper(total);
			TimerAttack = 40;
			state = menu;
			break;
		
		case menu:
		//button A
			if( GetBit(tmpA,7) && !(GetBit(tmpA,6)) && !(GetBit(tmpA,5)) ){
				//send game into time attack
				state = GM1Beg;
				GameMode1 = 1;
				LCD_ClearScreen();
				//HS OUT
				LCD_DisplayString(1,HSDisplay);
				LCD_Cursor(13); LCD_WriteData(hs_array[0] + 48);
				LCD_Cursor(14); LCD_WriteData(hs_array[1] + 48);
				LCD_Cursor(15); LCD_WriteData(hs_array[2] + 48);
				//YOUR OUT
				LCD_Cursor(17);
				LCD_DisplayString(17,PlayerDisplay);					
				LCD_Cursor(29); LCD_WriteData(yours_array[0] + 48);
				LCD_Cursor(30); LCD_WriteData(yours_array[1] + 48);			
				LCD_Cursor(31); LCD_WriteData(yours_array[2] + 48);						
				break;
			}
		//button B
			if( !(GetBit(tmpA,7)) && (GetBit(tmpA,6)) && !(GetBit(tmpA,5)) ){
				//send game into Bit
				state = GM2Beg;
				GameMode2 = 1;
				LCD_ClearScreen();
				LCD_DisplayString(1, "P1: ");
				LCD_DisplayString(17, "P2: ");
				break;
			}
			break;
		
		case GM1Beg:
			//reset check
			if( GetBit(tmpA,4) ){
				//sets reset flag to 1
				reset_flag = 1;
				GameMode1 = 0; GameMode2 = 0; playerScore = 0;
				yours_array[0] = 0; yours_array[1] = 0;	yours_array[2] = 0;
				current_player = 1; waitTime = 0; P1_Cursor = 5;
				P2_Cursor = 21; TimerAttack = 40;
				P2_letter_count = P1_letter_count = 0;
				lock_in_flag = 0; need_to_make = 0;	countdown_to_make = 5;
				made_shot = 0; tmpD = 0; 
				hun_count = 0;
				state = LCDstart;
				break;
			}

			if(GameMode1 == 0){
				tmpD = 0;
				state = GM1End;
				LCD_ClearScreen();
				if(highScore > playerScore){
					LCD_DisplayString(1,TryAgain);
					playerScore = 0;
				}
				else{
					highScore = playerScore;
					playerScore = 0;
					LCD_ClearScreen();
					LCD_DisplayString(1,NewHS);
					//eeprom
					eeprom_write_byte((uint8_t*)75, highScore);					
				}
				yours_array[0] = 0;
				yours_array[1] = 0;
				yours_array[2] = 0;
				hs_array[0] = highScore / 100;
				
				while(highScore >= 100){
					highScore -= 100;
					hun_count++;
				}
				highScore = highScore + (hun_count * 100);
				if(highScore >= 100){
					hs_array[1] = (highScore - (hun_count*100)) / 10;
					hs_array[2] = (highScore - (hun_count*100)) % 10;					
				}
				else{
					hs_array[1] = highScore / 10;
					hs_array[2] = highScore % 10;					
				}
				hun_count = 0;
			}
			else{state = GM1Beg;}
			break;
		
		case GM1End:
			//loop back to main menu
			if(waitTime >= 50){
				state = LCDstart;
				waitTime = 0;
			}
			else{
				state = GM1End;
			}
			break;
		
		case GM2Beg:
			//reset check
			if( GetBit(tmpA,4) ){
				//sets reset flag to 1
				state = LCDstart;
				reset_flag = 1;
				GameMode1 = 0; GameMode2 = 0; playerScore = 0;
				tmpD = 0; yours_array[0] = 0; yours_array[1] = 0;
				yours_array[2] = 0;
				current_player = 1; waitTime = 0; P1_Cursor = 5;
				P2_Cursor = 21; TimerAttack = 40;
				P2_letter_count = P1_letter_count = 0;
				lock_in_flag = 0; need_to_make = 0;	countdown_to_make = 5;
				locked_location = 0;
				made_shot = 0;
				hun_count = 0;
				break;
			}
			if(GameMode2 == 0){
				tmpD = 0;
				LCD_ClearScreen();				
				state = GM2End;
				if(M2winner == 1){
					LCD_DisplayString(1,P1Win);
					M2winner = 0;
				}
				else if(M2winner == 2){
					LCD_DisplayString(1,P2Win);
					M2winner = 0;
				}
				
			}
			else{state = GM2Beg;}
			break;
		
		case GM2End:
			if(waitTime >= 50){
				state = LCDstart;
				current_player = 1;
				waitTime = 0;
				P1_Cursor = 5;
				P2_Cursor = 21;
			}
			else{
				state = GM2End;
			}
			break;
		
		default:
			state = LCDstart;
			break;
	}
	
	//state logic
	switch (state){
		case LCDstart:
			break;
		
		case menu:
			break;
		
		case GM1Beg:
			if(TimerAttack >=15){
				tmpD = tmpD | 0x02;
			}
			else{
				tmpD = tmpD | 0x06;
			}
			if(TimerAttack < 0){
				GameMode1 = 0;
				firstNum = Write7Seg(0);
				secondNum = Write7Seg(0);
				total = (short)secondNum << 8;
				total |= (short)firstNum;
				transmit_chain_upper(total);				
				TimerAttack = 40;
				break;
			}
			else{
				if(madeBasket){
					if(TimerAttack >= 15){
						playerScore += 2;
					}
					else{
						playerScore += 3;
					}				
					yours_array[0] = playerScore / 100;
					while(playerScore >= 100){
						playerScore -= 100;
						hun_count++;
					}
					playerScore = playerScore + (hun_count * 100);
					if(playerScore >= 100){
						yours_array[1] = (playerScore - (hun_count* 100)) / 10;
						yours_array[2] = (playerScore - (hun_count * 100)) % 10;
					}
					else{
						yours_array[1] = playerScore / 10;
						yours_array[2] = playerScore % 10;						
					}
					hun_count = 0;
					LCD_Cursor(29); LCD_WriteData(yours_array[0] + 48);
					LCD_Cursor(30); LCD_WriteData(yours_array[1] + 48);
					LCD_Cursor(31); LCD_WriteData(yours_array[2] + 48);
				}				
			}					
			break;
		
		case GM1End:
			waitTime++;					
			break;
		
		case GM2Beg:
			if(need_to_make){
				locked_location = 1;
			}
			if(lock_in_flag == 0){
				if(!(GetBit(tmpA,7)) && !(GetBit(tmpA,6)) && GetBit(tmpA,5)){
					locked_location = 1;
					lock_in_flag = 1;
					countdown_to_make = 5;
				}				
			}
			if(countdown_to_make < 0){
				locked_location = 0;
				if(current_player == 1){
					firstNum = Write7Seg(current_player + 1);					
				}
				else if(current_player == 2){
					firstNum = Write7Seg(current_player - 1);				
				}
				secondNum = Write7Seg(0);
				total = (short)secondNum << 8;
				total |= (short)firstNum;
				transmit_chain_upper(total);	
				countdown_to_make = 5;			
				if(made_shot){
					//to tie
					if(need_to_make){
						need_to_make = 0;
						lock_in_flag = 0;
					}
					else{
						need_to_make = 1;
					}
					if(current_player == 1){
						current_player = 2;
					}
					else if(current_player == 2){
						current_player = 1;
					}
				}
				else{
					if(need_to_make){
						need_to_make = 0;
						if(current_player == 1){
							lettersP1[P1_letter_count] = letters[P1_letter_count];
							P1_letter_count++;
							//write to LCD
							LCD_Cursor(P1_Cursor);
							LCD_WriteData(lettersP1[P1_letter_count - 1]);
							P1_Cursor++;
							if(P1_letter_count >=3){
								//p1 loses
								GameMode2 = 0;
								M2winner = 2;
								P2_letter_count = P1_letter_count = 0;
							}
						}
						else if (current_player == 2){
							lettersP2[P2_letter_count] = letters[P2_letter_count];
							P2_letter_count++;
							//write to LCD
							LCD_Cursor(P2_Cursor);
							LCD_WriteData(lettersP2[P2_letter_count - 1]);
							P2_Cursor++;							
							if(P2_letter_count >= 3){
								//p2 loses
								GameMode2 = 0;
								M2winner = 1;
								P2_letter_count = P1_letter_count = 0;					
							}
						}
						if(current_player == 1){
							current_player = 2;
						}
						else if(current_player == 2){
							current_player = 1;
						}
					}
					else{
						need_to_make = 0;
						if(current_player == 1){
							current_player = 2;
						}
						else if(current_player == 2){
							current_player = 1;
						}					
					}
				}
				lock_in_flag = 0;
				made_shot = 0;				
			}
			break;
		
		case GM2End:
			waitTime++;	
			break;
		
		default:
			state = LCDstart;
			break;
	}
	PORTD = tmpD;
	return state;	
}

//////////////////////////////////////////////////
enum IR_States {IR_Read};
int IR_Tick(int state){
	switch(state){
		case IR_Read:
			state = IR_Read;
			break;
		
		default:
			state = IR_Read;
			break;
	}
	
	switch(state){
		case IR_Read:
			if(GameMode2){
				if(ADC < IRclear){
					made_shot = madeBasket = 1;
					tmpD = 1;
				}
				else{
					madeBasket = 0;
					tmpD = 0;
				}	
				break;
			}
			if(ADC < IRclear){
				madeBasket = 1;	
				tmpD = tmpD | 0x01;
			}
			else{ madeBasket = 0;
				tmpD = tmpD & 0x06;}

			break;
		
		default:
			state = IR_Read;
			break;
	}
	
	PORTD = tmpD;
	return state;
};

enum Seven_States {init, mode1, mode2};
int Seven_Tick(int state){
	
	switch(state){
		case init:
			if(GameMode1){
				state = mode1;
			}
			else if(GameMode2){
				state = mode2;
			}
			else{state = init;}
			break;
		case mode1:
			if(GameMode1 == 0){
				state = init;
			}
			break;
		case mode2:
			if(GameMode2 == 0){
				state = init;
			}
			break;
		
		default:
			state = init;
			break;
	}
	
	switch(state){
		case init:
			break;
		case mode1:
			if(GameMode1){
					firstNum = TimerAttack / 10;
					secondNum = TimerAttack % 10;
				
					firstNum = Write7Seg(firstNum);
					secondNum = Write7Seg(secondNum);
					total = (short)secondNum << 8;
					total |= (short)firstNum;
					transmit_chain_upper(total);
					TimerAttack--;
			}
			break;
		case mode2:
			if(GameMode2){
				//first seven seg outputs current player
				firstNum = Write7Seg(current_player);	
				//second is the count down
				if(locked_location){
					secondNum = Write7Seg(countdown_to_make);
					total = (short)secondNum << 8;
					total |= (short)firstNum;
					transmit_chain_upper(total);
					countdown_to_make--;
				}
				
			}
			break;
		
		default:
			state = init;
			break;
	}
	return state;
}

//////////////////////////////////////////////////
enum Motor_States {Motor_load, movement1, movement2, movement3, movement4, stop, Mode2_moveL, Mode2_moveR, lock_in,  GM2_Wait, soft_reset};
int Motor_Tick(int state){
	static unsigned short delay = 0;
	static unsigned char i = 0;
	static unsigned char j = 0;
	switch(state){
		
		case soft_reset:
			if(motor_reset == 0){
					mottor_position = 3;
					rightMove = leftMove = 0;
					state = Motor_load;
					reset_flag = 0;
			}
			break;

		case Motor_load:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;	
			}
			if(GameMode1){
				M1steps = 0;
				if(TimerAttack <= 14){
					state = movement1;
				}				
			}
			if(GameMode2){
				state = GM2_Wait;
			}
			break;
		case movement1:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			M1steps++;
			if(TimerAttack < 0){
				state = soft_reset;
				break;
			}
			if(M1steps >= 768){
				M1steps = 0;
				state = movement2;
				i = 0;
			}
			break;
		case movement2:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			M1steps++;
			if(TimerAttack < 0){
				state = soft_reset;
				break;
			}
			if(M1steps >= 768){
				M1steps = 0;
				state = movement3;
				j = 0;
			}		
			break;
		case movement3:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			M1steps++;
			if(TimerAttack < 0){
				state = soft_reset;
				break;
			}
			if(M1steps >= 768){
				M1steps = 0;
				state = movement4;
				j = 0;
			}
			break;
		case movement4:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			M1steps++;
			if(TimerAttack < 0){
				state = soft_reset;
				break;
			}
			if(M1steps >= 768){
				M1steps = 0;
				state = movement1;
				i = 0;
			}
			break;
			
		case stop:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			if(TimerAttack > 15){
				state = Motor_load;
			}
			break;
			
		case Mode2_moveL:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			if(leftMove){
				M2steps++;
				if(M2steps >= small_move){
					M2steps = leftMove = 0;
					i = j = 0;
					state = lock_in;
				}
			}
		
			break;
			
		case Mode2_moveR:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			if(rightMove){
				M2steps++;
				if(M2steps >= small_move){
					M2steps = rightMove = 0;
					i = j = 0;
					state = lock_in;
				}
			}
			break;
		
		case lock_in:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			if(GameMode2 == 0){
				state = soft_reset;
				break;
			}
			if(locked_location == 0){
				if( GetBit(tmpA,7) && !(GetBit(tmpA,6)) && !(GetBit(tmpA,5))){
					if(mottor_position != 0){
						leftMove = 1;
						mottor_position--;
						state = Mode2_moveL;
					}
				}
				if( !(GetBit(tmpA,7)) && GetBit(tmpA,6) && !(GetBit(tmpA,5)) ){
					if(mottor_position != 7){
						rightMove = 1;
						mottor_position++;
						state = Mode2_moveR;
					}	
				}
				eeprom_write_byte((uint8_t*)50, mottor_position);					
			}
			
			break;
			
		
		case GM2_Wait:
			if( GetBit(tmpA,4) ){
				state = soft_reset;
				delay = i = j = 0;
				M1steps = 0;
				break;
			}
			if(delay >= 400){
				state = lock_in;
				delay = 0;
			}
			break;
		
		default:
			state = stop;
			break;
	}
	switch(state){
		case soft_reset:
			if(motor_reset < 0){
				++i;
				motor_reset++;
				PORTB = CW[i]; 
				if(i >= 8){
					i = 0;
				}
			}
			else if(motor_reset > 0){
				++j;
				motor_reset--;
				PORTB = CCW[j];
				if(j >= 8){
					j = 0;
				}
			}
		
		break;
		
		case Motor_load:
			break;
		case movement1:
			PORTB = CCW[i];
			motor_reset--;
			++i;
			if(i >=8){
				i = 0;
			}
			break;
		case movement2:
			PORTB = CW[j];
			motor_reset++;
			++j;
			if(j >= 8){
				j = 0;
			}
			break;
		case movement3:
			PORTB = CW[j];
			motor_reset++;
			++j;
			if(j >=8){
				j = 0;
			}
			break;
		case movement4:
			PORTB = CCW[i];
			motor_reset--;
			++i;
			if(i >= 8){
				i = 0;
			}
			break;	
		
		case stop:
			PORTB = 0;
			motor_reset = 0;
			i = j = 0;
			break;
			
		case Mode2_moveL:
			PORTB = CCW[i];
			motor_reset--;
			++i;
			if(i >= 8){
				i = 0;
			}		
			break;
		case Mode2_moveR:
			PORTB = CW[j];
			motor_reset++;
			++j;
			if(j >= 8){
				j = 0;
			}
			break;
		case lock_in:
			PORTB = 0;
			i = j = 0;
			break;
					
		break;
		
		case GM2_Wait:
		delay++;
		break;
			
		default:
			state = stop;
			i = j = 0;
			break;
		
	}
	
	return state;
	
}


#endif
