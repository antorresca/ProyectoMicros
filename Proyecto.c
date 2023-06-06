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
unsigned char Temp, Humedad;
unsigned char cara[]={
    0b00000000,
    0b00001010,
    0b00001010,
    0b00001010,
    0b00000000,
    0b00011111,
    0b00001110,
    0b00000000
    };
unsigned char Tecla;
unsigned char Cont = 6;
unsigned char ing = 0;
int c = 1000000;

//Configuraciones
#pragma config FOSC=INTOSC_EC
#pragma config PBADEN = OFF
#pragma config WDT = OFF
#pragma config LVP=OFF

#define _XTAL_FREQ 8000000 //Frecuencia de reloj
#define DATA_DIR TRISC0
#define DATA_IN RC0
#define DATA_OUT LATC0

//Prototipos
unsigned char LeerTeclado(void);
unsigned char LeerByte(void);
unsigned char LeerBit(void);
void Transmitir(unsigned char);
unsigned char Recibir(void);
void TransmitirDatos(unsigned int Ent1, unsigned int Ent2);
void LeerHT11(void);
void Velocidad(unsigned int val);

void main(void){
    //Configuracion de pines
    TRISC = 0;
    OSCCON = 0b11110110; //Frecuencia a 8MHz
    //Configuracion del LCD
    TRISD = 0;
    TRISE0 = 1;
    TRISE2 = 0;
    TRISE1 = 0;
    InicializaLCD();
    BorraLCD();
    //Fin de configuracion del LCD
    //Configuracion del ADC
    ADCON0 = 0b00000001;
    ADCON1 = 0b00001100;
    ADCON2 = 0b10001000;
    //Fin de configuracion del ADC
    //Configuración matricial
    TRISB=0b11110000;
    RBPU=0;
    RBIF=0;
    RBIE=1;
    //Fin de configuracion matricial
    //Configuracion del Timer 0
    T0CON=0b00000011;//No habilita timer0, 16 bits de resolucion, reloj interno
    TMR0IF=0;// apaga bandera
    TMR0=64911; // valor pre carga
    TMR0IE=1; //Habilita la interrupcion 
    GIE=1; //habilita interrupciones globales
    TMR0ON=1;
    //Fin de configuracion del Timer 0
    //Configuracion para PWM
    TRISC2 = 0; //Canal de salida para PWM
    PR2 = 125; 
    CCPR1L = 0; //Para canal escogido ***PUEDE GENERAR PROBLEMAS*** (PR2+1)*D
    T2CON = 0b00000010;
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
    //Sensor
    TRISC0= 1;
    UTRDIS = 1;
    USBEN = 0;
    //Protocolo de inicio
    __delay_ms(1000); //Retraso para evitar errores
    BorraLCD(); 
    NuevoCaracter(0,cara);
    DireccionaLCD(0x82);
    EscribeLCD_c(0);
    MensajeLCD_Word("Bienvenido");
    EscribeLCD_c(0);
    DireccionaLCD(0xC7);
    __delay_ms(1500);
    DireccionaLCD(0x80); //Colocar el cursor en la primera posicion de primera fila
    MensajeLCD_Word("                "); //Mandar mensaje vacio para limpiar
    DireccionaLCD(0x80);
    MensajeLCD_Word("Password:");
    while(ing!=Cont){
        DireccionaLCD(0xC1);
        MensajeLCD_Word("                ");
    }
    DireccionaLCD(0x80);
    MensajeLCD_Word("                ");
    DireccionaLCD(0xC1);
    MensajeLCD_Word("                ");
    __delay_ms(500);
    while(1){   
        __delay_ms(500);
        LeerHT11();        
        TransmitirDatos(0, 0);
        Velocidad(Temp);
    }
}

