#define MINUTE 60000  // Number of milliseconds in a minute
#define F_CPU 8000000UL  // CPU frequency in Hz
#include <avr/io.h>			
#include <avr/interrupt.h>
#include <util/delay.h>	
#include<stdio.h>
#include<stdlib.h>		

#define LCD_Data_Dir DDRB	
#define LCD_Command_Dir DDRA	
#define LCD_Data_Port PORTB	
#define LCD_Command_Port PORTA		
#define RS PINA2				
#define RW PINA3				
#define EN PINA4				
#define Hour_Button PINC0
#define Minute_Button PINC1
#define Year_Button PINC2
#define Month_Button PINC3
#define Day_Button PINC4
#define Up_Button PINC5
#define Down_Button PINC6
#define Alarm_Button PINC7

volatile uint8_t tot_overflow;

void LCD_Command(unsigned char cmnd);
void LCD_Char (unsigned char char_data);
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();
void increment_minutes(char current[], int pos, int flag);
void decrement_minutes(char current[], int pos);
void increment_hours(char current[], int pos, int flag);
void decrement_hours(char current[], int pos);
void increment_days(char current[], int pos, int flag);
void decrement_days(char current[], int pos);
void increment_months(char current[], int pos, int flag);
void decrement_months(char current[], int pos);
void increment_years(char current[], int pos, int flag);
void decrement_years(char current[], int pos); 
int check_month(char current[], int pos);
void timer1_init();
void set_alarm(char time_only[]);


// Setting an initial value for date and time for when the program starts running

char *current_date = "Day 01/02/23";
char *current_time = "Time: 05:59";


// flags for lcd state and alarm state and int for alarm on/off (initially off)
int state_lcd = 0;
int alarm = 0;
char *alarm_time = "Time: 00:00";


// Interrupt for timer1
ISR(TIMER1_OVF_vect)
{
	// keep track of number of overflows
	tot_overflow++;
	
	// 114 overflows = 60 seconds approximately on the current clock frequency (8 MHZ with 64 prescaler)
	if (tot_overflow >= 114) 
	{
		// increment time
		increment_minutes(current_time, 9, 1);
		if (state_lcd == 0) {
			LCD_Clear();
			LCD_String(current_date);
			LCD_Command(0xC0);
			LCD_String(current_time);
		}
		// if alarm is on and current time is equal to the alarm time set PIND0 to high which is connected to buzzer (LED)
		if (alarm == 1) {
			if (if_alarm(alarm_time) == 1) {
				LCD_Clear();
				LCD_String("ALARM!!!!");
				PORTD |= 1 << PIND0;
				_delay_ms(300);
				LCD_Clear();
				LCD_String(current_date);
				LCD_Command(0xC0);
				LCD_String(current_time);
				PORTD &= ~(1 << PIND0);
			}
		}
		
		tot_overflow = 0;   // reset overflow counter
	}
}


