#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MIDI.h>
#include <Audio.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#include <Bounce.h>
#include <EEPROM.h>
#include <Encoder.h>

//MIDI port set up
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI2);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI3);

// A variable to know how long the LED has been turned on
elapsedMillis ledOnMillis;

int i;

//Basic parameters
const int ledPin = 21;
int blinkrate=0;
bool activity = false;
bool dataintegrity = false;
const int TotalMacro=10;

//encoder button pin
const int Enc_button = 2;
Bounce button0 = Bounce(Enc_button,5);

//encoder acceleration slope

int height = 4; //height
float slope = 0.3; //slope


unsigned long last_time;
unsigned long delta_time = 0;
unsigned long time_now = 0;
unsigned long encoderSlope=1;

//Audio
// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum1;          //xy=322,378
AudioOutputAnalog        dac1;           //xy=569,381
AudioConnection          patchCord1(drum1, dac1);
// GUItool: end automatically generated code


//two buttons
const int Assign_button_1 = 14;
const int Assign_button_2 = 15;
Bounce button1 = Bounce(Assign_button_1,5);
Bounce button2 = Bounce(Assign_button_2,5);


//Encoder config
const int encoder0PinA = 12;
const int encoder0PinB = 11;
Encoder mainEnc(encoder0PinA, encoder0PinB);
//long encoderPosition  = -999;
long newEncoderReading = -999;
int maxEncoder = 512; //512
int maxValue = 127;
int encoderAcc = 10;
int progress = 0;
int val = 0;
const int PWMled1 = 22;
const int PWMled2 = 23;

//static uint8_t sysex[6] = {0x7D, 0x01, 0x10, 0x0e, 0x0f, 0x01};

struct Button{
  uint8_t USBport=1;          //USB port # to send a data out, zero means don't send
  uint8_t MIDIport=1;         //HW MIDI port # to send a data out, zero means don't send
  uint8_t type=0;             //1=CC, 2=Note
  uint8_t number;             //CC# or Note#
  uint8_t channel=1;          //MIDI Channel
  uint8_t value;              //Velocity for Note ON messages or CC value
  boolean push;               //true=push, false=toggle for NOTE ON
  boolean toggled = false;
};
struct Controller{
  uint8_t USBport=1;          //USB port # to send a data out, zero means don't send
  uint8_t MIDIport=1;         //HW MIDI port # to send a data out, zero means don't send
  uint8_t type=0;             //1=CC, 2=Note, 3=Program Change, 4=NRPN, 5=SynclavierMODE(can be the ONLY in a macro)
  uint8_t channel=1;          //MIDI Channel
  uint8_t number;             //CC# or Note# or Program#
  uint8_t lowvalue;           //Low Value for CC, main value for Note
  uint8_t highvalue;          //High Value for CC, ignored Note
  uint8_t velocity;           //Velocity for Note ON messages
  uint8_t mapstart;           //Low Value for CC, main value for Note
  uint8_t mapend;             //High Value for CC, ignored Note
  int     deviation=0;        //For creating curves for CC messages deviation -100 to 100.
  boolean toggled=false;      //For Note or Program Change toggled tracking
 
};

struct MacroPot{
  char name[8] = {'M', 'a', 'c', 'r', 'o', ' ', 'X', '\0'};   //Display name
  struct Button Button;
  struct Controller Controller[5];                      // 5 controllers simultaniously to send
  uint8_t MidiMerge = 0;                                // need to define MIDI IN to MERGE OUT 1-3, 0 means no merge.
  long encoderPosition = 0;
};

struct MacroPot MacroPot[TotalMacro];
uint8_t CurrentMacro=0;



void myClock() {
  digitalWrite(ledPin, HIGH); // Any Note-On turns on LED
  analogWrite(PWMled1, 255);
  blinkrate++;
  if (blinkrate > 8) {
    blinkrate=-8;
    digitalWrite(ledPin, LOW);
    analogWrite(PWMled1, 0);
  }

  //Audio Output on clock Test Example here:
  drum1.noteOn();

}

void EpromSave() {
  double addr=0;
  for (i = 0; i < TotalMacro; i++) {
    EEPROM.put(addr, MacroPot[i]);
    addr=addr+sizeof(MacroPot[i]);  
  }
  EEPROM.put(addr+sizeof(MacroPot[TotalMacro]), CurrentMacro);
   
}

void EpromLoad() {
  double addr=0;
  for (i = 0; i < TotalMacro; i++) {
    EEPROM.get(addr, MacroPot[i]);
    addr=addr+sizeof(MacroPot[i]);  
  }
  EEPROM.get(addr+sizeof(MacroPot[TotalMacro]), CurrentMacro);    

}