void Velocidad(unsigned int val){
    if (val < 22) {
        CCPR1L = 0;
    } else if (val >= 22 && val < 25) {
        CCPR1L = 19;
    } else if (val >= 25 && val < 28) {
        CCPR1L = 38;
    } else if (val >= 28 && val < 31) {
        CCPR1L = 57;
    } else if (val >= 31 && val < 34) {
        CCPR1L = 76;
    } else if (val >= 34 && val < 37) {
        CCPR1L = 95;
    } else if (val >= 37 && val < 40) {
        CCPR1L = 113;
    } else if (val >= 40) {
        CCPR1L = 126;
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
    Humedad = LeerByte();
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
    switch (Tecla){
        case 1:
            Tecla='1';
            break;
        case 2:
            Tecla='2';
            break;
        case 3: 
            Tecla='3';
            break;
        case 4:
            Tecla='+';
            break;
        case 5:
            Tecla='4';
            break;
        case 6:
            Tecla ='5';
            break;
        case 7:
            Tecla='6';
            break;
        case 8:
            Tecla='-';
            break;
        case 9: 
            Tecla='7';
            break;
        case 10:
            Tecla='8';
            break;
        case 11:
            Tecla='9';
            break;
        case 12:
            Tecla='X';
            break;
        case 13:
            Tecla='B';
            break;
        case 14:
            Tecla='0';
            break;
        case 15:
            Tecla='=';
            break;
        case 16:
            Tecla='/';
            break;
    }
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
    unsigned int n = Ent1 * 10 + Ent2, TempC = Temp, HumedadC = Humedad;
    unsigned int Simb = 67;
    BorraLCD();
    switch (n) {
        case 00://Celsius
            TempC = Temp;
            Simb = 67; //C
            break;
        case 01://Kelvin
            TempC = Temp + 273;
            Simb = 75; //K
            break;
        case 10://Rankine
            TempC = Temp * 9 / 5 + 492;
            Simb = 82; //R
            break;
        case 11://Fahrenheit
            TempC = Temp * 9 / 5 + 32;
            Simb = 70; //F
            break;
    }
    Transmitir('T');
    Transmitir('e');
    Transmitir('m');
    Transmitir('p');
    Transmitir(':');
    Transmitir(' ');

    MensajeLCD_Word("Temp:");
    if (TempC / 100 > 0) {
        Transmitir(TempC / 100 + 48);
        EscribeLCD_c(TempC / 100 + 48);
        TempC = TempC % 100;
    }
    Transmitir(TempC / 10 + 48);
    Transmitir(TempC % 10 + 48);
    Transmitir(167);
    Transmitir(Simb);
    Transmitir(' ');
    Transmitir('\n');
    Transmitir('H');
    Transmitir('u');
    Transmitir('m');
    Transmitir(':');
    Transmitir(' ');
    Transmitir(Humedad / 10 + 48);
    Transmitir(Humedad % 10 + 48);
    Transmitir(' ');
    Transmitir('%');
    Transmitir('\n');
    Transmitir(' ');
    //Imprimir en LCD
    EscribeLCD_c(TempC / 10 + 48);
    EscribeLCD_c(TempC % 10 + 48);
    EscribeLCD_c(Simb);
    DireccionaLCD(192);
    MensajeLCD_Word("Hum:");
    EscribeLCD_c(Humedad / 10 + 48);
    EscribeLCD_c(Humedad % 10 + 48);
    EscribeLCD_c('%');

}

void __interrupt() ISR(void){
    if(TMR0IF == 1){
        TMR0IF = 0;
        TMR0 = 64911;
        contador += 1;
        if(RE0 == 1){
            CCP1CON = 0 ;
            __delay_ms(100);
            SLEEP();
            while(1);
        }
    }
    if(RBIF==1){
        if(PORTB!=0b11110000){
            Tecla=0;
            LATB=0b11111110;
            if(RB4==0) Tecla=16;
            else if(RB5==0) Tecla=12;
            else if(RB6==0) Tecla=8;
            else if(RB7==0) Tecla=4;
            else{
                LATB=0b11111101;
                if(RB4==0) Tecla=15;
                else if(RB5==0) Tecla=11;
                else if(RB6==0) Tecla=7;
                else if(RB7==0) Tecla=3;
                else{
                    LATB=0b11111011;
                    if(RB4==0) Tecla=14;
                    else if(RB5==0) Tecla=10;
                    else if(RB6==0) Tecla=6;
                    else if(RB7==0) Tecla=2;
                    else{
                        LATB=0b11110111;
                        if(RB4==0) Tecla=13;
                        else if(RB5==0) Tecla=9;
                        else if(RB6==0) Tecla=5;
                        else if(RB7==0) Tecla=1;
                    }
                }
            }
            LATB=0b11110000;              
            ing = Tecla;
            LeerTeclado();
            EscribeLCD_c(Tecla);
        }
        RBIF=0;        
        __delay_ms(300);
         
    }
}