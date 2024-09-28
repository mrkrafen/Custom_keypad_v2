//Scritto da: Alessandro Mammino
//mail: krafen885@hotmail.com
//ultima: la più libera possibile, ognuno faccia quello che vuole. GNU GPL

#include <Bounce2.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <Keypad.h>

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// impostazioni relative allo schermo, sperando che siano corrette

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//JOYSTICK PINS
#define vertJoyPIN A2
#define horJoyPIN A3
#define buttonJoyPIN 4
//JOYSTICK SETUP
#define analogDeadBand 20
#define sensitivityScroll 18000// il CONTRARIO della sensitivity (DEFAULT 15000)
#define sensitivityArrow 20000
#define singleScroll 1
//KEYPAD MATRIX CHARACTERISTICS
#define ROWS 7
#define COLS 3

// Instantiate a Bounce object
Bounce debouncer = Bounce();
// Definizione dei label tasti per la  matrice tasti.
char keys[ROWS][COLS] = {
  {'0', '1', '2'},
  {'3', '4', '5'},
  {'6', '7', '8'},
  {'9', 'A', 'B'},
  {'C', 'D', 'E'},
  {'F', 'G', 'H'},
  {'I', 'L', 'M'}
};

byte rowPins[ROWS] = {A1 , A0 , 15 , 14 , 16 , 10, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5 , 6 , 7}; //connect to the column pinouts of the keypad

int vertPin = 0;  // Analog output of vertical joystick pin
int selPin = 2;  // select button pin of joystick

String jState = "Wheel";
String nState = "Mcro";

bool joyConf = false;
bool screenUpdate = true;
bool clickDisplay = false;
bool clrScr = false;

int vertZero, horZero; // Stores the initial value of each axis, usually around 512
int vertRawValue, horRawValue;
int scrollPeriod;

unsigned long lastVertScroll = 0, lastHorScroll = 0;
unsigned long keyStrCount = 0, ScrollCount = 0;
unsigned long printTimerEnd = 0;

int mouseClickFlag = 0;
int keyFuncState = 0;
char numBak1 = 'w';
char numBak2 = 'w';
char numBak3 = 'w';

//INIZIALIZE KEYPAD MATRIX

Keypad kpad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//FINE DELLA DICHIARAZIONE DI VARIABILI.
void setup()
{
  pinMode(vertJoyPIN, INPUT);  // set button select pin as input
  pinMode(horJoyPIN, INPUT);  // set button select pin as input
  pinMode(buttonJoyPIN, INPUT_PULLUP);  // set button select pin as input

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(buttonJoyPIN);
  debouncer.interval(5); // interval in ms

  delay(100);  // short delay to let outputs settle
  vertZero = analogRead(vertJoyPIN);  // lettura del valore zero per il joystick orizzontale. non muovere lo stick all'accensione
  horZero = analogRead(horJoyPIN);  // lettura del valore zero per il joystick verticale. non muovere lo stick all'accensione

  //inizializzazione schermo da mettere qui
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  
  //IMPOSTO LA ROTAZIONE VERTICALE DELLO SCHERMO. (valori possibili di setRotation: 0, 1, 2, 3
  display.setRotation(3);  
  //FINE IMPOSTAZIONE SCHERMO

  Serial.begin(9600);

  //RESET KEYBOARD
  Keyboard.begin();
  Keyboard.releaseAll();
}

