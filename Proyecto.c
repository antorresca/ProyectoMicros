#include <xc.h>
#define _XTAL_FREQ 8000000 //Frecuencia de reloj
#include "LibLCDXC8.h"

/*****Informacion de conexiones*****
 * Puerto D (RD0-RD7) para LCD
 * Puerto B (RB0-RB7) para Teclado matricial
*/

//Prototipos
unsigned char LeerTeclado(void);

void main(void){
    //Configuracion de pines
    TRISB = 0;
    TRISD = 0;
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