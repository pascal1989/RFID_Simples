/*
#####################################################################################
#  	Arquivo:            RFID_Simples.ino                                             
#       Micro-processador:  Arduino UNO R3
#  	Linguagem:	    Wiring / C /Processing /Fritzing / Arduino IDE
#       Versão:             1.0
#						
#	Objetivo:           Arduino RFID - Sistema de Control de Acceso RFID
#										  
#	Funcionamento:	    Utilizando RFID MFCR-522AN, controlar o acesso de pessoas
#                           autorizadas em um ambiente.
#                           Para isso verificar o cartão ou token de RF; sendo válido:
#                           acender um LED, tocar uma música e acionar um servo-motor
#                           de abertura de porta - sendo inválido, rejeitar o acesso
#                           piscando um LED, tocando uma música e mantendo a porta 
#                           travada.
#                           Um LCD de 20 caracteres x 4 linhas mostra as mensagens 
#                           referentes à liberação ou negação do acesso.
#
#			
#   Autor:              Omar Contreras - Sebastian Garay 
#					
#####################################################################################
*/

// librerias
#include <SPI.h>
#include <RFID.h>
#include <Servo.h>
#include "pitches.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// definir RFID
RFID rfid(10,5);

byte emaomos[5] = {0xF6,0xD6,0x66,0x00,0x46}; // Maestra
// agregar tarjetas
byte llavero1[5]= {0x91,0xD8,0x92,0x2B,0xF0};
// Declarar LCD y tipo, puerto
LiquidCrystal_I2C lcd(0x27,16,2);

byte serNum[5];
byte data[5];

// sonidos del buzzer
int access_melody[] = {NOTE_G4,0,NOTE_A4,0, NOTE_B4,0,NOTE_A4,0,NOTE_B4,0, NOTE_C5,0};
int access_noteDurations[] = {8,8,8,8,8,4,8,8,8,8,8,4};
int fail_melody[] = {NOTE_G2,0,NOTE_F2,0,NOTE_D2,0};
int fail_noteDurations[] = {8,8,8,8,8,4};

// definir pin del LED, Buzzer y Servo-motor
int LED_access = 2;
int LED_intruder = 3;
int speaker_pin = 8;
int servoPin = 9;

// definir servo
Servo doorLock;


void setup(){
  doorLock.attach(servoPin); // vinculando servo-motor al pin
  Serial.begin(9600); // inicializa comunicacion serial 
  lcd.init(); // inicializa LCD
  lcd.backlight();
  lcd.clear();// limpia LCD
  SPI.begin(); // inicializa comunicacion SPI para el RFID
  rfid.init(); // inicializacion RFID
  Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+ Omar Contreras - Sebastian Garay                                              +"); 
  Serial.println("+ Sistema de Control de Acceso RFID                                             +");
  Serial.println("+ RFID iniciado - esperando por tag                                          ...+");
  Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  delay(1000);
  pinMode(LED_access,OUTPUT);
  pinMode(LED_intruder,OUTPUT);
  pinMode(speaker_pin,OUTPUT);
  pinMode(servoPin,OUTPUT);
}
 
void loop(){
  lcd.clear();
  lcd.noBacklight();
  // variable para cada usuario
  boolean emaomos_card = true; // maestra
  boolean llavero1_card = true;
  
  if (rfid.isCard()){ // encontrado tarjeta valida
    if (rfid.readCardSerial()){ // lee tarjeta
      delay(1000);
      data[0] = rfid.serNum[0]; // almacena numero de serie
      data[1] = rfid.serNum[1];
      data[2] = rfid.serNum[2];
      data[3] = rfid.serNum[3];
      data[4] = rfid.serNum[4];
    }
  
  //rfid.halt(); // RFID standby
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("ID:");
  Serial.print("ID:");
  //lcd.setCursor(0,1);
  //lcd.print("     ");
  if(data[0] < 16){
    lcd.print("0");
  }
  lcd.print(data[0],HEX);
  
  if(data[1] < 16){
    lcd.print("0");
  }
  lcd.print(data[1],HEX);
  
  if(data[2] < 16){
    lcd.print("0");
  }
  lcd.print(data[2],HEX);
  
  if(data[3] < 16){
    lcd.print("0");
  }
  lcd.print(data[3],HEX);
  
  if(data[4] < 16){
    lcd.print("0");
  }
  lcd.print(data[4],HEX);
  
  for(int i=0; i<5; i++){
    if(data[i] != emaomos[i]) emaomos_card = false;// si no es la maestra pasa como falsa
    // agregar tarjetas permitidas
  }
  for(int i=0; i<5; i++){
    if(data[i] != llavero1[i]) llavero1_card = false;
  }
  Serial.println();
  if (emaomos_card){ // si se encuentra una tarjeta
    lcd.setCursor(0,3);
    lcd.print("MAESTRA!");
    Serial.println("Tarjeta Maestra!"); // imprime mensaje
    for (int i = 0; i < 12; i++){ // toca música de bienvenida
      int access_noteDuration = 1000/access_noteDurations[i];
      tone(speaker_pin, access_melody[i],access_noteDuration);
      int access_pauseBetweenNotes = access_noteDuration * 1.30;
      delay(access_pauseBetweenNotes);
      noTone(speaker_pin);
    }
  }
  
  else if(llavero1_card){
    lcd.setCursor(0,3);
    lcd.print("Hola Llavero 1");
    Serial.println("Hola Llavero 1");
    for (int i = 0; i < 12; i++){
      int access_noteDuration = 1000/access_noteDurations[i];
      tone(speaker_pin, access_melody[i],access_noteDuration);
      int access_pauseBetweenNotes = access_noteDuration * 1.30;
      delay(access_pauseBetweenNotes);
      noTone(speaker_pin);
    }
  } 
  
  else{ // si encuentra una tarjeta desconocida
    //lcd.setCursor(0,0);
    //lcd.print("ID no reconocido!");
    lcd.setCursor(0,3);
    lcd.print("Acceso Negado!");
    Serial.println("No reconocido!"); 
    digitalWrite(LED_intruder, HIGH); // prende LED rojo
    for (int i = 0; i < 6; i++){ // toca música de rechazo de usuario
      int fail_noteDuration = 1000/fail_noteDurations[i];
      tone(speaker_pin, fail_melody[i],fail_noteDuration);
      int fail_pauseBetweenNotes = fail_noteDuration * 1.30;
      delay(fail_pauseBetweenNotes);
      noTone(speaker_pin);
    }
    delay(1000);
    digitalWrite(LED_intruder, LOW); // apaga LED rojo
  }
  if (emaomos_card || llavero1_card){// agregar usuarios con lógica ||
    // mensaje bienvenida y acceso permitido  
    lcd.setCursor(0,0);
    lcd.print("Acceso Permitido!");
    lcd.setCursor(0,3);
    lcd.print("Bienvenido!");
    Serial.println("Acesso permitido!... Bienvenido!");
    digitalWrite(LED_access,HIGH); // prende LED verde
    doorLock.write(180); // libera puerta
    delay(5000); // espera
    doorLock.write(0); // trava puerta
    digitalWrite(LED_access,LOW); // apaga LED verde
  }
  Serial.println();
  delay(500);
  rfid.halt();
  lcd.noBacklight();
  }
}
  

