/*
 * File:   uart.c
 * Author: Bernd
 *
 * Created on 16 maart 2017, 16:31
 */


#include <xc.h>
#include "uart.h"
#include "delay.h"


char isCommandSent = TRUE;
unsigned char *currentMessagePointer;
unsigned char uart_receive_buffer[BUFFER_SIZE];
unsigned int uart_receive_buffer_index = 0;
unsigned int last_received_message_index = 0;
unsigned char last_uart_message[MAX_MESSAGE_SIZE];

void initUART1(void){
    //Init the UART1
    
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;
    //5 steps: see datasheet page 355
    TXSTA1bits.BRGH = 1;
    BAUDCON1bits.BRG16 = 1;
    /////1/////
    //Baud rate calculations:
    //SPBRGHx:SPBRGx =  ((Fosc/Desired Baud Rate)/4) - 1
    //Fosc = 8MHz
    //Desired Baud Rate = 57600
    //SPBRGHx:SPBRGx = 34
    //Datasheet page: 349-350
    SPBRGH1 = 0;
    SPBRG1 = 34;
    
    /////2/////
    //SYNC is default 0: Datasheet page 346
    TXSTA1bits.SYNC = 0;
    RCSTA1bits.SPEN = 1;
    //Datasheet page 347
    
    /////3/////
    PIE1bits.TXIE = 0;
    PIE1bits.RC1IE = 1;
    //Datasheet page 113
    
    /////4/////
    // page 357
    RCSTA1bits.CREN = 1;
}
void UARTReceive(char on_or_off){
    if(on_or_off == ON){
        RCSTA1bits.CREN = 1;
    }
    else{
        RCSTA1bits.CREN = 0;
    }
}
void sendUARTMessage(unsigned char *newMessagePointer){
    // Check if previous message is sent  
    //Change the current message
    currentMessagePointer = newMessagePointer;
    //The new message isn't sent yet

    //If TXEN is set to 1 --> TX1IF is set implicitly
    //PIE1bits.TXIE = 1;
    last_received_message_index = uart_receive_buffer_index;
    TXSTA1bits.TXEN = 1;
//    delay_ms(100);
    while(*currentMessagePointer != '\0'){
        if(PIR1bits.TXIF == 1){
            //Prepare next byte for sending
            TXREG1 = *currentMessagePointer;
            //Go to the next byte
            currentMessagePointer += 1;
            delay_ms(1);
        }
    }
}
void getLastReceivedMessage(){
    unsigned char index = 0;
    while(uart_receive_buffer_index != last_received_message_index){
        last_uart_message[index] = uart_receive_buffer[last_received_message_index];
        last_received_message_index++;
        index++;
        if(last_received_message_index > BUFFER_SIZE){
            last_received_message_index = 0;
        }
    }
    last_uart_message[index] = uart_receive_buffer[last_received_message_index];
    last_uart_message[index + 1] = '\0';
}
void clearUARTReceiveBuffer(void){
    for(int i = 0; i<BUFFER_SIZE; i++){
        uart_receive_buffer[i] = '\0';
    }
    uart_receive_buffer_index= 0 ;
}
    
void uart_interrupt(void){
    //Interrupt for the receiving part
    if(PIR1bits.RC1IF == 1){
        PIR1bits.RC1IF = 0;
        // Save the received byte in the receive buffer
        uart_receive_buffer[uart_receive_buffer_index] = RCREG1;
        uart_receive_buffer_index += 1;
        // If there is overflow, clear the buffer
        // this is only for emergencies, buffer needs to be cleared with
        // clearUARTReceiveBuffer
        if(uart_receive_buffer_index > BUFFER_SIZE){
            uart_receive_buffer_index = 0;
        }
    }
    
    
}