void setup() {
  Serial.begin(115200);
  //1 MIDI rate
  //Serial.begin(31250);
  pinMode(Enc_button, INPUT_PULLUP);
  pinMode(Assign_button_1, INPUT_PULLUP);
  pinMode(Assign_button_2, INPUT_PULLUP);
  
  pinMode(ledPin, OUTPUT);
  pinMode(PWMled1, OUTPUT);
  pinMode(PWMled2, OUTPUT);
  analogWrite(PWMled1,0);
  analogWrite(PWMled2,0);
/*for non pull up encoder
  pinMode (encoder0PinA, INPUT);
  pinMode (encoder0PinB, INPUT);
  pinMode(encoder0PinA, INPUT_PULLUP);
  pinMode(encoder0PinB, INPUT_PULLUP);  
*/

  //Audio
  AudioMemory(10);
  AudioNoInterrupts();
  drum1.frequency(60);
  drum1.length(1500);
  drum1.secondMix(0.0);
  drum1.pitchMod(0.55);
  AudioInterrupts();
 
  //MIDI
  MIDI1.begin(MIDI_CHANNEL_OMNI);
  MIDI2.begin(MIDI_CHANNEL_OMNI);
  MIDI3.begin(MIDI_CHANNEL_OMNI);
  MIDI1.turnThruOff();
  MIDI2.turnThruOff();
  MIDI3.turnThruOff();

  
  usbMIDI.setHandleClock(myClock);
   
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  delay(5);
 
 //display init, not required for "good" OLED
 //   delay(3000); 
  


  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Deftaudio");
  
 // display.setTextColor(BLACK, WHITE);
 // display.setCursor(90,0);
 // display.print("127");

 // display.fillRect(0, 17, display.width(), display.height()-17, WHITE);
  display.drawRect(0, 17, display.width(), display.height()-17, WHITE);
  display.display();
  delay(100);

//export all EPROM data
 // usbMIDI.sendSysEx(6, sysex);

//version LED blink
  analogWrite(PWMled1, 255);
  digitalWriteFast(ledPin, HIGH);
  delay(200);
  digitalWriteFast(ledPin, LOW);
  analogWrite(PWMled1, 0);
  button1.update();
  button2.update();
  

/*
//TEST DEFINE Macro:
  for (i = 0; i <= 10; i++) {
    MacroPot[i].name[6] = '1'+i; }

CurrentMacro=0;
MacroPot[CurrentMacro].name[0] = {'T'};
MacroPot[CurrentMacro].name[1] = {'e'};
MacroPot[CurrentMacro].name[2] = {'s'};
MacroPot[CurrentMacro].name[3] = {'t'};
MacroPot[CurrentMacro].name[4] = {':'};
MacroPot[CurrentMacro].name[5] = {' '};
MacroPot[CurrentMacro].name[6] = {'1'};
MacroPot[CurrentMacro].name[7] = {'\0'};
MacroPot[CurrentMacro].Button.USBport=1;
MacroPot[CurrentMacro].Button.MIDIport=1;
MacroPot[CurrentMacro].Button.type=2;
MacroPot[CurrentMacro].Button.channel=1;
MacroPot[CurrentMacro].Button.number=64;
MacroPot[CurrentMacro].Button.value=127;
MacroPot[CurrentMacro].Button.push=false;

MacroPot[CurrentMacro].Controller[0].USBport=1;
MacroPot[CurrentMacro].Controller[0].MIDIport=1;
MacroPot[CurrentMacro].Controller[0].type=1;
MacroPot[CurrentMacro].Controller[0].channel=1;
MacroPot[CurrentMacro].Controller[0].number=44;
MacroPot[CurrentMacro].Controller[0].lowvalue=0;
MacroPot[CurrentMacro].Controller[0].highvalue=127;
MacroPot[CurrentMacro].Controller[0].mapstart=0;
MacroPot[CurrentMacro].Controller[0].mapend=127;
MacroPot[CurrentMacro].Controller[0].deviation=0;

MacroPot[CurrentMacro].Controller[1].USBport=2;
MacroPot[CurrentMacro].Controller[1].MIDIport=1;
MacroPot[CurrentMacro].Controller[1].type=1;
MacroPot[CurrentMacro].Controller[1].channel=2;
MacroPot[CurrentMacro].Controller[1].number=45;
MacroPot[CurrentMacro].Controller[1].lowvalue=0;
MacroPot[CurrentMacro].Controller[1].highvalue=127;
MacroPot[CurrentMacro].Controller[1].mapstart=64;
MacroPot[CurrentMacro].Controller[1].mapend=127;
MacroPot[CurrentMacro].Controller[1].deviation=0;

CurrentMacro=1;
MacroPot[CurrentMacro].name[0] = {'S'};
MacroPot[CurrentMacro].name[1] = {'e'};
MacroPot[CurrentMacro].name[2] = {'s'};
MacroPot[CurrentMacro].name[3] = {'t'};
MacroPot[CurrentMacro].name[4] = {':'};
MacroPot[CurrentMacro].name[5] = {' '};
MacroPot[CurrentMacro].name[6] = {'2'};
MacroPot[CurrentMacro].name[7] = {'\0'};

MacroPot[CurrentMacro].Controller[0].USBport=2;
MacroPot[CurrentMacro].Controller[0].MIDIport=1;
MacroPot[CurrentMacro].Controller[0].type=5;
MacroPot[CurrentMacro].Controller[0].channel=2;

CurrentMacro=2;
MacroPot[CurrentMacro].name[0] = {'T'};
MacroPot[CurrentMacro].name[1] = {'e'};
MacroPot[CurrentMacro].name[2] = {'s'};
MacroPot[CurrentMacro].name[3] = {'t'};
MacroPot[CurrentMacro].name[4] = {':'};
MacroPot[CurrentMacro].name[5] = {' '};
MacroPot[CurrentMacro].name[6] = {'3'};
MacroPot[CurrentMacro].name[7] = {'\0'};

CurrentMacro=3;
MacroPot[CurrentMacro].name[0] = {'T'};
MacroPot[CurrentMacro].name[1] = {'e'};
MacroPot[CurrentMacro].name[2] = {'s'};
MacroPot[CurrentMacro].name[3] = {'t'};
MacroPot[CurrentMacro].name[4] = {':'};
MacroPot[CurrentMacro].name[5] = {' '};
MacroPot[CurrentMacro].name[6] = {'4'};
MacroPot[CurrentMacro].name[7] = {'\0'};

CurrentMacro=4;
MacroPot[CurrentMacro].name[0] = {'T'};
MacroPot[CurrentMacro].name[1] = {'e'};
MacroPot[CurrentMacro].name[2] = {'s'};
MacroPot[CurrentMacro].name[3] = {'t'};
MacroPot[CurrentMacro].name[4] = {':'};
MacroPot[CurrentMacro].name[5] = {' '};
MacroPot[CurrentMacro].name[6] = {'5'};
MacroPot[CurrentMacro].name[7] = {'\0'};
*/

// Zero Macro state
CurrentMacro=0;

//EPROM LOAD
EpromLoad();


//INIT button UP/DOWN code
checkButtons();

//INIT check encoder button
checkEncButton();


//initial display macros name
DisplayTitleUpdate();

}

