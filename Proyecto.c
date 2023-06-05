#include <xc.h>
#define _XTAL_FREQ 8000000 //Frecuencia de reloj
#include "LibLCDXC8.h"

/*****Informacion de conexiones*****
 * Puerto D (RD0-RD7) para LCD 
 * Puerto B (RB0-RB7) para Teclado matricial
 * RA1 para ADC
 * RC6 y RC7 para comunicacion serial RS232
 * RC2 para motor
 * RC1 para Servo
 * -TM0- para bajo consumo
 * -INT0- para parada de emergencia
*/

//Variables
int contador = 0;
int duty = 0;
unsigned char Temp, Hum;

//Configuraciones
#pragma config PBADEN = OFF
#pragma 

#define _XTAL_FREQ 8000000 //Frecuencia de reloj
#define DATA_DIR TRISC2
#define DATA_IN RC2
#define DATA_OUT LATC2

//Prototipos
unsigned char LeerTeclado(void);
unsigned char LeerByte(void);
unsigned char LeerBit(void);
void Transmitir(unsigned char);
unsigned char Recibir(void);
void TransmitirDatos(unsigned int Ent1, unsigned int Ent2);

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
    //Configuracion del ADC
    ADCON0 = 0b00000001;
    ADCON1 = 0b00001100;
    ADCON2 = 0b10001000;
    //Fin de configuracion del ADC
    //Configuracion del Timer 0
    T0CON = 0b0000110;
    TMR0 = 49911; //Precarga para 1 segundo
    TMR0IF = 0;
    TMR0IE = 1;
    TMR0ON = 1;
    GIE = 1;
    //Fin de configuracion del Timer 0
    //Configuracion para PWM
    TRISC2 = 0; //Canal de salida para PWM
    PR2 = 249; 
    CCPR1L = 0; //Para canal escogido ***PUEDE GENERAR PROBLEMAS***
    T2CON = 0b00000000;
    CCP1CON = 0b00001100;
    TMR2 = 0;
    TMR2ON = 1;
    //Fin de cofiguracion para PWM 
    //Configuracion de comunicacion
    TRISC6 = 1;
    TRISC7 = 1;
    TXSTA = 0b00100000;
    RCSTA = 0b10010000;
    SPBRG = 12; 
    //Fin de configuracion de comunicacion    
    while(1){
        
    }
}

void LeerHT11(void) {
    //Por defecto el pin de comunicaci?n est? en alto, para iniciar la comunicaci?n se debe poner la l?nea de datos en bajo durante 18ms
    DATA_DIR = 0; //Configura el pin como salida, por defecto su valor de salida es 0
    __delay_ms(18); //Se esperan los 18ms
    DATA_DIR = 1; //Se reestablece el pin a entrada digital
    //Ahora se espera la respuesta del sensor
    while (DATA_IN == 1); //Tiempo en alto mientras el sensor responde
    __delay_us(120); //Pulso bajo, respuesta del sensor 80us, posteriormente pulso en alto de una duraci?n similar.
    while (DATA_IN == 1); //Tiempo en alto que dura hasta que el sensor toma control del canal de comunicaci?n
    //Recepci?n de datos
    Hum = LeerByte();
    LeerByte();
    Temp = LeerByte();
    LeerByte();
    LeerByte();
}

unsigned char LeerByte(void) {
    unsigned char res = 0, i;
    for (i = 8; i > 0; i--) {
        res = (res << 1) | LeerBit(); //Va moviendo los digitos del byte a la izquierta y a?adiendo los valores le?dos
    } //Comienza 00000000, lee 1, entonces 0000001, lee 0, entonces 00000010, lee 1, entonces 00000101, hasta llenar el byte
    return res;
}

unsigned char LeerBit(void) {
    unsigned char res = 0;
    while (DATA_IN == 0);
    __delay_us(13);
    if (DATA_IN == 1) res = 0; //Si el pulso dura menos de 30 us el bit es 0
    __delay_us(22);
    if (DATA_IN == 1) { // Sino, el bit es 1
        res = 1;
        while (DATA_IN == 1);
    }
    return res;
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

void Transmitir(unsigned char BufferT) {
    while (TRMT == 0);
    TXREG = BufferT;
}

unsigned char Recibir(void){
    while(RCIF==0);
    return RCREG;
}

void TransmitirDatos(unsigned int Ent1, unsigned int Ent2) {
    Transmitir(Ent1); //Falta colocar que se tiene que tran$mitir
}

void __interrupt() ISR(void){
    if(TMR0IF == 1){
        TMR0IF = 0;
        TMR0 = 49911;
        contador += 1;
        if(contador == 20){
            SLEEP();
            while(1);
        }
    }
    if(INT0IF == 1){
        INT0IF = 0;
        SLEEP();
        while(1);
    }
}