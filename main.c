#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include <math.h>
#include "lcd.h"
#include "avr.h"

#define MY_NULL 15
#define TRUE 1
#define FALSE 0
#define MIN_LEN 60000

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978


typedef enum NOTE_LENGTH
{
	whole, half, quarter, eighth
} NoteLength;

typedef struct NOTE
{
	int frequency;
	NoteLength noteLen;
	int timeHigh;
	int timeLow;  
} Note;

typedef struct SONG
{
	int tempo;
	int numberOfNotes;
	int wholeNoteLength; 
	int halfNoteLength; 
	int quarterNoteLength; 
	int eighthNoteLength; 
	int pitches[100];
	NoteLength duration[100]; 
} Song;




void playNote(int frequency, int duration)
{
	int period = 1000/frequency; 
	float t_high = period/ 2;
	float t_low = period/ 2;
	float k = (frequency/1000) * duration; 
	
	clr_lcd();
	char buf[17];
	sprintf(buf, "in %d", period);
	puts_lcd2(buf);
	wait_avr(2000);
	
	for(int i =0; i< k; ++i){
		SET_BIT(PORTB, 3);
		wait_avr(10);
		CLR_BIT(PORTB, 3);
		wait_avr(10);
	}	
}

int is_pressed(int r, int c){
	
	//##### NC ######
	DDRC= 0; // set everything to input
	PORTC = 0; // set to N/C
	
	//####  write weak 1 to c+4 ####
	SET_BIT(PORTC, c+4);
	
	// ######  write strong 0 to r #######
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	
	
	// check if button is pressed
	if(GET_BIT(PINC, c+4)) return 0;
	return 1;
}
// converts to actually number
unsigned char get_value(int key){
	switch(key){
		case 1:
		return 1;
		case 2:
		return 2;
		case 3:
		return 3;
		case 5:
		return 4;
		case 6:
		return 5;
		case 7:
		return 6;
		case 9:
		return 7;
		case 10:
		return 8;
		case 11:
		return 9;
		case 14:
		return 0;
		case 4:
		return 65; // capital A
		//case 8:
		//return 66; // capital B
		//case 16:
		//return 68; // capital D
		default:
		return MY_NULL;
	}
	
}

unsigned char get_key(){
	int r,c;
	for(r=0; r<4;++r){
		for(c=0; c<4;++c){
			if(is_pressed(r,c)){
				return get_value((r*4)+c+1);
			}
		}
	}
	return get_value(0);
}

void print(char * msg){
	clr_lcd();
	puts_lcd2(msg);
	wait_avr(1000);
}

void printTempo(int numberOfElementsInArray, int numberArr[4])
{
	char buf[17];
	clr_lcd();
	pos_lcd(0,0);
	puts_lcd2("Enter Tempo");
	for(int i = 0; i<numberOfElementsInArray; ++i)
	{
		pos_lcd(1, i+4); 
		sprintf(buf, "%d", numberArr[i]);
		puts_lcd2(buf); 
		wait_avr(10); 
	}
};

void calculateNoteDivisions(Song *song)
{
	song->wholeNoteLength = (MIN_LEN/song->tempo)*4;
	song->halfNoteLength = (MIN_LEN/song->tempo)*2;
	song->quarterNoteLength = (MIN_LEN/song->tempo);
	song->eighthNoteLength = MIN_LEN/(song->tempo*2);
}

void calculateTempo(int numberOfElementInArray, int numberArr[5], Song *song)
{
	song->tempo = 0; 
	
	int digit = numberOfElementInArray; 
	for(int i = 0; i<numberOfElementInArray; ++i)
	{
		song->tempo += (numberArr[i] * pow(10, digit-1)); 
		--digit; 
	}
	
	calculateNoteDivisions(song); 
}

void getTempo(Song *song)
{
	char c = MY_NULL; 
	int numberArr[4]; 
	int i = 0; 
	printTempo(i, numberArr);
	while(c != 'A'){ 
		c = get_key();
		if(c != MY_NULL && c != 'A')
		{
			numberArr[i] = c;
			++i;
			printTempo(i, numberArr);
			wait_avr(300); 
		}
		else if(c == 'A' || i == 4)
		{
			break;
		}
		wait_avr(10); 
	}
	calculateTempo(i, numberArr, song); 
}

void playMusic(Song s)
{
	for(int i = 0; i<s.numberOfNotes; ++i)
	{
		playNote(s.pitches[i], s.duration[i]);
	}
}


int main(void)
{
	ini_avr();
	ini_lcd();
    SET_BIT(DDRB, 3); // make B3 an output
    //Song song;
	//getTempo(&song); 
	
	
	clr_lcd();
	puts_lcd2("Play a song"); 
	
	
	//char buf[17];
	//sprintf(buf, "%d %d", song.halfNoteLength, song.eighthNoteLength);
	//puts_lcd2(buf); 
	//wait_avr(10100); 
	
	
	playNote(1000, 10); 
	
	
	while(TRUE){
		print("the after"); 
		wait_avr(10); 
	}
	
	
}

