#define VFD_in 7
#define VFD_clk 8
#define VFD_cs 9
#define VFD_rst 10  // You can use a pin of Arduino to do the Reset of driver... active at LOW value, normal running at HIGH
#define VFD_led 13  // only to debug.
bool flag_r = false;

//#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  Include following libraries to access IR sensor
#include <IRremote.hpp>
#define IR_RECEIVE_PIN 12
uint8_t var = 0x00;
char tmp[14];
int dataOverflows[14];

unsigned char data[20];
unsigned char str[20];
byte dataIndex;

void sndStbNo(unsigned char a) {
  // send without stb
  uint8_t data = 170;  //value to transmit, binary 10101010
  uint8_t mask = 1;    //our bitmask
  data = a;
  //This don't send the strobe signal, to be used in burst data send
  for (mask = 0b00000001; mask > 0; mask <<= 1) {  //iterate through bit mask
    digitalWrite(VFD_clk, LOW);
    delayMicroseconds(1);
    if (data & mask) {  // if bitwise AND resolves to true
      digitalWrite(VFD_in, HIGH);
      //Serial.print("1");
    } else {  //if bitwise and resolves to false
      digitalWrite(VFD_in, LOW);
      //Serial.print("0");
    }
    delayMicroseconds(1);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(2);
  }
  delayMicroseconds(9);
  //Serial.println();
}
void sndStbYes(unsigned char a) {
  // send with stb
  uint8_t data = 170;  //value to transmit, binary 10101010
  uint8_t mask = 1;    //our bitmask
  data = a;
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(2);
  for (mask = 0b00000001; mask > 0; mask <<= 1) {  //iterate through bit mask
    digitalWrite(VFD_clk, LOW);
    delayMicroseconds(1);
    if (data & mask) {  // if bitwise AND resolves to true
      digitalWrite(VFD_in, HIGH);
      //Serial.print("1");
    } else {  //if bitwise and resolves to false
      digitalWrite(VFD_in, LOW);
      //Serial.print("0");
    }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
  }
  delayMicroseconds(10);
  digitalWrite(VFD_cs, HIGH);
  delayMicroseconds(9);
  //Serial.println();
}
void ML9286_init(void) {
  delay(200);                  //power_up delay
  digitalWrite(VFD_rst, LOW);  //Reset event happen at LOW value.
  delay(50);                   //power_up delay
  //
  digitalWrite(VFD_rst, HIGH);
  delay(200);   
  //power_up delay
  //
  //To the Yamaha TSR-5810, need send first the LSB to MSB
  //------------------bit:
  // Configure VFD display (All on/off)
  sndStbYes(0b11100000);  // Here I configure the display as "Normal"
  delayMicroseconds(10);

  // Configure VFD display (number of grids)
  sndStbYes(0b11001100);  //
  delayMicroseconds(10);

  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(8);
  sndStbNo(0b10100000);  //(0b101*****) Duty Cycle 0/256 cmd initial
  sndStbNo(0b11111110);  // Duty Cycle value 240/256 Max. Bright intensity is max at 0b11111111
  delayMicroseconds(16);
  digitalWrite(VFD_cs, HIGH);
}
void dispNormal() {
  // Configure VFD display as 0b111***00: Normal display
  sndStbYes(0b11100000);  //
  delayMicroseconds(10);
}
void dispOFF() {
  // Configure VFD display (All on/off)
  sndStbYes(0b11100001);  //
  delayMicroseconds(10);
}
void dispON() {
  // Configure VFD display (All on/off)
  sndStbYes(0b11100011);  //
  delayMicroseconds(10);
}
void testModeOn() {
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(100);
  sndStbNo(0b01001000);  // Test mode, this is not to be used, only to test!
  delayMicroseconds(100);
  sndStbNo(0b00000000);  //
  delayMicroseconds(100);
  digitalWrite(VFD_cs, HIGH);
  delayMicroseconds(100);
}
void testModeOff() {
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(100);
  sndStbNo(0b00010000);  // Test mode, this is not to be used, only to test!
  delayMicroseconds(100);
  sndStbNo(0b00000000);  //
  delayMicroseconds(100);
  digitalWrite(VFD_cs, HIGH);
  delayMicroseconds(100);
}
//********************************************************************
void ML9286_print(unsigned char address, unsigned char *text) {
  unsigned char c;
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(2);
  //sndStbNo((0b01000000) | (address & 0x0F)); //((0b00010000)) + (address & 0x0F) );//)(0b00010000
  sndStbNo(0b00100000);  //((0b00010000)) + (address & 0x0F) );//)(0b00010000
  while (c = (*text++)) {
    //Serial.println(c, HEX);
    sndStbNo(c);  //& 0x7F); // 0x7F to 7bits ASCII code
  }
  delayMicroseconds(2);
  digitalWrite(VFD_cs, HIGH);
}
void strrevert1(char *string) {
  // Inverter the contents of pointer of string
  // and let it reverted until the next call of
  // function! exp: char letter[16] = "jogos.....tarde";
  // To do a declaration of pointer:  char* ptr=letter;
  // don't forget the null char "\0", this is a 1 more char
  // presente on the string.

  int len = 0;
  while (string[len])
    len++;

  int down = 0;
  int up = len - 1;

  while (up > down) {
    char c = string[down];
    string[down++] = string[up];
    string[up--] = c;
  }
}
void msgEmpty() {
  strcpy(data, "                   ");  // Fill the string with 12 spaces to stay empty
  strrevert1(data);              // Do the string reverting
  ML9286_print(0, data);         // write a grid number 1
  delay(300);
}
void msgHiFolks() {
  strcpy(data, "abcdefghijklmnopqrst");  // Fill the string
  strrevert1(data);                      // Do the string reverting
  ML9286_print(1, data);                 // write a grid number 1
  delay(1);                              // Give time to see the message on VFD
}
//********************************************************************
void setCGRAM() {
  //This allow construct of characters pattern data 5*7. Address 0x00 to 0x0F.
  //Have four bit address, and can be dispplayd by specfying the character code(adddress) by DCRAM
  //Here is draw the invader on the grid 5*7 characters.
  digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo(0b01000000);  //
        for (uint8_t n = 0; n < 4; n++){  // I filled all 16 positions by this four blocks to draw the invader!
          sndStbNo(0b01111110); //
          sndStbNo(0b00010101); //
          sndStbNo(0b01110111); //
          sndStbNo(0b00010101); //
          sndStbNo(0b01111110); //

          sndStbNo(0b00011110); //
          sndStbNo(0b01110101); //
          sndStbNo(0b00010111); //
          sndStbNo(0b01110101); //
          sndStbNo(0b00011110); //

          sndStbNo(0b01111110); //
          sndStbNo(0b00011101); //
          sndStbNo(0b01110111); //
          sndStbNo(0b00011101); //
          sndStbNo(0b01111110); //

          sndStbNo(0b00011110); //
          sndStbNo(0b01110101); //
          sndStbNo(0b00010111); //
          sndStbNo(0b01110101); //
          sndStbNo(0b00011110); //
        }
        // for(uint8_t n =8; n < 24; n++){
        //   sndStbNo(0b00000000); //
        //   sndStbNo(0b00000000); //
        //   sndStbNo(0b00000000); //
        //   sndStbNo(0b00000000); //
        //   sndStbNo(0b00000000); //
        // }
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
}
void clrCGRAM() {
  //This allow construct of characters pattern data 5*7. Address 0x00 to 0x0F.
  //Have four bit address, and can be dispplayd by specfying the character code(adddress) by DCRAM
  digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo(0b01000000);  //
        for (uint8_t n = 0; n < 25; n++) {  //
          sndStbNo(0b00000000);  //
          sndStbNo(0b00000000);  //
          sndStbNo(0b00000000);  //
          sndStbNo(0b00000000);  //
          sndStbNo(0b00000000);  //
        }
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
}
void setADRAM() {
  //This allows you to activate additional symbols, such as the  
  //red bar below the 5x7 alphanumeric character grid of this VFD.
  // I apply it at 14 digits on bellow cycle FOR.
  for (uint8_t n = 0; n < 14; n++) {  //
    digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo((0b01100000) | n);  //
    sndStbNo(0b00000011);  //
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
  }
}
void unSetADRAM() {
  //This allows you to activate additional symbols, such as the red bar below the 5x7 alphanumeric character grid of this VFD.
  // I apply it at 14 digits on bellow cycle FOR.
  for (uint8_t n = 0; n < 14; n++) {  //
    digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo((0b01100000) | n);  //
    sndStbNo(0b00000000);  //
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
  }
}
void setDCRAM() {
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(8);
  sndStbNo(0b00100000);  //
  //for (uint8_t n=0; n < 0x1; n++){  //
  //Note: The table char to the DL9286-03 not correspond!
  sndStbNo(0b00110000);  // Digit "0"
  sndStbNo(0b01010000);  // Digit "P"
  sndStbNo(0b10100000);  // Digit "Y"
  sndStbNo(0b11100000);  // Digit "ã"
  sndStbNo(0b01010101);  // Digit "U"
  sndStbNo(0b00110101);  // Digit "5"
  sndStbNo(0b11110101);  // Digit "õ"
  sndStbNo(0b00110000);  // Digit "0"
  sndStbNo(0b01010000);  // Digit "P"
  sndStbNo(0b10100000);  // Digit "Y"
  sndStbNo(0b11100000);  // Digit "ã"
  sndStbNo(0b01010101);  // Digit "U"
  sndStbNo(0b00110101);  // Digit "5"
  sndStbNo(0b11110101);  // Digit "õ"
  sndStbNo(0b00110000);  // Digit "0"
  sndStbNo(0b01010000);  // Digit "P"
  sndStbNo(0b10100000);  // Digit "Y"
  sndStbNo(0b11100000);  // Digit "ã"
  sndStbNo(0b01010101);  // Digit "U"
  sndStbNo(0b00110101);  // Digit "5"
  sndStbNo(0b11110101);  // Digit "õ"
  sndStbNo(0b00110000);  // Digit "0"
  sndStbNo(0b01010000);  // Digit "P"
  sndStbNo(0b10100000);  // Digit "Y"
  sndStbNo(0b11100000);  // Digit "ã"
  sndStbNo(0b01010101);  // Digit "U"
  sndStbNo(0b00110101);  // Digit "5"
  sndStbNo(0b11110101);  // Digit "õ"
  sndStbNo(0b11100000);  // Digit "ã"
  sndStbNo(0b01010101);  // Digit "U"
  sndStbNo(0b00110101);  // Digit "5"
  sndStbNo(0b11110101);  // Digit "õ"
  //}
  delayMicroseconds(16);
  digitalWrite(VFD_cs, HIGH);
}
void unSetDCRAM() {
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(8);
  sndStbNo(0b00100000);  //
  for (uint8_t n=0; n < 0x10; n++){  //
  //Note: The table char to the DL9286-03 not correspond!
  sndStbNo(0b01000001);  // Digit "0"
  }
  delayMicroseconds(16);
  digitalWrite(VFD_cs, HIGH);
}
void wrDCRAM(unsigned char address, unsigned char *text){  
    //Note: The table char to the DL9286-03 not correspond!
    unsigned char c;
    uint8_t i = 0x00;
    //dispNormal();
  for(uint8_t p=0; p < 20; p++){
      str[p] = ' ';
    }
          digitalWrite(VFD_cs, LOW);
          delayMicroseconds(8);
          sndStbNo(0b00100000 | address); // 001xxxxx: Write to DCRAM, 5 bits address
            while(c=(*text++)){
              sndStbNo(c); //
              // str[i]=c;
              // i=i+1;
              // Serial.println((char*) str);
            }
          delayMicroseconds(16);
          digitalWrite(VFD_cs, HIGH);
          
  //grids();
   //for(uint8_t b = 0; b < 200; b++){
      //gridControl();
    //}
}
void rdDCRAM(){
  for(uint8_t s = 0; s < 0x14; s++){
      digitalWrite(VFD_cs, LOW);
      delayMicroseconds(8);
      sndStbNo((0b00100000) | s);  // 001xxxxx: Write to DCRAM, 5 bits address. It copy to the grid 0 to 14.
      sndStbNo((0b00000010)); //This is the position from 0x00 to 0x0F to extract the character created.
      delayMicroseconds(16);
      digitalWrite(VFD_cs, HIGH);
  }        
}
void animeUpDCRAM(){
  uint8_t i = 0x00;
      for(uint8_t s = 0; s < 13; s++){
              i++;
            ML9286_print(0, "                   ");  
            digitalWrite(VFD_cs, LOW);
            delayMicroseconds(8);
            sndStbNo((0b00100000) | s);  // 001xxxxx: Write to DCRAM, 5 bits address
            switch (i) {
                case 1:  sndStbNo((0b00000000)); break; //This is the position from 0x00 to 0x0F to extract the character created.
                case 2:  sndStbNo((0b00000001)); break; //This is the position from 0x00 to 0x0F to extract the character created.
                case 3:  sndStbNo((0b00000010)); break; //This is the position from 0x00 to 0x0F to extract the character created.
                case 4:  sndStbNo((0b00000011)); i = 0; break;//This is the position from 0x00 to 0x0F to extract the character created.
                default: delay(3); break;
            }
            //sndStbNo((0b00000010)); //This is the position from 0x00 to 0x0F to extract the character created.
            delayMicroseconds(16);
            digitalWrite(VFD_cs, HIGH);
            delay(300);
      }
            
}
void animeDownDCRAM(){
  uint8_t i = 0x00;
      for(int s = 13; s >= 0; s--){ //Important: Here, a signed integer is used to prevent overflow when passing to a negative value!
              i++;
            ML9286_print(0, "                   ");  
            digitalWrite(VFD_cs, LOW);
            delayMicroseconds(8);
            sndStbNo((0b00100000) | s); // 001xxxxx: Write to DCRAM, 5 bits address, for it used direct the value of "s", is reason use signed integer above!
                                        // offcourse have other solutions to solve it, I can decrement "s" on the "OR" operation: "s-1" and use positive 1-14...
            switch (i) {
                case 1:  sndStbNo((0b00000000)); break; //This is the position from 0x00 to 0x0F to extract the character created.
                case 2:  sndStbNo((0b00000001)); break; //This is the position from 0x00 to 0x0F to extract the character created.
                case 3:  sndStbNo((0b00000010)); break; //This is the position from 0x00 to 0x0F to extract the character created.
                case 4:  sndStbNo((0b00000011)); i = 0; break;//This is the position from 0x00 to 0x0F to extract the character created.
                default: delay(3); break;
            }
            //sndStbNo((0b00000010)); //This is the position from 0x00 to 0x0F to extract the character created.
            delayMicroseconds(16);
            digitalWrite(VFD_cs, HIGH);
            delay(300);
      }
            
}
void grids(){
  double gridPosition = 0x00000001;
  uint16_t grid0 = 0x0001;
  uint8_t grid1 = 0x01;
  uint8_t grid2 = 0x01;
  uint8_t value = 0x00;

  for(uint8_t n = 0x00; n < 10; n++){
    

     if(grid0 == 0x0100){
           grid0 = 0x0001;
         }
         else{
          grid0 = grid0 <<=1;
         }
         grid1 = ((grid0) & 0x00FF);
     Serial.println(grid0, BIN);
    delay(5);
    }
}
void gridControl(){
  long gridPosition = 0x00000001;
  uint8_t wordA = 0;
  uint8_t wordB = 0;
  uint8_t wordC = 0;
    digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo(0b10000000); 
        for(uint8_t v = 0; v < 25; v++){  //24 digits * 3 bytes each (it use only 20 bits of this 3 bytes)!
          wordC = (gridPosition & 0x00ff0000UL) >>  16;
          wordB = (gridPosition & 0x0000ff00UL) >>  8;
          wordA = (gridPosition & 0x000000ffUL) >>  0;
          sndStbNo((0b00000000) | wordA);  // first char below line at left is: 0b10000000.
          sndStbNo((0b00000000) | wordB);  // Data to fill table of 5 grids * 16 segm = 80 bits on the table
          sndStbNo((0b00000000) | wordC);  // Data to fill table of 5 grids * 16 segm = 80 bits on the table
          gridPosition = gridPosition <<=1;
        }
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
}
void clsDCRAM() {
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(8);
  sndStbNo(0b00100000);                 //
  for (uint8_t n = 0; n < 0x20; n++) {  //
    sndStbNo(0b00000000);               //
  }
  delayMicroseconds(16);
  digitalWrite(VFD_cs, HIGH);
}
void setCGRAM_00(unsigned char address, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4) {
  digitalWrite(VFD_cs, LOW);
  delayMicroseconds(8);
  sndStbNo((0b01000000) | address);  // 0100xxxx: Selects CGRAM data write mode and four bits of address.
  for(uint8_t x = 0; x < 16; x++){
      sndStbNo(data0);  //
      sndStbNo(data1);  //
      sndStbNo(data2);  //
      sndStbNo(data3);  //
      sndStbNo(data4);  //
  }
  delayMicroseconds(16);
  digitalWrite(VFD_cs, HIGH);
}
void setGCRAM0to7() {
  //This determine which is the grid's stay ON... Only 3 can be accepted, more will turn display OFF?
  uint8_t v = 0x01;
  for (uint8_t p = 0x00; p < 8; p++) {
    digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo(0b10000000);  //

    sndStbNo((0b00000000) | v);  // 
    sndStbNo(0b00000000);        // 
    sndStbNo(0b00000000);        // 
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
    
    v = (v <<= 1);
    delay(500);
  }
}
void setGCRAM8to15() {
  //This determine which is the grid's stay ON... Only 3 can be accepted, more will turn display OFF?
  uint8_t v = 0x01;
  for (uint8_t p = 0x00; p < 8; p++) {
    digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo(0b10000000);  //

    sndStbNo(0b00000000);        // 
    sndStbNo((0b00000000) | v);  // 
    sndStbNo(0b00000000);        // 
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
    
    v = (v <<= 1);
    delay(500);
  }
}
void setGCRAM16to23() {
  //This determine which is the grid's stay ON... Only 3 can be accepted, more will turn display OFF?
  uint8_t v = 0x01;
  for (uint8_t p = 0x00; p < 8; p++) {
    digitalWrite(VFD_cs, LOW);
    delayMicroseconds(8);
    sndStbNo(0b10000000);  //

    sndStbNo(0b00000000);        // first char below line at left is: 0b10000000.
    sndStbNo(0b00000000);        // Data to fill table of 5 grids * 16 segm = 80 bits on the table
    sndStbNo((0b00000000) | v);  // Data to fill table of 5 grids * 16 segm = 80 bits on the table
    delayMicroseconds(16);
    digitalWrite(VFD_cs, HIGH);
    
    v = (v <<= 1);
    delay(500);
  }
}
void led() {
  for (uint8_t m = 0; m < 8; m++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  digitalWrite(VFD_rst, LOW);
  delay(5);
  digitalWrite(VFD_rst, HIGH);
  delay(100);  //Only to protect the action of Reset time!

  pinMode(12, INPUT_PULLUP);  // Our infra red receiver pin is here through a resistor, like 1K

  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);

  ML9286_init();
}
void loop() {
  // put your main code here, to run repeatedly:
  flag_r = false;  //!flag_r;
  uint8_t lng = 0x00;
  // //Next 3 line is only to manufacturer mode of test! Comment it!
  // testModeOn();
  // delay(200);
  // testModeOff();
  //
  //Next cycle if to switch on/off all segments of VFD.
  for(uint8_t i = 0; i < 5; i++){
      dispON();  //All segments ON
      delay(250);
      dispOFF(); //All segments OFF
      delay(250);
  }
  //The next line is very important, because it will let the display in the "Normal" mode!
  dispNormal();
  //
  clrCGRAM();
  strcpy(data, "Hi Folks!          ");  // Fill the string
  data[20] = '\0';
  lng = sizeof(data);
  //strrevert1(data);  // Do the string reverting
  wrDCRAM(0, data);  // write a grid number 0
  delay(1500);
  //
  clrCGRAM();
  strcpy(data, "ML9286:LAPIS      ");  // Fill the string
  data[20] = '\0';
  lng = sizeof(data);
  //strrevert1(data);  // Do the string reverting
  wrDCRAM(0, data);  // write a grid number 0
  delay(1500);
  //Blinking LED of Arduino, used only to debug process. Comment it!
  led();

  for(uint8_t s = 0; s < 4; s++){
         setCGRAM();
         delay(500);
  }
  
// for(uint8_t t = 0; t < 4; t++){
  clrCGRAM();
  ML9286_print(0, "Underline: \"_\"  \0");
  delay(500);
  //The next cycle FOR is a sample to active on/off of underline red bar.
  for(uint8_t u = 0; u < 4; u++){
      setADRAM();
      delay(500);
      unSetADRAM();
      delay(500);
   }
  //
  clrCGRAM();
  delay(500);
  setCGRAM();
  delay(500);
  // msgEmpty();
  // delay(500);
  ML9286_print(0, "                   \0");
  animeUpDCRAM(); 
  animeDownDCRAM(); 
  //
  rdDCRAM();
  delay(500);
  
  gridControl();
  //
  // setGCRAM0to7(); //100*****//Used to define grid is ON, max: 3 grids
  // setGCRAM8to15();
  // setGCRAM16to23();
   delay(500);
  //

  led();
}

