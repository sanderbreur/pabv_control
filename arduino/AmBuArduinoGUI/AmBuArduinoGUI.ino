#include "Arduino.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "GUI.h"


#define SPI_DEFAULT_FREQ 20000000

//encoder
#define pin_switch 7
#define pin_encoderA 6
#define pin_encoderB 5


static constexpr uint8_t pPEEP=0;
static constexpr uint8_t pPIP= 1;
static constexpr uint8_t pVol=  2;
static constexpr uint8_t pRR= 3;
static constexpr uint8_t pIH= 4;
static constexpr uint8_t pTH= 5;
static constexpr uint8_t pVmax= 6;
static constexpr uint8_t pPmin= 7;
static constexpr uint8_t pPmax= 8;
static constexpr uint8_t nParam = 9;


//gets data from master
float parms[nParam]={0.f};

GUI_value gui_value[nParam]={
  {.name="PEEP",  //Last cycle min pressure
   .id=pPEEP,
   .val=&parms[pPEEP],
   .dval=10.0f, 
   .dx=2.0f, 
   .min=0.0f, 
   .max=30.0f,
   .fmt="%02.1f"
  },
  {.name="PIP",  //last cycle max pressure
   .id=pPIP,
   .val=&parms[pPIP],
   .dval=30.0f, 
   .dx=2.0f, 
   .min=20.0f, 
   .max=50.0f,
   .fmt="%02.1f"
  },
  {.name="Vol",  // last cycle volume mL
   .id=pVol,
   .val=&parms[pVol],
   .dval=500.0f, 
   .dx=10.0f, 
   .min=200.0f, 
   .max=1000.0f,
   .fmt="%3.0f"
  },
  {.name="RR",    // resp rate in Hz
   .id=pRR,
   .val=&parms[pRR],
   .dval=20.0f, 
   .dx=0.5f, 
   .min=10.0f,
   .max= 30.0f,
   .fmt="%02.1f"
  },
  {.name="IH",    //Inhale time in seconds
   .id=pIH,
   .val=&parms[pIH],
   .dval=1.0f,
   .dx= 0.1f, 
   .min=  0.5f, 
   .max = 2.5f,
   .fmt="%02.1f"
  },
  {.name="TH",    //threhsold cmH2O
   .id=pTH,
   .val=&parms[pTH],
   .dval=0.0f,       
   .dx=0.1f, 
   .min=-10.0f, 
   .max=30.0f,
   .fmt="%02.1f"
  },
  {.name="Vmax",  //Threshold where volume will stop IH short
   .id=pVmax,
   .val=&parms[pVmax],
   .dval=800.0f, 
   .dx=10.0f, 
   .min=250.0f, 
   .max=1000.0f,
   .fmt="%3.0f"
  },
  {.name="Pmin",  //Threshold where pressure will alarm
   .id=pPmin,
   .val=&parms[pPmin],
   .dval=-10.0f, 
   .dx=2.0f, 
   .min=-10.0f, 
   .max=30.0f,
   .fmt="%02.1f"
  },
  {.name="Pmax",  //Threshold where pressure will alarm
   .id=pPmax,
   .val=&parms[pPmax],
   .dval=10.0f, 
   .dx=2.0f, 
   .min=10.0f, 
   .max=60.0f,
   .fmt="%02.1f"
  }
};


GUI gui;

void setup_display(){
  //uint8_t fs = 2;
  uint16_t  y = 10; 
  uint16_t x;
  uint16_t vspace = 25;
  uint16_t hspace = 5;
  x =  hspace;
  gui.addItem( gui_value[pPEEP], {x,y,ILI9341_PINK,2,ILI9341_PINK,3});   
  x =  GUI::TFT_THIRD + hspace;
  gui.addItem(gui_value[pPIP],  {x,y,ILI9341_GREENYELLOW,2,ILI9341_GREENYELLOW,3});   
  x =  2*GUI::TFT_THIRD + hspace;
  gui.addItem( gui_value[pVol], {x,y,ILI9341_CYAN,2,ILI9341_CYAN,3});   
  y =  1 + 8*2 + 8*3 + 7 + vspace;
  x = 3*hspace;
  gui.addItem( gui_value[pRR], {x,y,ILI9341_WHITE,2,ILI9341_WHITE,2});
  x =  GUI::TFT_THIRD + hspace;
  gui.addItem( gui_value[pIH], {x,y,ILI9341_WHITE,2,ILI9341_WHITE,2});
  x =  2*GUI::TFT_THIRD + hspace;
  gui.addItem( gui_value[pTH], {x,y,ILI9341_WHITE,2,ILI9341_WHITE,2});
  y =  1 + 8*2 + 8*3 + 7 + 14 + 8*2 + 7 + 8*2 + vspace;
  x = 3*hspace;
  gui.addItem( gui_value[pVmax], {x,y,ILI9341_WHITE,2,ILI9341_WHITE,2});
  x =  GUI::TFT_THIRD + hspace;
  gui.addItem( gui_value[pPmin], {x,y,ILI9341_WHITE,2,ILI9341_WHITE,2});
  x =  2*GUI::TFT_THIRD + hspace;
  gui.addItem( gui_value[pPmax], {x,y,ILI9341_WHITE,2,ILI9341_WHITE,2});
  gui.setup();
}


