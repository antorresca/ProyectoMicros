#include <xc.h>
#define _XTAL_FREQ 8000000 //Frecuencia de reloj
#include "LibLCDXC8.h"

/*****Informacion de conexiones*****
 * Puerto D (RD0-RD7) para LCD
 * Puerto B (RB0-RB7) para Teclado matricial
*/

//Variables
int contador = 0;
int duty = 0;

//Prototipos
unsigned char LeerTeclado(void);

void main(void){
    //Configuracion de pines
    TRISB = 0;
    OSCCON = 0b11110110; //Frecuencia a 8MHz
    //Configuracion del LCD
    TRISD = 0;
    TRISE0 = 0;
    TRISE1 = 0;
    InicializaLCD();
    BorraLCD();
    //Fin de configuracion del LCD
    //Configuracion del Timer 0
    T0CON = 0b0000110;
    TMR0 = 49911; //Precarga para 1 segundo
    TMR0IF = 0;
    TMR0IE = 1;
    TMR0ON=1;
    GIE = 1;
    //Fin de configuracion del Timer 0
    //Configuracion para PWM
    TRISC2 = 0; //Canal de salida para PWM
    PR2 = 249; 
    CCPR1L = 0; //Para canal escogido ***PUEDE GENERAR PROBLEMAS***
    T2CON=0b00000000;
    CCP1CON=0b00001100;
    TMR2=0;
    TMR2ON=1;
    //Fin de cofiguracion para PWM 
    //Configuracion de comunicacion
    TRISC6 = 1;
    TRISC7 = 1;
    TXSTA = 0b00100000;
    RCSTA = 0b10010000;
    //Fin de configuracion de comunicacion    
    while(1){
        
    }
}

unsigned char LeerTeclado(void){
    //Algoritmo de barrido para teclado matricial
    while(RB4==1 && RB5==1 && RB6==1 && RB7==1);
    LATB=0b11111110;
    if(RB4==0) return '1';
    else if(RB5==0) return '2';
    else if(RB6==0) return '3';
    else if(RB7==0 & RC6==0) return '+';
    else if(RB7==0 & RC6==1) return '!';
    else{
    LATB=0b11111101;
    if(RB4==0) return '4';
    else if(RB5==0) return '5';
    else if(RB6==0) return '6';
    else if(RB7==0 & RC6==0) return '-';
    else if(RB7==0 & RC6==1) return '^';
    else{
    LATB=0b11111011;
    if(RB4==0) return '7';
    else if(RB5==0) return '8';
    else if(RB6==0) return '9';
    else if(RB7==0) return '/';
    else{
    LATB=0b11110111;
    if(RB4==0) return 'C';
    else if(RB5==0) return '0';
    else if(RB6==0) return '=';
    else if(RB7==0) return 'x';
    }
    }
    }
    return ' ';
}

void __interrupt() ISR(void){
    if(TMR0IF == 1){
        TMR0IF=0;
        TMR0 = 49911;
        contador += 1;
        if(contador == 20){
            SLEEP();
            while(1);
        }
    }
}