int main()
{
	// setting all of the pins on the C direction register to input
	DDRC = 0x00;
	// setting the pin connected to the buzzer (LED) to output and initializing pin to low
	DDRD |= 1 << PIND0;
	PORTD &= ~(1 << PIND0);
	// initializing LCD
	LCD_Init();			
	// initializing timer1
	timer1_init();
	
	// printing the current date and time
	LCD_String(current_date);	
	LCD_Command(0xC0);		
	LCD_String(current_time);	
	
	while (1) {
		state_lcd = 0;
		// if minute button is pressed go into the time state on the lcd to freely increment and decrement time and then breaking out of the state if minute button is pressed again
		if ((PINC & (1 << Minute_Button))) {
			_delay_ms(50);
			char time_only[6];
			for (int i = 0; i <= 4; i++) {
				time_only[i] = current_time[i + 6];
			}
			time_only[5] = '\0';
			LCD_Clear();
			LCD_String(time_only);
			state_lcd = 1;
			while (1) {
				if (PINC & (1 << Up_Button)) {
					increment_minutes(time_only, 3, 0);
					increment_minutes(current_time, 9, 0);
					LCD_Clear();
					LCD_String(time_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Down_Button)) {
					decrement_minutes(time_only, 3);
					decrement_minutes(current_time, 9);
					LCD_Clear();
					LCD_String(time_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Minute_Button)) {
					LCD_Clear();
					LCD_String(current_date);
					LCD_Command(0xC0);
					LCD_String(current_time);
					_delay_ms(50);
					break;
				}
			}
			
		}
		
		// if hour button is pressed go into the time state on the lcd to freely increment and decrement time and then breaking out of the state if hour button is pressed again
		if (PINC & (1 << Hour_Button)) {
			_delay_ms(50);
			char time_only[6];
			for (int i = 0; i <= 4; i++) {
				time_only[i] = current_time[i + 6];
			}
			time_only[5] = '\0';
			LCD_Clear();
			LCD_String(time_only);
			state_lcd = 1;
			while (1) {
				if (PINC & (1 << Up_Button)) {
					increment_hours(time_only, 0, 0);
					increment_hours(current_time, 6, 0);
					LCD_Clear();
					LCD_String(time_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Down_Button)) {
					decrement_hours(time_only, 0);
					decrement_hours(current_time, 6);
					LCD_Clear();
					LCD_String(time_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Hour_Button)) {
					LCD_Clear();
					LCD_String(current_date);
					LCD_Command(0xC0);
					LCD_String(current_time);
					_delay_ms(50);
					break;
				}
			}
		}
		// if day button is pressed go into the date state on the lcd to freely increment and decrement date and then breaking out of the state if day button is pressed again
		if (PINC & (1 << Day_Button)) {
			_delay_ms(50);
			char date_only[9];
			for (int i = 0; i <= 7; i++) {
				date_only[i] = current_date[i + 4];
			}
			date_only[8] = '\0';
			LCD_Clear();
			LCD_String(date_only);
			state_lcd = 1;
			while (1) {
				if (PINC & (1 << Up_Button)) {
					increment_days(date_only, 0, 0);
					increment_days(current_date, 4, 0);
					LCD_Clear();
					LCD_String(date_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Down_Button)) {
					decrement_days(date_only, 0);
					decrement_days(current_date, 4);
					LCD_Clear();
					LCD_String(date_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Day_Button)) {
					LCD_Clear();
					LCD_String(current_date);
					LCD_Command(0xC0);
					LCD_String(current_time);
					_delay_ms(50);
					break;
				}
			}
		}
		
		// if month button is pressed go into the date state on the lcd to freely increment and decrement date and then breaking out of the state if month button is pressed again
		if (PINC & (1 << Month_Button)) {
			_delay_ms(50);
			char date_only[9];
			for (int i = 0; i <= 7; i++) {
				date_only[i] = current_date[i + 4];
			}
			date_only[8] = '\0';
			LCD_Clear();
			LCD_String(date_only);
			state_lcd = 1;
			while (1) {
				if (PINC & (1 << Up_Button)) {
					increment_months(date_only, 3, 0);
					increment_months(current_date, 7, 0);
					LCD_Clear();
					LCD_String(date_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Down_Button)) {
					decrement_months(date_only, 3);
					decrement_months(current_date, 7);
					LCD_Clear();
					LCD_String(date_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Month_Button)) {
					LCD_Clear();
					LCD_String(current_date);
					LCD_Command(0xC0);
					LCD_String(current_time);
					_delay_ms(50);
					break;
				}
			}
		}
	
		// if year button is pressed go into the date state on the lcd to freely increment and decrement date and then breaking out of the state if year button is pressed again
		if (PINC & (1 << Year_Button)) {
			_delay_ms(50);
			char date_only[9];
			for (int i = 0; i <= 7; i++) {
				date_only[i] = current_date[i + 4];
			}
			date_only[8] = '\0';
			LCD_Clear();
			LCD_String(date_only);
			state_lcd = 1;
			while (1) {
				if (PINC & (1 << Up_Button)) {
					increment_years(date_only, 6, 0);
					increment_years(current_date, 10, 0);
					LCD_Clear();
					LCD_String(date_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Down_Button)) {
					decrement_years(date_only, 6);
					decrement_years(current_date, 10);
					LCD_Clear();
					LCD_String(date_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Year_Button)) {
					LCD_Clear();
					LCD_String(current_date);
					LCD_Command(0xC0);
					LCD_String(current_time);
					_delay_ms(50);
					break;
				}
			}
		}
		// if alarm button is pressed go into the alarm state on the lcd to freely increment and decrement alarm time and then setting the alarm to 1 and breaking out of the state if alarm button is pressed again
		if (PINC & (1 << Alarm_Button)) {
			_delay_ms(50);
			char time_only[] = "00:00";
			LCD_Clear();
			LCD_String(time_only);
			while (1) {

				if (PINC & (1 << Hour_Button)) {
					increment_hours(time_only, 0, 0);
					LCD_Clear();
					LCD_String(time_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Minute_Button)) {
					increment_minutes(time_only, 3, 0);
					LCD_Clear();
					LCD_String(time_only);
					_delay_ms(50);
				}
				if (PINC & (1 << Alarm_Button)) {
					alarm = 1;
					set_alarm(time_only);
					LCD_Clear();
					LCD_String(current_date);
					LCD_Command(0xC0);
					LCD_String(current_time);
					_delay_ms(50);
					break;
				}
			}
		}
	}

}