void DisplayTitleUpdate(){
  display.clearDisplay();  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(String(MacroPot[CurrentMacro].name));
  display.drawRect(0, 17, display.width(), display.height()-17, WHITE);

  display.display();
  
 // delay(1000);
}

void SysexMode(){
  //Special Sysex MODE
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(BLACK, WHITE);
  display.print("Sysex Mod");
  display.display();
  button1.update();
  button2.update();

  //WAIT HERE:
  while(!button1.fallingEdge())
  {
    button1.update();

  if (usbMIDI.read()) {
    byte type = usbMIDI.getType();
    //byte channel = usbMIDI.getChannel();
    //byte data1 = usbMIDI.getData1();
    //byte data2 = usbMIDI.getData2();
    //byte cable = usbMIDI.getCable();

    if (type == usbMIDI.SystemExclusive) {

      // SysEx messages are special.  The message length is given in data1 & data2
      //unsigned int SysExLength = data1 + data2 * 256;
        
      // Unpack SysEx
      byte * sysbytes = usbMIDI.getSysExArray();
      
     /*
        for (i = 0; i <= 16; i++) { 
        display.clearDisplay();
        display.setTextColor(WHITE, BLACK);
        display.setCursor(40,18);
        display.print("         ");
        display.setCursor(40,18);
        display.print(i);
        display.print(": ");
        display.print(sysbytes[i], HEX);
        display.display();
        delay (1000); 
        }
      */
      // Check for SysEx  config message
      if (sysbytes[0] == 0xf0 && sysbytes[1] == 0x7d) { // check start f0 and ID 7D.
           
        //CurrentMacro = sysbytes[2];
        switch (sysbytes[3]) {
          //Main parameters
          case 100:
            MacroPot[sysbytes[2]].name[0] = sysbytes[4];
            MacroPot[sysbytes[2]].name[1] = sysbytes[5];
            MacroPot[sysbytes[2]].name[2] = sysbytes[6];
            MacroPot[sysbytes[2]].name[3] = sysbytes[7];
            MacroPot[sysbytes[2]].name[4] = sysbytes[8];
            MacroPot[sysbytes[2]].name[5] = sysbytes[9];
            MacroPot[sysbytes[2]].name[6] = sysbytes[10];
            MacroPot[sysbytes[2]].name[7] = {'\0'};
            MacroPot[sysbytes[2]].MidiMerge = sysbytes[11];

            if (sysbytes[12] == 0x7f) { dataintegrity=true; } // check the last byte
            display.clearDisplay();
            display.setTextColor(WHITE, BLACK);
            display.setCursor(0,0);
            display.println("OK: UP for exit");
            display.display();
            break;
            
          //Button configuration
          case 0:
            MacroPot[sysbytes[2]].Button.USBport=sysbytes[4];
            MacroPot[sysbytes[2]].Button.MIDIport=sysbytes[5];
            MacroPot[sysbytes[2]].Button.type=sysbytes[6];
            MacroPot[sysbytes[2]].Button.channel=sysbytes[7];
            MacroPot[sysbytes[2]].Button.number=sysbytes[8];
            MacroPot[sysbytes[2]].Button.value=sysbytes[9];
            MacroPot[sysbytes[2]].Button.push=sysbytes[10];
            
            if (sysbytes[11] == 0x7f) { dataintegrity=true; } // check the last byte
            display.clearDisplay();
            display.setTextColor(WHITE, BLACK);
            display.setCursor(0,0);
            display.println("OK: UP for exit");
            display.display();
            break;
            
          //Controller configuration
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].USBport=sysbytes[4];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].MIDIport=sysbytes[5];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].type=sysbytes[6];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].channel=sysbytes[7];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].number=sysbytes[8];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].lowvalue=sysbytes[9];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].highvalue=sysbytes[10];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].mapstart=sysbytes[11];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].mapend=sysbytes[12];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].deviation=sysbytes[13];
            MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].velocity=sysbytes[14];
            
            if (sysbytes[15] == 0x7f) { dataintegrity=true; } // check the last byte
            display.clearDisplay();
            display.setTextColor(WHITE, BLACK);
            display.setCursor(0,0);
            display.println("OK: UP for exit");
            //display.println(MacroPot[sysbytes[2]].Controller[sysbytes[3]-1].deviation);
            display.display();           
            break;
        }
      } 
     }
      
  
  
  }
 }   
  display.clearDisplay();
  
  //Save ALL
  if (dataintegrity) {
   EpromSave();   
   }
 
  DisplayTitleUpdate();
  UpdateState();
  activity = true;
}

