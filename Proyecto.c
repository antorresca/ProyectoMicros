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
unsigned char Temp;
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
unsigned char Cont = 6; //Numero de posicion
unsigned char ing[4];
int c = 0;
unsigned int a = 125;
unsigned int teclaIf = 0;//bandera para deshabilitar el teclado 
unsigned int teclaRecibidaIf = 0;//bandera para deshabilitar el teclado 
//Configuraciones
#pragma config FOSC=INTOSC_EC
#pragma config PBADEN = OFF
#pragma config WDT = OFF
#pragma config LVP=OFF

#define _XTAL_FREQ 8000000 //Frecuencia de reloj
#define DATA_DIR TRISA5
#define DATA_IN RA5
#define DATA_OUT LATA5

//Prototipos
unsigned char LeerTeclado(void);
unsigned char LeerByte(void);
unsigned char LeerBit(void);
void Transmitir(unsigned char);
unsigned char Recibir(void);
void TransmitirDatos(unsigned int Ent1, unsigned int Ent2);
void LeerHT11(void);
void Velocidad(unsigned int val);
void Movimiento(void);
unsigned int ConvertirUnidades(unsigned char canal);
int Password(char* pass);

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
    TRISA = 0b00100001;
    ADCON0 = 0b00000001;
    ADCON1 = 0b00001100;
    ADCON2 = 0b00001000;
    //Fin de configuracion del ADC q    
    //Configuraci�n matricial
    TRISB=0b11110000;
    RBPU=0;
    RBIF=0;
    RBIE=1;
    //Fin de configuracion matricial
    //Configuracion del Timer 0
    T0CON=0b00000011;//No habilita timer0, 16 bits de resolucion, reloj interno
    TMR0IF=0;// apaga bandera
    TMR0=64286; // valor pre carga
    TMR0IE=1; //Habilita la interrupcion 
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
    TRISA5= 1;
    UTRDIS = 1;
    USBEN = 0;
    //Servomotor
    TRISC1=0;      
    TMR1=60536;        //Precarga del Timer1 que asegura los 20 ms mas el tiempo del pulso
    T1CON=0b10110000;  //PS de 8
    CCPR2=60536+125;    //Servomotor a 0�
    CCP2CON=0b00001001; //Establece modo de comparaci�n para generaci�n de pulso
    TMR1IF=0;
    TMR1IE=1;
    PEIE=1;
    TMR1ON=1;    
    GIE=1; //habilita interrupciones globales
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
    
    while (!Password(ing)) {
        teclaIf = 1;
        for (int i = 0; i <4; i++) {
            while(teclaRecibidaIf==0);
            teclaRecibidaIf=0;
            ing[i] = Tecla;
            
        }
        teclaIf = 0;
        if (Password(ing)) {
            MensajeLCD_Word("                      ");
            DireccionaLCD(0xC1);
            EscribeLCD_c(0);
            __delay_ms(1000);
        } else {
            
            DireccionaLCD(0xC1);
            MensajeLCD_Word("Incorrecto");
            __delay_ms(1000);
            DireccionaLCD(0xC1);
            MensajeLCD_Word("                      ");
        }
    }
    BorraLCD();
    /*DireccionaLCD(0x80);
    MensajeLCD_Word("                ");
    DireccionaLCD(0xC1);
    MensajeLCD_Word("                ");*/
    __delay_ms(500);
    
    
    while(1){   
        __delay_ms(500);
        LeerHT11();        
        TransmitirDatos(0, 0);
        Velocidad(Temp);
        ConvertirUnidades(0);
        Movimiento();
    }
}

void Movimiento(void){
    if(ADRES>0 & ADRES<=255){//0%- 25%
        a = 125;
    }else if(ADRES>255 & ADRES<=511){//25%-50%
        a = 292;
    }else if(ADRES>511 & ADRES<=767){ //50%-75%
        a = 458;
    }else if(ADRES>767 & ADRES<=1024){//75%-100%
        a = 625;
    }
}