void loop()
{
  // Update the Bounce instance :
  debouncer.update();
  //read raw values
  vertRawValue = analogRead(vertJoyPIN);
  horRawValue = analogRead(horJoyPIN);
  
  //rilevo se è stato cliccato il pulsante centrale del joystick.
  if (debouncer.rose())
  {
    joyConf = !joyConf;
    if (joyConf == 0)
    {
      jState = "Wheel";
      screenUpdate = true;
    }
    if (joyConf == 1)
    {
      jState = "Arrow";
      screenUpdate = true;
    }
    Serial.println(joyConf);
  }

  //############################GESTIONE ASSE VERTICALE############################
  if (abs(vertRawValue - vertZero) > analogDeadBand)
  {
    //joystick horizz mosso in modo ponderato
    if (!joyConf)
    {
      scrollPeriod = abs(sensitivityScroll / (vertRawValue - vertZero));
      if ((millis() - lastVertScroll) >= scrollPeriod)                            //#
      {
        if ((vertRawValue - vertZero) > 0)                                        //#
        {
          Mouse.move(0, 0, -singleScroll);
          ScrollCount = ScrollCount + singleScroll;
        }
        else
        {
          Mouse.move(0, 0, +singleScroll);
          ScrollCount = ScrollCount + singleScroll;
        }
        lastVertScroll = millis();
      }
    }
    else
    {
      //frecce
      scrollPeriod = abs(sensitivityArrow / (vertRawValue - vertZero));
      if ((millis() - lastVertScroll) >= scrollPeriod)                            //#
      {
        if ((vertRawValue - vertZero) > 0)                                        //#
        {
          Keyboard.press(KEY_DOWN_ARROW);
          Keyboard.release(KEY_DOWN_ARROW);
        }
        else
        {
          Keyboard.press(KEY_UP_ARROW);
          Keyboard.release(KEY_UP_ARROW);
        }
        lastVertScroll = millis();
      }
    }

  }

  //##############################################################################

  //############################GESTIONE ASSE ORIZZONTALE#########################
  if (abs(horRawValue - horZero) > analogDeadBand)
  {
    //joystick horizz mosso in modo ponderato
    if (!joyConf)
    {
      scrollPeriod = abs(sensitivityArrow / (horRawValue - horZero));
      if ((millis() - lastHorScroll) >= scrollPeriod)                             //#
      {
        if ((horRawValue - horZero) > 0)                                        //#
        {
          Mouse.move(0, 0, +singleScroll*3);
          ScrollCount = ScrollCount + singleScroll*3;
        }
        else
        {
          Mouse.move(0, 0, -singleScroll*3);
          ScrollCount = ScrollCount + singleScroll*3;
        }
        lastHorScroll = millis();
      }
    }
    else
    {
      scrollPeriod = abs(sensitivityArrow / (horRawValue - horZero));
      if ((millis() - lastHorScroll) >= scrollPeriod)                            //#
      {
        if ((horRawValue - horZero) > 0)                                        //#
        {
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_ARROW);
        }
        else
        {
          Keyboard.press(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_RIGHT_ARROW);
        }
        lastHorScroll = millis();
      }

    }
  }

  //##############################################################################
  //###########################KEYPAD GESTIONE ###################################

  if (kpad.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpad.key[i].stateChanged )   // Only find keys that have changed state.
      {
        //###########################ALLOCAZIONE MACRO E TASTI#########################
        char tasto = (char)kpad.key[i].kchar;
        Serial.println(tasto);
		
		//CONTATORE CLICK: aumento il contatore solo nel caso di case: PRESSED
		switch (kpad.key[i].kstate) {
			case PRESSED:
			strCountIncr();
			break;
			case RELEASED:
			break;
			case IDLE:
			break;
			case HOLD:
			break;
		}
		//fine della parentesi contatore
		
		//inizio con la definizione dei vari pulsanti		
        if (tasto == '2')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press('=');
              numpad_print('=');	
              break;
            case RELEASED:
			        Keyboard.release('=');
              break;
            case IDLE:
              break;
            case HOLD:
			        printTimer();
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '1')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
			        funcSwitch();  //cambio modalità tra numpad ed il cad di turno
              //Keyboard.press(176); // tasto ENTER era la vecchia funzione
              break;
            case RELEASED:
              //Keyboard.release(176);
              break;
            case IDLE:
              break;
            case HOLD:
              printTimer();  //mi mostra quante volte ho pigiato i tasti
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '0')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
			  if (keyFuncState == 0)
              {
                Keyboard.press(KEY_LEFT_CTRL); //PAN in Solidworks
                Mouse.press(MOUSE_MIDDLE);
              }
              if (keyFuncState == 1){Keyboard.press('/');numpad_print('/');}
              break;
            case RELEASED:
              if (keyFuncState == 0)
              {
                Keyboard.release(KEY_LEFT_CTRL); //PAN in Solidworks
                Mouse.release(MOUSE_MIDDLE);
              }
              if (keyFuncState == 1){Keyboard.release('/');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '3')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0){Mouse.press(MOUSE_MIDDLE);}
              if (keyFuncState == 1){Keyboard.press('*');numpad_print('*');}
              break;
            case RELEASED:
              if (keyFuncState == 0){Mouse.release(MOUSE_MIDDLE);}
              if (keyFuncState == 1){Keyboard.release('*');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '4')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.print("line");
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('-');numpad_print('-');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('-');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '5')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.print("circle");
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('7');numpad_print('7');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('7');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '6')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
				Keyboard.print("dimlinear"); //powerquote di AUTOCAD
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();                
              }
              if (keyFuncState == 1){Keyboard.press('(');numpad_print('(');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('(');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '7')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
 				Keyboard.print("extend"); //estendi in AUTOCAD
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press(')');numpad_print(')');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release(')');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '8')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0){Keyboard.press(KEY_F8);}
              if (keyFuncState == 1){Keyboard.press('^');numpad_print('^');} //ho messo il simbolo della potenza perchè è già compatibile con il keycap messo e perchè ha davvero senso su excel
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
			  if (keyFuncState == 0){Keyboard.release(KEY_F8);}
              if (keyFuncState == 1){Keyboard.release('^');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == '9')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
				Keyboard.print("trim"); //taglia in AUTOCAD. per accorciare le linee
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('8');numpad_print('8');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('8');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'A')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
				Keyboard.press(KEY_LEFT_CTRL);
                Keyboard.press('c'); //C COPIA
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('9');numpad_print('9');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('9');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'B')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.press(KEY_LEFT_CTRL);
                Keyboard.press('v'); //V INCOLLA
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('+');numpad_print('+');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('+');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'C')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0){Keyboard.press(KEY_LEFT_SHIFT);}
              if (keyFuncState == 1){Keyboard.press('4');numpad_print('4');}
              break;
            case RELEASED:
              if (keyFuncState == 0){Keyboard.release(KEY_LEFT_SHIFT);}
              if (keyFuncState == 1){Keyboard.release('4');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'D')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.press(KEY_LEFT_CTRL);
                Keyboard.press('z'); //Z AZIONE PRECEDENTE
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('5');numpad_print('5');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('5');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'E')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0){Keyboard.press(KEY_DELETE);}
              if (keyFuncState == 1){Keyboard.press('6');numpad_print('6');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 0){Keyboard.release(KEY_DELETE);}
              if (keyFuncState == 1){Keyboard.release('6');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'F')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.print("rotate");
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('1');
              numpad_print('1');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('1');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'G')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.print("mirror"); //comando specchio autocad
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('2');
              numpad_print('2');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('2');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'H')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0){
                Keyboard.print("scale");
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('3');
              numpad_print('3');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('3');}
              break;
          }
        }
		//$$$ tasto successivo $$$
		if (tasto == 'I')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
                if (keyFuncState == 0){Mouse.press(KEY_LEFT_CTRL);}
                if (keyFuncState == 1){Keyboard.press('0');numpad_print('0');}
              break;
            case RELEASED:
                if (keyFuncState == 0){Mouse.release(KEY_LEFT_CTRL);}
                if (keyFuncState == 1){Keyboard.release('0');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'L')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0)
              {
                Keyboard.print("movecopy");
                delay(50);
                Keyboard.press(176);  //CAMBIA TAB
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press('.');
              numpad_print('.');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release('.');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        if (tasto == 'M')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              if (keyFuncState == 0){
                Keyboard.print("move");
                delay(50);
                Keyboard.press(176);  
                delay(50);
                Keyboard.releaseAll();
              }
              if (keyFuncState == 1){Keyboard.press(',');
              numpad_print(',');}
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              if (keyFuncState == 1){Keyboard.release(',');}
              break;
          }
        }
		//$$$ tasto successivo $$$
        //#######################################FINE IMPOSTAZIONE#################################
      }

    }
  }

  //###########################STAMPA A SCHERMO###################################
  
  if (millis() > printTimerEnd)
  {
	clickDisplay = false;
	screenUpdate = true;
  }
  
  if (screenUpdate)
  {
    joystatPrt();
  }
  
  screenUpdate = false;
}


