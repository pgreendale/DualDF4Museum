#include <SoftwareSerial.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DFRobotDFPlayerMini.h>
//display definitions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display( SCREEN_WIDTH,
                          SCREEN_HEIGHT, 
                          &Wire, 
                          OLED_RESET,
                          800000UL,
                          100000UL);
//pin definitions 
#define PIN_SERIAL1_RX   3   //2nd software UART
#define PIN_SERIAL1_TX   4   //2nd software UART
#define PIN_SERIAL2_RX   7
#define PIN_SERIAL2_TX   2
#define PIN_LEDSTATUS 13  //Status LED
#define PIN_LEDKH1    6   //Play enable info led DFPlayer1
#define PIN_LEDKH2    5   //Play enable info led DFPlayer2
#define PIN_INSERT1   10   //Headphone detection for DFPlayer1
#define PIN_INSERT2   8   //Headphone detection for DFPlayer2
#define PIN_VOLUME1   A0  //Volume control analog input for DFPlayer1
#define PIN_VOLUME2   A1  //Volume control analog input for DFPlayer2

//initialize DFPlayer object
SoftwareSerial SoftSerial(PIN_SERIAL1_RX,PIN_SERIAL1_TX);
SoftwareSerial SoftSerial2(PIN_SERIAL2_RX,PIN_SERIAL2_TX);
DFRobotDFPlayerMini myDFPlayer;
DFRobotDFPlayerMini myDFPlayer2;

// Function declarations
void printDetail(uint8_t type, int value);
int readVolume(int InputPin, int EnablePin); 
void errorBlink(int numberBlinks);

enum player_status{
  pstop, 
  pause, 
  start
};
enum headphone_status{
  inserted,
  removed
};

void setup() {
  pinMode(PIN_LEDSTATUS,OUTPUT);
  pinMode(PIN_LEDKH1,OUTPUT);
  pinMode(PIN_LEDKH2,OUTPUT);
  pinMode(PIN_INSERT1,INPUT_PULLUP);
  pinMode(PIN_INSERT2,INPUT_PULLUP);
  pinMode(PIN_LEDSTATUS,OUTPUT);
  //Serial.begin(9600);
  SoftSerial.begin(9600);
  SoftSerial2.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    while(true){ // Don't proceed, loop forever and blink LED for error
      errorBlink(0);
    }
  }
  display.display();
  delay(100);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if(!myDFPlayer.begin(SoftSerial2,true,false)) { //Initialize Player 1 ! Error Handling not functional !
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Player1 err");
    display.display();
    while(true){
      errorBlink(1);
    }
   }
  if(!myDFPlayer2.begin(SoftSerial,true,false)) {  // Initialize Player2 ! Error handling not functional !
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Player2 err");
    display.display();
    while(true){
      errorBlink(2);  
    }
   }
  delay(500);
  // Configure to go!
  myDFPlayer.setTimeOut(500);
  myDFPlayer2.setTimeOut(500);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.enableLoopAll(); 
  myDFPlayer2.enableLoopAll();
  myDFPlayer.enableLoop(); 
  myDFPlayer2.enableLoop();
  digitalWrite(PIN_LEDSTATUS,HIGH);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Init complete");
  display.println("Soft: V.Weiss, Edelweisz");
  display.println("12/2020");
  display.display();
  delay(500);
}

