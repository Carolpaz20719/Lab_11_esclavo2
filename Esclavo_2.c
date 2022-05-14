/*
 * File:   Esclavo_2.c
 * Author: Carolina Paz
 *
 * Created on 12 de mayo de 2022, 04:18 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Cloc1||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||k Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#include <xc.h>
#include <stdint.h>
/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 1000000
#define FLAG_SPI 0xFF
#define BOTON      PORTBbits.RB0     // Asignamos un alias a RB0
#define BOTON2     PORTBbits.RB1     // Boton que va a decrementar

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
char valor;         // Variable para guardar el valor recibido

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if (PIR1bits.SSPIF){                     // Revisar si fue interrupcion de SPI
        valor = SSPBUF;                      // SSPBUF a valor
        PORTD = SSPBUF;                      // guardamos el dato en PORTD
        
        CCPR1L = (valor>>3)+ 7;              // Corriento de ADRESH de 3 bit (0-32) -- 7 (bits mas sig. controlen el mov. total del servo)
        CCP1CONbits.DC1B = (valor & 0b01);   // Controlar los últimos bit para más prescision (bit menos sig. del ADRESH)
        PIR1bits.SSPIF = 0;                  // Limpiamos bandera de interrupcion
    }      
    return;
}
/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){ }
    return;
}
/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSEL = 0;                  // Usaremos I/O digitales
    ANSELH = 0;
      
    TRISD = 0;                  // PORTD como salida
    PORTD = 0;                  // Limpiar PORTD
    
    TRISA = 0b00100000;         // Colocar el pin RA5 como entrada
    PORTA = 0;                  // Limpiar el PORTA
    
    // Configuracion del reloj
    OSCCONbits.IRCF = 0b100;     // 1MHz
    OSCCONbits.SCS = 1;          // Reloj interno
    
    // Configuracion de SPI
    // Configs del esclavo
    TRISC = 0b00011000;          // -> SDI y SCK entradas, SD0 como salida
    PORTC = 0;

    // SSPCON <5:0>
    SSPCONbits.SSPM = 0b0100;    // -> SPI Esclavo, SS hablitado
    SSPCONbits.CKP = 0;          // -> Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;        // -> Habilitamos pines de SPI
    // SSPSTAT<7:6>
    SSPSTATbits.CKE = 1;         // -> Dato enviado cada flanco de subida
    SSPSTATbits.SMP = 0;         // -> Dato al final del pulso de reloj

    PIR1bits.SSPIF = 0;          // Limpiamos bandera de SPI
    PIE1bits.SSPIE = 1;          // Habilitamos int. de SPI
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    //Configuración del PWM
    TRISCbits.TRISC2 = 1;        // RC2//CCP1 como entrada
    PR2 = 62;                    // periodo de 4ms
   
    //Configuración del CCP
    CCP1CONbits.P1M = 0;         // single output 
    CCP1CONbits.CCP1M =0b1100;   // Modo PWM 1100
    CCPR1L = 0x0f;               // Ciclo de trabajo inicial
    CCP1CONbits.DC1B = 0;
    
    //Configuracion del TMR2
    PIR1bits.TMR2IF = 0;         // Limpiamos bandera de interrupcion del TMR2
    T2CONbits.T2CKPS = 0b11;     // prescaler 1:16
    T2CONbits.TMR2ON = 1;        // Encendemos TMR2
    while(!PIR1bits.TMR2IF);     // Esperar un cliclo del TMR2
    PIR1bits.TMR2IF = 0;         // Limpiamos bandera de interrupcion del TMR2 nuevamente
    
    TRISCbits.TRISC2 = 0;        // Habilitamos salida de PWM
  
}