// function for sending a command to the LCD by setting the RS and RW to 0
void LCD_Command(unsigned char cmnd) {
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1 << RS);	
	LCD_Command_Port &= ~(1 << RW);	
	LCD_Command_Port |= (1 << EN);	
	_delay_us(1);
	LCD_Command_Port &= ~(1 << EN);
	_delay_ms(3);
}

// function for sending a character to the LCD by setting RS to 1 (data mode) and RW to 0
void LCD_Char(unsigned char char_data) {
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1 << RS);	
	LCD_Command_Port &= ~(1 << RW);	
	LCD_Command_Port |= (1 << EN);	
	_delay_us(1);
	LCD_Command_Port &= ~(1 << EN);
	_delay_ms(1);
}

// function for initializing LCD display
void LCD_Init(void) {
	LCD_Command_Dir = 0xFF;		
	LCD_Data_Dir = 0xFF;		
	_delay_ms(20);			
	
	LCD_Command(0x38);		// Initialization of 16X2 LCD in 8bit mode 
	LCD_Command(0x0C);		// Display ON Cursor OFF 
	LCD_Command(0x06);		// Auto Increment cursor 
	LCD_Command(0x01);		// Clear display
	LCD_Command(0x80);		// Cursor at home position
}

// function for sending a string to the LCD by sending char by char in the array
void LCD_String(char *str)	{
	int i;
	for(i = 0;str[i] != 0; i++)		
	{
		LCD_Char (str[i]);
	}
}

// function to send a string to the LCD with a specific position
void LCD_String_xy(char row, char pos, char *str) {
	if (row == 0 && pos < 16)
	LCD_Command((pos & 0x0F) | 0x80);	
	else if (row == 1 && pos < 16)
	LCD_Command((pos & 0x0F) | 0xC0);	
	LCD_String(str);		
}

// function for clearing LCD
void LCD_Clear() {
	LCD_Command(0x01);		
	LCD_Command(0x80);		// cursor at home position 
}