// main loop
void loop() {
  int volume1 = 15; 
  int volume2 = 15; 
  int counter1 = 0; 
  int counter2 = 0; 
  int file1 =0; 
  int file2 =0; 
  headphone_status laststate1 = removed;
  headphone_status laststate2 = removed;
  display.clearDisplay(); 
  display.setCursor(0,0); 
  display.println("Start MP3 Playback");
  display.print("MP3-1:");
  display.print(myDFPlayer.readFileCounts(),DEC);
  display.println(" Files");
  display.print("MP3-2:");
  display.print(myDFPlayer2.readFileCounts(),DEC);
  display.println(" Files");
  display.display();
  delay(500);
  while(true){
    display.clearDisplay();
    display.setCursor(0,0);
    volume1 = 30 - readVolume(PIN_VOLUME1, PIN_LEDKH1)/34;
    volume2 = 30 - readVolume(PIN_VOLUME2, PIN_LEDKH2)/34; 
    display.print("Vol1:\t");
    display.print(volume1, DEC); 
    display.print(" Vol2:\t");
    display.println(volume2, DEC);
    myDFPlayer.volume(volume1);
    myDFPlayer2.volume(volume2);
    display.print("Track1:");
    display.print(file1,DEC);
    display.print(" Track2:");
    display.println(file2,DEC);
    //play control
    if(!digitalRead(PIN_INSERT1)){
     display.print("KH1:On ");
     if(laststate1 == removed){
      myDFPlayer.next();
      file1 = myDFPlayer.readCurrentFileNumber();
      counter1++;
      digitalWrite(PIN_LEDKH1, HIGH);
      laststate1 = inserted;
     }
      else{
       // digitalWrite(PIN_LEDKH1, !digitalRead(PIN_LEDKH1)); 
      }
    }
    else{
     display.print("KH1:Off  ");
     if(laststate1 == inserted){
       myDFPlayer.pause();
        digitalWrite(PIN_LEDKH1, LOW); 
       laststate1 = removed;
     }
    } // End Check Headphone_1
   if(!digitalRead(PIN_INSERT2)){
     display.println(" KH2:On ");
     if(laststate2 == removed){
      myDFPlayer2.next();
      file2 = myDFPlayer2.readCurrentFileNumber();
      counter2++;
      digitalWrite(PIN_LEDKH2, HIGH);
      laststate2 = inserted;
     }
     else{
      //digitalWrite(PIN_LEDKH2, !digitalRead(PIN_LEDKH2)); 
     }
    }
    else{
     display.println(" KH2:Off");
     if(laststate2 == inserted){
       myDFPlayer2.pause();
       digitalWrite(PIN_LEDKH2, LOW);
       laststate2 = removed; 
     }
    }
  display.print("Use 1:");
    display.print(counter1, DEC);
    display.print(" Use 2:");
    display.println(counter2, DEC);
    display.display(); 
    delay(500);
  }
 //End of loop
}

int readVolume(int InputPin, int EnablePin){      //read volume pot
  bool temp = digitalRead(EnablePin); 
  int  tempvalue;
  digitalWrite(EnablePin,HIGH);
  delay(5); //Wait to settle 
  tempvalue= analogRead(InputPin); 
  digitalWrite(EnablePin, temp); 
  return tempvalue; 
}
void errorBlink(int numberBlinks){                //blink n times short with 1s long blink after
    bool temp = digitalRead(PIN_LEDSTATUS); //save previous state
    while(numberBlinks)
    {
      digitalWrite(PIN_LEDSTATUS,HIGH);
      delay(250);
      digitalWrite(PIN_LEDSTATUS,LOW);
      delay(250);
      numberBlinks--; 
    }
    digitalWrite(PIN_LEDSTATUS, HIGH); 
    delay(1000);
    digitalWrite(PIN_LEDSTATUS,LOW);
    digitalWrite(PIN_LEDSTATUS,temp );   //restore state
}
void printDetail(uint8_t type, int value){        //from DFplayer Message handling implementation
  switch (type) {
    case TimeOut:
      display.println(F("Time Out!"));
      break;
    case WrongStack:
      display.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      display.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      display.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      display.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      display.println(F("Number:"));
      display.println(value);
      display.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      display.println(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          display.println(F("Card not found"));
          break;
        case Sleeping:
          display.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          display.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          display.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          display.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          display.println(F("Cannot Find File"));
          break;
        case Advertise:
          display.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  display.display();
} 