void update_display() {
  gui.update();
}


double get_rand(double rmin, double rmax){
  unsigned long rand_long = random();
  return rmin + (rmax - rmin)*((double)rand_long / 4.2949e9);
}


void setup() {
  asm(".global _printf_float");
  Serial.begin(9600);
  Serial.print("startup\n");
  // Encoder Pins
  pinMode (pin_encoderA, INPUT_PULLUP);
  pinMode (pin_encoderB, INPUT_PULLUP);
  pinMode (pin_switch, INPUT_PULLUP);

  setup_display();
  delay(2000);
}


uint32_t measTime = 0;
unsigned counter=0;
char s[64];
uint32_t curTime;
int8_t encDT;   // -1 for CCW, +1 for CW
bool encPushed;
int8_t guiParamSelected;  //-1 for non and 0... for n
//int8_t encVal = 0;
//int8_t encPushes = 0;
int8_t guiSelectedP = -1;
const uint32_t guiTimeout = 10000; //millis till gui times out
uint32_t guiPrevActionTime;
const uint32_t encTDeadTime = 50; //millis
const uint32_t encPDeadTime = 500; //millis
long encLastPTime = 0; //millis
long encLastTTime = 0; //millis
bool stateA, stateB;


void loop() {
  curTime = millis();
  // Check encoder for update
  encDT = 0;
  encPushed = false;
  if ( digitalRead(pin_switch) == false ) {
    if (curTime - encLastPTime > encPDeadTime) {
      encLastPTime = curTime;
      Serial.println("Switched");
      encPushed = true;
      //update_display();
    }
  }
  stateA = digitalRead(pin_encoderA);
  stateB = digitalRead(pin_encoderB);
  if (stateA == false  &&  encPushed == false) {
    // Here deal with turned encoder input.
    // I dont want to process pushes and turns at the same
    // time since they shouldn't happen in the same cycle. 
    if (curTime - encLastTTime > encTDeadTime) {
      encLastTTime = curTime;
      if (stateB != stateA) encDT--;
      else encDT++;
    Serial.println(encDT);
    }
  }
  if (encDT!=0 || encPushed) {
    // update the timeout timer since some input was recieved
    guiPrevActionTime == curTime;
  }
  // Take actions based on encoder input
  if ( encDT != 0) { // The encoder was turned in either direction
    if (guiParamSelected == -1) {
      // no parameter is selected so select the first
      guiParamSelected = 3;
      gui.items[guiParamSelected].elem.highlight = true;
      
    }
    else {
      // A parameter is activly selected or highlighted
      if (gui.items[guiParamSelected].elem.highlight) {
        // the parameter is only highlighted
        // turning encoder should move to the next parameter
        guiParamSelected += encDT;
        // guiParamSelected should always be in the range 3+[0-5]
        if (guiParamSelected == 2)       guiParamSelected = 8;
        else if (guiParamSelected == 9)  guiParamSelected = 2;  
      }
      else if (gui.items[guiParamSelected].elem.selected) {
        // the parmater is selected and the value should be changed
        gui.change_value(guiParamSelected, encDT);
      }
    }
  }
  else if (encPushed) {
    if (gui.items[guiParamSelected].elem.highlight) {
      gui.items[guiParamSelected].elem.highlight = false;
      gui.items[guiParamSelected].elem.selected = true;
    }
    else if (gui.items[guiParamSelected].elem.selected) 
    {
      gui.items[guiParamSelected].elem.highlight = true;
      gui.items[guiParamSelected].elem.selected = false;
    }
  }
  
  // Update sensor parameters at nominal 1Hz
  if ( (curTime - measTime) > 1000 ){
    // Take a sudo measurments of PEEP/PIP/Vol
    for(unsigned i=0;i<3;i++) {
      parms[i]=get_rand(gui_value[i].min, gui_value[i].max);
    }       
    Serial.print("Measure ");
    for(unsigned int i=0;i<nParam;i++) {
      sprintf(s, gui_value[i].fmt, *gui_value[i].val);
      Serial.print(s);
      Serial.print(" ");
    }
    Serial.print("\n");
    update_display();
    measTime=curTime;
  }
  
}