// function for incrementing minutes from time string by typecasting the chars at minutes position to int and subtracting '0' from it to get the true int value for them and then doing the required operations and then adding '0' and assign them to their positions again
void increment_minutes(char current[], int pos, int flag) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y++;
	if (y % 10 == 0) {
		x++;
		y = 0;
	}
	if (x == 6) {
		x = 0;
		y = 0;
		if (pos == 9 && flag == 1) {
			increment_hours(current_time, 6, 1);
		}
		
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for decrementing minutes from time string
void decrement_minutes(char current[], int pos) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y--;
	if (y == -1) {
		x--;
		y = 9;
	}
	if (x == -1) {
		x = 5;
		y = 9;
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for incrementing hours from time string
void increment_hours(char current[], int pos, int flag) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y++;
	if (y % 10 == 0) {
		x++;
		y = 0;
	}
	if (x == 2 && y == 4) {
		x = 0;
		y = 0;
		if (pos == 6 && flag == 1) {
			increment_days(current_date, 4, 1);
		}
		
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for decrementing hours from time string
void decrement_hours(char current[], int pos) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y--;
	if (y == -1) {
		x--;
		y = 9;
	}
	if (x == -1) {
		x = 2;
		y = 3;
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}


// function for incrementing days from date string
void increment_days(char current[], int pos, int flag) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y++;
	int check = check_month(current, pos + 3);
	if (y % 10 == 0) {
		x++;
		y = 0;
	}
	
	// if month is 31 days
	if (check == 1) {
		if (y == 2 && x == 3) {
			x = 0;
			y = 1;
			if (pos == 4 && flag == 1) {
				increment_months(current_date, 7, 1);
			}
		}
	}
	// if month is 30 days
	else if (check == 0) {
		if (y == 1 && x == 3) {
			x = 0;
			y = 1;
			if (pos == 4 && flag == 1) {
				increment_months(current_date, 7, 1);
			}
		}
	}
	// if month is February
	else {
		if (y == 9 && x == 2) {
			x = 0;
			y = 1;
			if (pos == 4 && flag == 1) {
				increment_months(current_date, 7, 1);
			}
		}
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for decrementing days from date string
void decrement_days(char current[], int pos) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y--;
	int check = check_month(current, pos + 3);

	if (y == -1) {
		y = 9;
		x--;
	}
	
	if (x == 0 && y == 0) {
		// if month is 31 days
		if (check == 1) {
			x = 3;
			y = 1;
		}
		// if month is 30 days
		else if (check == 0) {
			x = 3;
			y = 0;
		}
		// if month is February
		else {
			x = 2;
			y = 8;
		}
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for incrementing months from date string
void increment_months(char current[], int pos, int flag) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y++;
	if (y % 10 == 0) {
		x++;
		y = 0;
	}
	if (x == 1 && y == 3) {
		x = 0;
		y = 1;
		if (pos == 7 && flag == 1) {
			increment_years(current_date, 10, 1);
		}
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for decrementing months from date string
void decrement_months(char current[], int pos) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y--;

	if (y == -1) {
		x--;
		y = 9;
	}
	if (x == 0 && y == 0) {
		x = 1;
		y = 2;
	}
	
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for incrementing years from date string
void increment_years(char current[], int pos, int flag) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y++;
	if (y % 10 == 0) {
		x++;
		y = 0;
	}
	if (x == 10) {
		x = 0;
		y = 0;
		if (pos == 10 && flag == 1) {
			current = "Sun 01/01/23";
			current_time = "Time 00:00";
		}
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// function for decrementing years from date string
void decrement_years(char current[], int pos) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	y--;
	if (y == -1) {
		x--;
		y = 9;
	}
	if (x == -1) {
		x = 0;
		y = 0;
	}
	x = x + '0';
	y = y + '0';
	current[pos] = x;
	current[pos + 1] = y;
}

// returns 1 for 30 day months, 0 for 30 day months and -1 for February
int check_month(char current[], int pos) {
	int x = (int) (current[pos]);
	int y = (int) (current[pos + 1]);
	x = x - '0';
	y = y - '0';
	
	if (y == 2 && x == 0) {
		return -1;
	}
	else if (((y == 4) && (x == 0)) | ((y == 6) && (x == 0)) | ((y == 9) && (x == 0)) | ((y == 1) && (x == 1))) {
		return 0;
	}
	else {
		return 1;
	}
}

// function for initializing timer1
void timer1_init()
{
	// set up timer with prescaler = 8
	TCCR1B |= (1 << CS11);
	
	// initialize counter
	TCNT1 = 0;
	
	// enable overflow interrupt
	TIMSK |= (1 << TOIE1);
	
	// enable global interrupts
	sei();
	
	// initialize overflow counter variable
	tot_overflow = 0;
}

// function for assigning alarm_time string to passed string
void set_alarm(char time_only[]) {
	for (int i = 6; i <= 10; i++) {
		alarm_time[i] = time_only[i - 6];
	}
}

// function for checking alarm on or off
int if_alarm(char alarm_time[]) {
	int equality = 1;
	for (int i = 6; i <= 10; i++) {
		if (alarm_time[i] != current_time[i]) {
			equality = 0;
		}
	}
	return equality;
}