void DisplaySynclavier(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(BLACK, WHITE);
  display.print("Synclavier");
  display.display();
  mainEnc.write(MacroPot[CurrentMacro].encoderPosition);
  encoderAcc=0;
}

void checkButtons() {

  button1.update();
  button2.update();  
/*
  //this is encoder button
  button0.update();
  if (button0.fallingEdge()) {
   display.clearDisplay();
   display.setCursor(0,0);
   display.setTextColor(BLACK, WHITE);
   display.print("Not avail.");
   display.display();
   delay(500);
   display.clearDisplay();
   DisplayTitleUpdate();
   UpdateState();
   activity = true;
  }
*/
  //this is UP button
  if (button1.fallingEdge()) {
      delay(100);
      button2.update();
      if (button2.fallingEdge()) {
      SysexMode();
      }
      else {
      //display.setCursor(40,18);
      //display.print("-b1next-");
      //display.display();
      if (CurrentMacro < TotalMacro-1) {
        CurrentMacro++;
      } else {
          analogWrite(PWMled1, 255);
          delay(100);
          digitalWriteFast(ledPin, LOW);
          analogWrite(PWMled1, 0);
       }
      if (MacroPot[CurrentMacro].Controller[0].type!=5) {
        DisplayTitleUpdate();
        UpdateState();
      } else {
        DisplaySynclavier();
      }
      activity = true;
      EpromSave();
      }
  }
  //this is Down button
  else if (button2.fallingEdge()) {
      delay(100);
      button1.update();
      if (button1.fallingEdge()) {
        SysexMode();
      }
      else {
      //display.setCursor(40,18);
      //display.print("-b2prev-");
      //display.display();
      if (CurrentMacro>0) {
        CurrentMacro--;
      } else {
          analogWrite(PWMled1, 255);
          delay(100);
          digitalWriteFast(ledPin, LOW);
          analogWrite(PWMled1, 0);
       }
  
      if (MacroPot[CurrentMacro].Controller[0].type!=5) {
        DisplayTitleUpdate();
        UpdateState();
      } else {
        DisplaySynclavier();
      }
      activity = true;
      EpromSave();
      }
  }

/* 
  if (button1.risingEdge()) {
      display.setCursor(40,18);
      display.print("-b1off-");
      display.display();
   }
  
  
  if (button2.risingEdge()) {
      display.setCursor(40,18);
      display.print("-b2off-");
      display.display();
   }
*/

}