void Velocidad(unsigned int val){
    if (val <= 22) {
        CCP1CON = 0;
        RC2 = 0;
    } else if (val > 22 && val < 25) {
        CCP1CON = 0b00001100;
        CCPR1L = 19;
    } else if (val >= 25 && val < 28) {
        CCP1CON = 0b00001100;
        CCPR1L = 38;
    } else if (val >= 28 && val < 31) {
        CCP1CON = 0b00001100;
        CCPR1L = 57;
    } else if (val >= 31 && val < 34) {
        CCP1CON = 0b00001100;
        CCPR1L = 76;
    } else if (val >= 34 && val < 37) {
        CCP1CON = 0b00001100;
        CCPR1L = 95;
    } else if (val >= 37 && val < 40) {
        CCP1CON = 0b00001100;
        CCPR1L = 113;
    } else if (val >= 40) {
        CCP1CON = 0;
        RC2 = 1;
    }
}

int Password(char* pass){
    unsigned char secret[] = {'0','1','2','3'};
    unsigned int access = 0; 
    DireccionaLCD(0xC1);
    for(int i=0; i<4; i++){
        
        EscribeLCD_c('*');
        if(pass[i] == secret[i]){
            access++;
        }
    }
    DireccionaLCD(0xC1);
    return (access==4)? 1:0; 
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
    LeerByte();
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
    teclaRecibidaIf=1;
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

unsigned int ConvertirUnidades(unsigned char canal) {
    ADCON0 = 0b00000001  | (canal << 2);
    GO = 1; //bsf ADCON0,1
    while (GO == 1);
    return ADRES;
}

unsigned char Recibir(void){
    while(RCIF==0);
    return RCREG;
}

void TransmitirDatos(unsigned int Ent1, unsigned int Ent2) {
    unsigned int n = Ent1 * 10 + Ent2, TempC = Temp;
    unsigned int Simb = 67;
    BorraLCD();
    TempC = Temp;
    Simb = 67;
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
    Transmitir('D');
    Transmitir('u');
    Transmitir('t');
    Transmitir('y');
    Transmitir(':');
    Transmitir(' ');
    if(CCP1CON==0&& RC2==1) Transmitir(1 + 48);
    Transmitir((CCP1CON!=0)? ((CCPR1L*100/126) / 10 + 48):0+48);
    Transmitir((CCP1CON!=0)? ((CCPR1L*100/126) % 10 + 48):0+48);
    Transmitir(' ');
    Transmitir('%');
    Transmitir('\n');
    Transmitir(' ');
    //Imprimir en LCD
    EscribeLCD_c(TempC / 10 + 48);
    EscribeLCD_c(TempC % 10 + 48);
    EscribeLCD_c(Simb);
    DireccionaLCD(192);
    MensajeLCD_Word("Duty:");
    if(CCP1CON==0 && RC2==1) EscribeLCD_c(1 + 48);
    EscribeLCD_c((CCP1CON!=0)? ((CCPR1L*100/126) / 10 + 48):0+48);
    EscribeLCD_c((CCP1CON!=0)? ((CCPR1L*100/126) % 10 + 48):0+48);
    EscribeLCD_c('%');
}

void __interrupt() ISR(void){
    if(TMR0IF == 1){
        TMR0IF = 0;
        TMR0 = 64286;
        if(RE0 == 1){
            CCP1CON = 0;
            RC2 = 0;
            __delay_ms(200);
            BorraLCD(); 
            MensajeLCD_Word("STOPED");
            SLEEP();
            while(1);
        }
    }
    if(RBIF==1){
        
        if(PORTB!=0b11110000 && teclaIf == 1){
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
            
            LeerTeclado();
            EscribeLCD_c(Tecla);
        }
        RBIF=0;        
        __delay_ms(50); //
         
    }
    if(TMR1IF==1){
        TMR1IF=0;
        TMR1=60536;        //Precarga del Timer1 que asegura los 20 ms mas el tiempo del pulso
        CCPR2=60536+a;    //Varia la duraci�n del p�lso y a su vez del angulo del servo
        CCP2CON=0b00001001;
    }
}