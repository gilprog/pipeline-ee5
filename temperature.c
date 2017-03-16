/*
 * File:   temperature.c
 * Author: Bernd
 *
 * Created on 16 maart 2017, 21:00
 */


#include <xc.h>
#include "temperature.h"

const unsigned char pipe_ascii[] = "Pipe";
const unsigned char ambient_ascii[] = "Amb.";
signed int plus_pipe;
signed int minus_pipe;
signed int plus_ambient;
signed int minus_ambient;
signed int temp_pipe;
signed int temp_ambient;
unsigned int currentChannel = PLUS_PIPE;    //defines which channel will be converted by the ADC (plus or minus terminal from temperature circuit))
unsigned char temp_display_message[] = "Pipe temp =      \r\n";
unsigned char asciiTemp[] = {' ',' ',' ',' ',' ',};

void initADC(void){
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    PORTAbits.RA0 = 0;
    PORTAbits.RA1 = 0;
    
    PORTBbits.RB0 = 0;
    TRISBbits.TRISB0 = 1;
    PORTBbits.RB1 = 0;
    TRISBbits.TRISB1 = 1;
	//PORTB = 0x00;
    //TRISB = 0x00;
    ANCON0 = 0xFC;  //Pin AN0 and AN1 are analog, the rest digital
    ANCON1 = 0x1C; //pin 8-12 are digital
    ADCON0 = 0x00;  //VSS, VDD, channel 00 (AN0), GO/DONE = 0 (idle), ADON = 1    clear ADCON0 to select channel 0 (AN0)
	ADCON1 = 0b10111110;    //Right justified, normal ADC operation, slowest acquisition time and conversion clock
  	ADCON0bits.ADON = 0x01;//Enable A/D module
    
    PIR1bits.ADIF = 0;  //clear ADC interrupt flag
    PIE1bits.ADIE = 1;  //enable ADC interrupt
    
    //IPR1bits.ADIP = 0;  //ADC is low priority
    ADCON0bits.GO_DONE = 1;
    
}

void fillInTemp(char pipe_or_ambience){
    signed int temp;
    if(pipe_or_ambience == PIPE){
        for(int i = 0; i<4; i++){
            temp_display_message[i] = pipe_ascii[i];   
        }
        temp = temp_pipe;
    }
    else{
        for(int i = 0; i<4; i++){
            temp_display_message[i] = ambient_ascii[i];   
        }
        temp = temp_ambient;
    }
    //int temp_0 = 25;
    //char temp = minus;
    if(temp < 0){
        asciiTemp[0] = '-';
        temp = - temp;
    }
    else{
        asciiTemp[0] = ' ';
    }
    asciiTemp[1] =(char) (temp/100 + 48); 
    asciiTemp[2] =(char) (temp % 100 / 10 + 48);
    asciiTemp[3] = (char) (temp % 10 + 48);
    
    
    // Fill in the tempDisplayCommand
    for(char i = 12; i<17; i++){
        temp_display_message[i] = asciiTemp[i-12];
    }
    
}
signed int calculateTemp(int plus, int minus){
    signed int temp; 
    temp = plus - minus;
    temp = (int)((double)(temp) / 1024 * 2750 / 10);
    
    return temp;
}

void makeTempMessage(char pipe_or_ambient){
    if(pipe_or_ambient == PIPE){
        temp_pipe = calculateTemp(plus_pipe, minus_pipe);
        fillInTemp(PIPE);
    }
    else{
        temp_ambient = calculateTemp(plus_ambient, minus_ambient);
        fillInTemp(AMBIENT);
    }
    
}
void temperature_interrupt(void){
//Interrupt AD
    if(PIR1bits.ADIF == 1)
    {
        PIR1bits.ADIF = 0;
        switch(currentChannel){
            case PLUS_PIPE:
                plus_pipe = ADRES;
                ADCON0bits.CHS = 0b1000;  //next channel is MINUS_PIPE = CH08
                currentChannel = MINUS_PIPE;
                break;
            case MINUS_PIPE:
                minus_pipe = ADRES;
                ADCON0bits.CHS = 0b0001;  //next channel is PLUS_AMBIENT = CH01
                currentChannel = PLUS_AMBIENT;
                break;
            case PLUS_AMBIENT:
                plus_ambient = ADRES;
                ADCON0bits.CHS = 0b0000;  //next channel is MINUS_AMBIENT = CH0
                currentChannel = MINUS_AMBIENT;
                break;
            case MINUS_AMBIENT:
                minus_ambient = ADRES;
                ADCON0bits.CHS = 0b1001;  //next channel is PLUS_PIE = CH09
                currentChannel = PLUS_PIPE;
                break;
        }
        ADCON0bits.GO_DONE = 1;
        
    }
}