void UpdateState() {
      //WRITE EPROM HERE TODO----
      
      mainEnc.write(MacroPot[CurrentMacro].encoderPosition);
      
      val = map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, maxValue);
      //Display update   
      display.setTextColor(BLACK, WHITE);
      display.setCursor(90,0);
      display.print("   ");
      display.setCursor(90,0);
      display.print(val);
      progress = map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 2, display.width()-2);
      display.drawRect(0, 17, display.width(), display.height()-17, WHITE);
      display.fillRect(progress, 19, display.width()-progress-2, display.height()-21, BLACK);
      display.fillRect(2, 19, progress-2, display.height()-21, WHITE);
          /* TEST AREA
        display.setCursor(0,0);
        display.print("     ");
        display.setCursor(0,0);
        display.print(encoderAcc);
       */  
      display.display();
      analogWrite(PWMled2, map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, 255));
}

void checkEncButton() {
  button0.update();
  if(MacroPot[CurrentMacro].Button.push) {
      //if push button
    if (button0.fallingEdge()&&(MacroPot[CurrentMacro].Button.type>0)) {
      //press event
      
      if (MacroPot[CurrentMacro].Button.type==1) {
        //CC
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        }
      }
    
       if (MacroPot[CurrentMacro].Button.type==2) {
        //note
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        }
       }        
      }

    if (button0.risingEdge()&&(MacroPot[CurrentMacro].Button.type>0)) {
      //release event
      
      if (MacroPot[CurrentMacro].Button.type==1) {
        //CC
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        }
      }
    
       if (MacroPot[CurrentMacro].Button.type==2) {
        //note
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        }
       }
    }
    
  } else {
    //if toggle mode
    if (button0.fallingEdge()&&(MacroPot[CurrentMacro].Button.type>0)) {
     if (MacroPot[CurrentMacro].Button.toggled) {
      if (MacroPot[CurrentMacro].Button.type==1) {
        //CC
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendControlChange(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        }
      }
    
       if (MacroPot[CurrentMacro].Button.type==2) {
        //note
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendNoteOff(MacroPot[CurrentMacro].Button.number, 0, MacroPot[CurrentMacro].Button.channel);
          break;
        }
       }
       
      MacroPot[CurrentMacro].Button.toggled = false;
     } else {
      if (MacroPot[CurrentMacro].Button.type==1) {
        //CC
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendControlChange(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        }
      }
    
       if (MacroPot[CurrentMacro].Button.type==2) {
        //note
        if (MacroPot[CurrentMacro].Button.USBport>0) {
         usbMIDI.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel, MacroPot[CurrentMacro].Button.USBport-1);
         usbMIDI.send_now();
        }
        switch (MacroPot[CurrentMacro].Button.MIDIport) {
        case 1:
          MIDI1.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 2:
          MIDI2.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        case 3:
          MIDI3.sendNoteOn(MacroPot[CurrentMacro].Button.number, MacroPot[CurrentMacro].Button.value, MacroPot[CurrentMacro].Button.channel);
          break;
        }
       }
      
      MacroPot[CurrentMacro].Button.toggled = true;
     } 
    }
   }
}


int Deviation(int value, double deviation)
 //https://stackoverflow.com/questions/41134365/formula-to-create-simple-midi-velocity-curves
    {

        float minMidiValue = 0;
        float maxMidiValue = 127;
        float midMidiValue = 63.5;

        // This is our control point for the quadratic bezier curve
        // We want this to be between 0 (min) and 63.5 (max)
        double controlPointX = midMidiValue + ((deviation / 100) * midMidiValue);

        // Get the percent position of the incoming value in relation to the max
        double t = (double)value / maxMidiValue;

        // The quadratic bezier curve formula
        // B(t) = ((1 - t) * (1 - t) * p0) + (2 * (1 - t) * t * p1) + (t * t * p2)

        // t  = the position on the curve between (0 and 1)
        // p0 = minMidiValue (0)
        // p1 = controlPointX (the bezier control point)
        // p2 = maxMidiValue (127)

        // Formula can now be simplified as:
        // B(t) = ((1 - t) * (1 - t) * minMidiValue) + (2 * (1 - t) * t * controlPointX) + (t * t * maxMidiValue)

        // What is the deviation from our value?
        int delta = (int)round((2 * (1 - t) * t * controlPointX) + (t * t * maxMidiValue));
        return (value - delta) + value;
    }

