/**
 *  Einbinden spezieller Libraries, die wir benötigen
 */
#include <Arduino.h>
#include <Servo.h>

/**
 *  Definition der Noten-Positionen (Winkel)
 */
// const int C = 120;
// const int D = 110;
// const int E = 102;
// const int F = 93;
// const int G = 85;
// const int A = 75;
// const int B = 68; // manchmal auch als "H" bezeichnet!
// const int H = 68;
// const int C1 = 58;

enum eNOTEN
{
  C,
  D,
  E,
  F,
  G,
  A,
  B,
  H = B,
  C1,
  NOTE_LAST
};

const int iC = 100;
const int iC1 = 45;

int16_t g_arrNote[NOTE_LAST];

/**
 * Variable zum Speichern des Tempo/Taktes
 * Diese wird zur Laufzeit überschrieben mit dem tempo aus dem Melodie-Array
 */
int takt = 100;

/**
 * spezielle Note, welche eine Pause repräsentiert
 */
const int PAUSE_1 = 1010;
const int PAUSE_2 = 1020;
const int PAUSE_3 = 1030;
const int PAUSE_4 = 1040;
const int PAUSE_5 = 1050;
const int PAUSE_6 = 1060;
const int PAUSE_7 = 1070;

const int ENDE = 9999;

int pause_1 = 50;
int pause_2 = 100;
int pause_3 = 200;
int pause_4 = 300;
int pause_5 = 400;
int pause_6 = 500;
int pause_7 = 600;

/**
 * Array mit Noten = Melodie
 *
 * An erster Stelle steht das Tempo/der Takt
 */
// Test
int TEST[] = {300, C, D, E, F, G, A, B, C1, ENDE};

// Melodie: Star Wars Imperial March, Part 1
int MARSCH[] = {250, E, E, E, C, G, E, C, G, E, PAUSE_4, B, B, B, C1, G, E, C, G, E, ENDE};

// Scooter
int SCOOTER[] = {50, E, PAUSE_2, H, PAUSE_2, E, PAUSE_2, H, A, G, PAUSE_2, G, PAUSE_2, G, PAUSE_4, H, G, PAUSE_2, H, PAUSE_2, G, PAUSE_2, H, G, E, PAUSE_2, E, PAUSE_2, E, ENDE};

int NARCOTIC[] = {1000, E, PAUSE_2, D, PAUSE_2, E, PAUSE_2, D, PAUSE_2, E, PAUSE_2, D, PAUSE_2, E, PAUSE_2, D, ENDE};

// Star Wars Intro, Part 1
int START_WARS[] = {10, C, PAUSE_6, G, PAUSE_6, F, E, D, C1, PAUSE_6, G, PAUSE_6, F, E, D, C1, PAUSE_6, G, PAUSE_6, F, E, F, D, PAUSE_7, D, D, D, D, D, D, C, PAUSE_6, G, PAUSE_6, F, E, D, C1, PAUSE_6, G, PAUSE_6, F, E, D, C1, PAUSE_6, G, PAUSE_6, F, E, F, D, ENDE};

int *g_arrSongs[] = {TEST, NARCOTIC, MARSCH, SCOOTER, START_WARS, NULL};
int g_iCurrentSong = 0;

/**
 * dynamische Werte
 */
const int pauseWinkelVertikal = 173;
const int pauseWinkelHorizontal = 90;

const int startWinkelVertikal = 68;
const int winkelZumSchlagen = 30;

/**
 * Initialisieren der Servo-Motoren
 */
Servo servoHorizontal;
Servo servoVertikal;

/**
 * Definition der Pins des Arduino, wo die Servos und der taster angeschlossen sind
 */
const int servoPinHorizontal = 10;
const int servoPinVertikal = 9;
const int buttonPin = 2;

/**
 * Eigene Funktionen
 */

void spieleTon(int ton)
{

  // den aktuellen Winkel des Servos auslesen
  // int aktuellerWinkel = servoHorizontal.read();

  servoHorizontal.write(g_arrNote[ton]);

  // delay(abs(ton - aktuellerWinkel) * 6);
  delay(150);

  // Note anschlagen
  servoVertikal.write(startWinkelVertikal - winkelZumSchlagen);
  delay(20);
  servoVertikal.write(startWinkelVertikal);
  delay(20);
}