//####################### DEFINIZIONE FUNZIONI #############################


//####################### FUNZIONE STAMPA A SCHERMO STATO JOYSTICK ########################
void joystatPrt()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0); 
  display.println("JOY");
  display.setCursor(0,10);
  display.println(jState);
  display.setCursor(0,25);
  display.println("KEYs"); 
  
  switch (keyFuncState)
  {
    case 0:
      nState = "mA";
      break;
    case 1:
      nState = "Nu";
      break;
  }
  display.setCursor(0,35);
  display.setTextSize(3);
  display.println(nState);
  
  //inserire stampa del contatore di click:
  //voglio che si attivi una volta schiacciato il tasto F8 in HOLD.
  //deve stampare il numero con un titoletto e poi sparire dopo 5 secondi.
  
  if (clickDisplay)
	{
    display.setTextSize(1);
	display.setCursor(0,88);
    display.println("Cl/sc");
	display.setCursor(0,98);
    display.println(keyStrCount);
  display.setCursor(0,110);
    //ScrollCountPrint(ScrollCount);
    display.println(ScrollCount);
	clickDisplay = false;
	}
  
  //effettivo comando di output a schermo
  display.display();
}

//funzione stampa numero digitato
void numpad_print(char numb)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.setTextSize(4);
  display.println(numb);
  display.setCursor(0,35);
  display.setTextSize(1);
  display.println("old");  
  display.setCursor(0,46);
  display.setTextSize(1);
  display.println(numBak1);
  display.setCursor(0,62);
  display.setTextSize(1);
  display.println(numBak2);
  display.setCursor(0,78);
  display.setTextSize(1);
  display.println(numBak3);
  display.setTextSize(1);
  display.setCursor(0,100);
  display.println("Cl");
  display.setCursor(0,110);
  display.println(keyStrCount);
  display.display();
  numBak3 = numBak2;
  numBak2 = numBak1;
  numBak1 = numb;
  printTimer2();
}

//funzione cambio mode tra numpad e software.. per ora
void funcSwitch()
{
  if (keyFuncState>=1)
  {
    keyFuncState = 0;
    screenUpdate = true;
  }
  else 
  {
    ++keyFuncState;
    screenUpdate = true;
  }
}

//funzione incremento variabile conta click
void strCountIncr()
{
  ++keyStrCount;
}

//funzione inizializzazione timer stampa contatore click
void printTimer()
{
  screenUpdate = true;
  clickDisplay = true;
  printTimerEnd = millis() + 2000;
}  

void printTimer2()
{
  printTimerEnd = millis() + 2000;
}  

//accorciamento numero analisi scrollate
//void ScrollCountPrint(unsigned long value)
//{
//  if (value<=99999)
//  {
//    display.println(value);
//  }
//  else 
//  {
//    display.print(value/1000);
//    display.println('m');
//  }
//}  