void EncoderProcessing() {

 for (i = 0; i < 5; i++) {
    
    switch (MacroPot[CurrentMacro].Controller[i].type) {
  
      case 1:
      // CC mode
        val = map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, 127); 
        val = Deviation(val, (double) map(MacroPot[CurrentMacro].Controller[i].deviation,0,127,-100,100));
        val = map(val, MacroPot[CurrentMacro].Controller[i].mapstart, MacroPot[CurrentMacro].Controller[i].mapend, MacroPot[CurrentMacro].Controller[i].lowvalue, MacroPot[CurrentMacro].Controller[i].highvalue);

        if (map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, 127) > MacroPot[CurrentMacro].Controller[i].mapstart && map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, 127) < MacroPot[CurrentMacro].Controller[i].mapend)
        {
        
          if (MacroPot[CurrentMacro].Controller[i].USBport>0) {
           usbMIDI.sendControlChange(MacroPot[CurrentMacro].Controller[i].number, val, MacroPot[CurrentMacro].Controller[i].channel, MacroPot[CurrentMacro].Controller[i].USBport-1);
           usbMIDI.send_now();
          }
        
          switch (MacroPot[CurrentMacro].Controller[i].MIDIport) {
            case 1:
               MIDI1.sendControlChange(MacroPot[CurrentMacro].Controller[i].number, val, MacroPot[CurrentMacro].Controller[i].channel);
                  break;
                case 2:
                  MIDI2.sendControlChange(MacroPot[CurrentMacro].Controller[i].number, val, MacroPot[CurrentMacro].Controller[i].channel);
                  break;
                case 3:
                  MIDI3.sendControlChange(MacroPot[CurrentMacro].Controller[i].number, val, MacroPot[CurrentMacro].Controller[i].channel);
                  break;
                }       
        }
        
        break;
  
      case 2:
      // Note mode
        val = map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, 127); 

        if (val > MacroPot[CurrentMacro].Controller[i].mapstart && val < MacroPot[CurrentMacro].Controller[i].mapend && MacroPot[CurrentMacro].Controller[i].toggled==false)
        {
            if (MacroPot[CurrentMacro].Controller[i].USBport>0) {
             usbMIDI.sendNoteOn(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel, MacroPot[CurrentMacro].Controller[i].USBport-1);
             usbMIDI.send_now();
            }
          
            switch (MacroPot[CurrentMacro].Controller[i].MIDIport) {
              case 1:
                 MIDI1.sendNoteOn(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  case 2:
                    MIDI2.sendNoteOn(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  case 3:
                    MIDI3.sendNoteOn(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  }       
        
         MacroPot[CurrentMacro].Controller[i].toggled=true;
        }

            
       if ((val < MacroPot[CurrentMacro].Controller[i].mapstart || val > MacroPot[CurrentMacro].Controller[i].mapend) && MacroPot[CurrentMacro].Controller[i].toggled==true)
        { 
            if (MacroPot[CurrentMacro].Controller[i].USBport>0) {
             usbMIDI.sendNoteOff(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel, MacroPot[CurrentMacro].Controller[i].USBport-1);
             usbMIDI.send_now();
            }
          
            switch (MacroPot[CurrentMacro].Controller[i].MIDIport) {
              case 1:
                 MIDI1.sendNoteOff(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  case 2:
                    MIDI2.sendNoteOff(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  case 3:
                    MIDI3.sendNoteOff(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].velocity, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  }       
         
          MacroPot[CurrentMacro].Controller[i].toggled=false;
         }
          
        break;
  
      case 3:
      // Program Change
        val = map(MacroPot[CurrentMacro].encoderPosition, 0, maxEncoder, 0, 127); 

        if (val > MacroPot[CurrentMacro].Controller[i].mapstart && val < MacroPot[CurrentMacro].Controller[i].mapend && MacroPot[CurrentMacro].Controller[i].toggled==false)
        {
            if (MacroPot[CurrentMacro].Controller[i].USBport>0) {
             usbMIDI.sendProgramChange(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].channel, MacroPot[CurrentMacro].Controller[i].USBport-1);
             usbMIDI.send_now();
            }
          
            switch (MacroPot[CurrentMacro].Controller[i].MIDIport) {
              case 1:
                 MIDI1.sendProgramChange(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  case 2:
                    MIDI2.sendProgramChange(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  case 3:
                    MIDI3.sendProgramChange(MacroPot[CurrentMacro].Controller[i].number, MacroPot[CurrentMacro].Controller[i].channel);
                    break;
                  }       
        
         MacroPot[CurrentMacro].Controller[i].toggled=true;
        }

            
       if ((val < MacroPot[CurrentMacro].Controller[i].mapstart || val > MacroPot[CurrentMacro].Controller[i].mapend) && MacroPot[CurrentMacro].Controller[i].toggled==true)
        { 

          MacroPot[CurrentMacro].Controller[i].toggled=false;
         }
          
        break;
  
      case 4:
      // NRPN
      // Not implemented
        break;
  
      case 5:
      // SynclavierMODE
        

        if (MacroPot[CurrentMacro].Controller[i].USBport>0) {
             usbMIDI.sendPitchBend(500*encoderAcc, MacroPot[CurrentMacro].Controller[i].channel, MacroPot[CurrentMacro].Controller[i].USBport-1);
             usbMIDI.send_now();
            }
          
            switch (MacroPot[CurrentMacro].Controller[i].MIDIport) {
              case 1:
                 MIDI1.sendPitchBend(500*encoderAcc, MacroPot[CurrentMacro].Controller[i].channel);

                    break;
                  case 2:
                    MIDI2.sendPitchBend(500*encoderAcc, MacroPot[CurrentMacro].Controller[i].channel);
        
                    break;
                  case 3:
                    MIDI3.sendPitchBend(500*encoderAcc, MacroPot[CurrentMacro].Controller[i].channel);
                    
                    break;
                  }       
        analogWrite(PWMled2, map(500*encoderAcc, -8000, 8000, 0, 255));
        display.setTextColor(WHITE, BLACK);
        display.setCursor(40,18);
        display.print("         ");
        display.setCursor(40,18);
        display.println(50*encoderAcc);
        display.display();
        ledOnMillis = 0;       
        MacroPot[CurrentMacro].Controller[i].toggled=true;
        break;    
  }
    
 }


    
}

void loop() {
  activity = false;

// HARDWARE MIDI IN FLOW
  if (MIDI1.read()) {
    // get a MIDI IN1 (Serial) message
    byte type = MIDI1.getType();
    byte channel = MIDI1.getChannel();
    byte data1 = MIDI1.getData1();
    byte data2 = MIDI1.getData2();

    // forward the message to USB MIDI virtual cable #0
    if (type != midi::SystemExclusive) {
      // Normal messages, simply give the data to the usbMIDI.send()
      usbMIDI.send(type, data1, data2, channel, 0);
      usbMIDI.send_now();
      
      if (bitRead(MacroPot[CurrentMacro].MidiMerge,0)==true) {
        MIDI1.send(type, data1, data2, channel);        
      }
      if (bitRead(MacroPot[CurrentMacro].MidiMerge,1)==true) {
        MIDI2.send(type, data1, data2, channel);
        
      }
      if (bitRead(MacroPot[CurrentMacro].MidiMerge,2)==true) {
        MIDI3.send(type, data1, data2, channel);    
      }

    } else {
      // SysEx messages are special.  The message length is given in data1 & data2
      unsigned int SysExLength = data1 + data2 * 256;
      usbMIDI.sendSysEx(SysExLength, MIDI1.getSysExArray(), true, 0);
      usbMIDI.send_now();
      
      if (bitRead(MacroPot[CurrentMacro].MidiMerge,0)==true) {
        MIDI1.sendSysEx(SysExLength, MIDI1.getSysExArray(), true);
        
      }
      if (bitRead(MacroPot[CurrentMacro].MidiMerge,1)==true) {
        MIDI2.sendSysEx(SysExLength, MIDI1.getSysExArray(), true);
        
      }
      if (bitRead(MacroPot[CurrentMacro].MidiMerge,2)==true) {
        MIDI3.sendSysEx(SysExLength, MIDI1.getSysExArray(), true);
        
      } 
    }
    activity = true;
  }

  if (MIDI2.read()) {
    // get a MIDI IN2 (Serial) message
    byte type = MIDI2.getType();
    byte channel = MIDI2.getChannel();
    byte data1 = MIDI2.getData1();
    byte data2 = MIDI2.getData2();

    // forward the message to USB MIDI virtual cable #1
    if (type != midi::SystemExclusive) {
      // Normal messages, simply give the data to the usbMIDI.send()
      usbMIDI.send(type, data1, data2, channel, 1);
      usbMIDI.send_now();
    } else {
      // SysEx messages are special.  The message length is given in data1 & data2
      unsigned int SysExLength = data1 + data2 * 256;
      usbMIDI.sendSysEx(SysExLength, MIDI1.getSysExArray(), true, 1);
      usbMIDI.send_now();
    }
    activity = true;
  }

  if (MIDI3.read()) {
    // get a MIDI IN1 (Serial) message
    byte type = MIDI3.getType();
    byte channel = MIDI3.getChannel();
    byte data1 = MIDI3.getData1();
    byte data2 = MIDI3.getData2();

    // forward the message to USB MIDI virtual cable #0
    if (type != midi::SystemExclusive) {
      // Normal messages, simply give the data to the usbMIDI.send()
      usbMIDI.send(type, data1, data2, channel, 2);
      usbMIDI.send_now();
    } else {
      // SysEx messages are special.  The message length is given in data1 & data2
      unsigned int SysExLength = data1 + data2 * 256;
      usbMIDI.sendSysEx(SysExLength, MIDI1.getSysExArray(), true, 2);
      usbMIDI.send_now();
    }
    activity = true;
  }

//END OF HARDWARE FLOW


// USB FLOW: coming out of USB
if (usbMIDI.read()) {
    // get the USB MIDI message, defined by these 5 numbers (except SysEX)
    byte type = usbMIDI.getType();
    byte channel = usbMIDI.getChannel();
    byte data1 = usbMIDI.getData1();
    byte data2 = usbMIDI.getData2();
    byte cable = usbMIDI.getCable();

    // forward this message to 1 of the 3 Serial MIDI OUT ports
    if (type != usbMIDI.SystemExclusive) {
      // Normal messages, first we must convert usbMIDI's type (an ordinary
      // byte) to the MIDI library's special MidiType.
      midi::MidiType mtype = (midi::MidiType)type;

      // Then simply give the data to the MIDI library send()
      switch (cable) {
        case 0:
          MIDI1.send(mtype, data1, data2, channel);
          break;
        case 1:
          MIDI2.send(mtype, data1, data2, channel);
          break;
        case 2:
          MIDI3.send(mtype, data1, data2, channel);
          break;
      }

    } else {
      // SysEx messages are special.  The message length is given in data1 & data2
      unsigned int SysExLength = data1 + data2 * 256;
        
        // Unpack SysEx
        byte * sysbytes = usbMIDI.getSysExArray();
 
      switch (cable) {
        case 0:
          MIDI1.sendSysEx(SysExLength, sysbytes, true);
          break;
        case 1:
          MIDI2.sendSysEx(SysExLength, sysbytes, true);
          break;
        case 2:
          MIDI3.sendSysEx(SysExLength, sysbytes, true);
          break;
      }
        
    }
    activity = true;
  }


//button UP/DOWN code
checkButtons();

//check encoder button
checkEncButton();




//encoder read and min/max values adjust
  newEncoderReading = mainEnc.read();

 if (MacroPot[CurrentMacro].Controller[0].type!=5) { 
  if (newEncoderReading > maxEncoder) {
    newEncoderReading = maxEncoder;
    mainEnc.write(maxEncoder);   
  }

  if (newEncoderReading < 0) {
    newEncoderReading = 0;
    mainEnc.write(0);    
  }
 }

    
//update on encoder change
  if (newEncoderReading != MacroPot[CurrentMacro].encoderPosition) {

  //encoder acceleration calc
    time_now = millis();
    delta_time = time_now - last_time;
    if (delta_time < 27) {
    encoderSlope = int(0.3 * (27 - delta_time) + 1);
    }
    last_time = time_now;
    encoderAcc = (newEncoderReading - MacroPot[CurrentMacro].encoderPosition) * encoderSlope;
     
    if ((newEncoderReading > MacroPot[CurrentMacro].encoderPosition)&&(newEncoderReading + encoderAcc < maxEncoder)){
      MacroPot[CurrentMacro].encoderPosition = newEncoderReading + encoderAcc;
      mainEnc.write(MacroPot[CurrentMacro].encoderPosition);
    }
    else if ((newEncoderReading < MacroPot[CurrentMacro].encoderPosition)&&(newEncoderReading + encoderAcc > 0)){
      MacroPot[CurrentMacro].encoderPosition = newEncoderReading + encoderAcc;
      mainEnc.write(MacroPot[CurrentMacro].encoderPosition);
    }
    else 
      MacroPot[CurrentMacro].encoderPosition = newEncoderReading;

    if (MacroPot[CurrentMacro].Controller[0].type!=5) {
     UpdateState();
          }

    EncoderProcessing();
  }


  // blink the LED when any activity has happened
  if (activity) {
    digitalWriteFast(ledPin, HIGH); // LED on
    ledOnMillis = 0;
  }
  if (ledOnMillis > 15) {
    digitalWriteFast(ledPin, LOW);  // LED off
  } 
  
 // Clean up for Synclavier mode - autozero function.
  if (ledOnMillis > 200 && MacroPot[CurrentMacro].Controller[0].type==5 && MacroPot[CurrentMacro].Controller[0].toggled==true) {
    encoderAcc=0;
    EncoderProcessing(); 
    MacroPot[CurrentMacro].Controller[0].toggled=false;
    analogWrite(PWMled2, 0);
  }

  
  delayMicroseconds(50); // limits message frequency
}