void spieleEnde()
{
  servoHorizontal.write(g_arrNote[C]);
  delay(100);
  servoVertikal.write(startWinkelVertikal - 8);
  delay(100);
  for (int winkel = g_arrNote[C]; winkel >= g_arrNote[C1]; winkel--)
  {
    servoHorizontal.write(winkel);
    delay(5);
  }
  delay(500);
  servoVertikal.write(startWinkelVertikal + winkelZumSchlagen);
  delay(100);
  servoVertikal.write(startWinkelVertikal - winkelZumSchlagen);
  delay(20);
  servoVertikal.write(startWinkelVertikal);
  delay(300);
}

/**
 * setup() wird beim Einschalten des Arduino einmalig aufgerufen
 */
void setup()
{
  Serial.begin(115200);
  // initialen Servo-Wert setzen
  servoHorizontal.write(pauseWinkelHorizontal);
  servoVertikal.write(pauseWinkelVertikal);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  // initialisiere Noten array
  float fSpace = (iC - iC1) / C1;
  for (int i = 0; i < NOTE_LAST; i++)
  {
    g_arrNote[i] = iC - (i * fSpace);
    Serial.println(g_arrNote[i]);
  }
}

void playSong(int *pSong)
{
  // Servos zu den Arduino Pins zuordnen
  servoHorizontal.attach(servoPinHorizontal);
  servoVertikal.attach(servoPinVertikal);

  // Servos auf Startposition fahren
  servoHorizontal.write(pauseWinkelHorizontal);
  delay(15);
  for (int winkel = pauseWinkelVertikal; winkel >= startWinkelVertikal; winkel--)
  {
    servoVertikal.write(winkel);
    delay(15);
  }

  takt = pSong[0]; // erster Wert im Array

  delay(500);

  int position = 1;
  while (pSong[position] != ENDE)
  {
    int ton = pSong[position];

    if (ton == PAUSE_1)
    {
      delay(pause_1);
    }
    else if (ton == PAUSE_2)
    {
      delay(pause_2);
    }
    else if (ton == PAUSE_3)
    {
      delay(pause_3);
    }
    else if (ton == PAUSE_4)
    {
      delay(pause_4);
    }
    else if (ton == PAUSE_5)
    {
      delay(pause_5);
    }
    else if (ton == PAUSE_6)
    {
      delay(pause_6);
    }
    else if (ton == PAUSE_7)
    {
      delay(pause_7);
    }
    else
    {

      spieleTon(ton);

      delay(takt);
    }
    position++;
  }

  delay(pause_7);

  spieleEnde();

  // Servos wieder zurück in Parkposition fahren
  servoHorizontal.write(pauseWinkelHorizontal);
  for (int winkel = startWinkelVertikal; winkel <= pauseWinkelVertikal; winkel++)
  {
    servoVertikal.write(winkel);
    delay(15);
  }

  // Servos von den Arduino Pins trennen, damit diese nicht dauerhaft "brummen"
  // (...Impulse vom Arduino gesendet bekommen)
  servoHorizontal.detach();
  servoVertikal.detach();
}

/**
 * loop() wird in einer Schleife aufgerufen, solange der Arduino Strom hat
 */
void loop()
{

  // Lesen des Wertes des Drucktasters
  int buttonState = digitalRead(buttonPin);

  // Überprüfen, ob der Drucktaster gedrückt ist. Wenn dies der Fall ist, ist der ButtonState auf HIGH
  if (buttonState == HIGH)
  {
    if (g_arrSongs[g_iCurrentSong] != NULL)
    {
      playSong(g_arrSongs[g_iCurrentSong]);
      g_iCurrentSong++;
    }
    else
    {
      g_iCurrentSong = 0;
      playSong(g_arrSongs[g_iCurrentSong]);
      g_iCurrentSong++;
    }
  }
}
