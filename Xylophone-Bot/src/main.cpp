/**
 *  Einbinden spezieller Libraries, die wir benötigen
 */
#include <Arduino.h>
#include <Servo.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

const int iC = 103;
const int iC1 = 47;

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

int *g_arrSongs[] = {MARSCH, SCOOTER, START_WARS, NULL};
int g_iCurrentSong = 0;

/**
 * dynamische Werte
 */
const int pauseWinkelSchlaegel = 180;
const int pauseWinkelDrehung = 90;

const int startWinkelSchlaegel = 110;
const int schlagWinkelSchlaegel = 50;

/**
 * Initialisieren der Servo-Motoren
 */
Servo servoDrehung;
Servo servoSchlaegel;

/**
 * Definition der Pins des Arduino, wo die Servos und der taster angeschlossen sind
 */
const int pinServoDrehung = 10;
const int pinServoSchlaegel = 9;
const int buttonPin = 2;

const size_t terminalBufferSize = 40;
char terminalBuffer[terminalBufferSize];
size_t terminalBufferPosition = 0;

/**
 * Eigene Funktionen
 */

void printTerminalHelp()
{
  Serial.println(F("UART-Diagnose:"));
  Serial.println(F("  servo 1 <0-180>  Horizontalservo (Pin 10)"));
  Serial.println(F("  servo 2 <0-180>  Vertikalservo (Pin 9)"));
  Serial.println(F("  p<folge>           Lied spielen, z.B. pa3c1d2"));
  Serial.println(F("  play <folge>       Alternative: play a3c1d2"));
  Serial.println(F("    Noten: c d e f g a b/h, hohes C: C"));
  Serial.println(F("    Pausen: 1 bis 7"));
  Serial.println(F("  status             Servozustand anzeigen"));
  Serial.println(F("  detach <1|2|all>   Servo(s) abschalten"));
  Serial.println(F("  help               Hilfe anzeigen"));
}

void printServoStatus(uint8_t servoNumber, Servo &servo, int pin)
{
  Serial.print(F("Servo "));
  Serial.print(servoNumber);
  Serial.print(F(": Pin "));
  Serial.print(pin);
  Serial.print(F(", Winkel "));
  Serial.print(servo.read());
  Serial.print(F(" Grad, "));
  Serial.println(servo.attached() ? F("aktiv") : F("abgeschaltet"));
}

Servo *getServo(uint8_t servoNumber, int &pin)
{
  if (servoNumber == 1)
  {
    pin = pinServoDrehung;
    return &servoDrehung;
  }

  if (servoNumber == 2)
  {
    pin = pinServoSchlaegel;
    return &servoSchlaegel;
  }

  return NULL;
}

void spieleTon(int ton);

int noteFromCharacter(char character)
{
  if (character == 'C')
  {
    return C1;
  }

  switch (tolower(character))
  {
  case 'c':
    return C;
  case 'd':
    return D;
  case 'e':
    return E;
  case 'f':
    return F;
  case 'g':
    return G;
  case 'a':
    return A;
  case 'b':
  case 'h':
    return B;
  default:
    return -1;
  }
}

void spielePause(uint8_t pauseNumber)
{
  switch (pauseNumber)
  {
  case 1:
    delay(pause_1);
    break;
  case 2:
    delay(pause_2);
    break;
  case 3:
    delay(pause_3);
    break;
  case 4:
    delay(pause_4);
    break;
  case 5:
    delay(pause_5);
    break;
  case 6:
    delay(pause_6);
    break;
  case 7:
    delay(pause_7);
    break;
  }
}

bool isValidMelody(const char *melody)
{
  if (melody == NULL || *melody == '\0')
  {
    return false;
  }

  while (*melody != '\0')
  {
    if (noteFromCharacter(*melody) < 0 &&
        (*melody < '1' || *melody > '7'))
    {
      return false;
    }
    melody++;
  }

  return true;
}

void playTerminalMelody(const char *melody)
{
  servoDrehung.attach(pinServoDrehung);
  servoSchlaegel.attach(pinServoSchlaegel);

  servoDrehung.write(pauseWinkelDrehung);
  servoSchlaegel.write(startWinkelSchlaegel);
  delay(500);

  Serial.print(F("Spiele: "));
  Serial.println(melody);

  while (*melody != '\0')
  {
    int note = noteFromCharacter(*melody);
    if (note >= 0)
    {
      spieleTon(note);
    }
    else
    {
      spielePause(*melody - '0');
    }
    melody++;
  }

  servoDrehung.write(pauseWinkelDrehung);
  servoSchlaegel.write(pauseWinkelSchlaegel);
  delay(500);
  servoDrehung.detach();
  servoSchlaegel.detach();

  Serial.println(F("Lied beendet."));
}

void processTerminalCommand(char *command)
{
  char *commandName = strtok(command, " \t");
  if (commandName == NULL)
  {
    return;
  }

  if (strcmp(commandName, "help") == 0)
  {
    printTerminalHelp();
    return;
  }

  if (strcmp(commandName, "status") == 0)
  {
    printServoStatus(1, servoDrehung, pinServoDrehung);
    printServoStatus(2, servoSchlaegel, pinServoSchlaegel);
    return;
  }

  const char *melody = NULL;
  if (strcmp(commandName, "play") == 0)
  {
    melody = strtok(NULL, " \t");
    if (strtok(NULL, " \t") != NULL)
    {
      melody = NULL;
    }
  }
  else if (commandName[0] == 'p' && commandName[1] != '\0')
  {
    melody = commandName + 1;
    if (strtok(NULL, " \t") != NULL)
    {
      melody = NULL;
    }
  }

  if (melody != NULL)
  {
    if (!isValidMelody(melody))
    {
      Serial.println(F("Fehler: Noten c,d,e,f,g,a,b/h,C und Pausen 1-7 erlaubt."));
      return;
    }

    playTerminalMelody(melody);
    return;
  }

  if (strcmp(commandName, "play") == 0 ||
      (commandName[0] == 'p' && commandName[1] != '\0'))
  {
    Serial.println(F("Fehler: Verwendung: pa3c1d2 oder play a3c1d2"));
    return;
  }

  if (strcmp(commandName, "servo") == 0)
  {
    char *servoText = strtok(NULL, " \t");
    char *angleText = strtok(NULL, " \t");
    char *extraText = strtok(NULL, " \t");

    if (servoText == NULL || angleText == NULL || extraText != NULL)
    {
      Serial.println(F("Fehler: Verwendung: servo <1|2> <0-180>"));
      return;
    }

    char *servoEnd;
    char *angleEnd;
    long servoNumber = strtol(servoText, &servoEnd, 10);
    long angle = strtol(angleText, &angleEnd, 10);

    int pin = 0;
    Servo *servo = getServo(servoNumber, pin);
    if (*servoEnd != '\0' || servo == NULL)
    {
      Serial.println(F("Fehler: Servonummer muss 1 oder 2 sein."));
      return;
    }

    if (*angleEnd != '\0' || angle < 0 || angle > 180)
    {
      Serial.println(F("Fehler: Winkel muss zwischen 0 und 180 Grad liegen."));
      return;
    }

    if (!servo->attached())
    {
      servo->attach(pin);
    }
    servo->write(angle);

    Serial.print(F("OK: Servo "));
    Serial.print(servoNumber);
    Serial.print(F(" faehrt auf "));
    Serial.print(angle);
    Serial.println(F(" Grad."));
    return;
  }

  if (strcmp(commandName, "detach") == 0)
  {
    char *servoText = strtok(NULL, " \t");
    char *extraText = strtok(NULL, " \t");
    if (servoText == NULL || extraText != NULL)
    {
      Serial.println(F("Fehler: Verwendung: detach <1|2|all>"));
      return;
    }

    if (strcmp(servoText, "all") == 0)
    {
      servoDrehung.detach();
      servoSchlaegel.detach();
      Serial.println(F("OK: Beide Servos abgeschaltet."));
      return;
    }

    char *servoEnd;
    long servoNumber = strtol(servoText, &servoEnd, 10);
    int pin = 0;
    Servo *servo = getServo(servoNumber, pin);
    if (*servoEnd != '\0' || servo == NULL)
    {
      Serial.println(F("Fehler: Servonummer muss 1 oder 2 sein."));
      return;
    }

    servo->detach();
    Serial.print(F("OK: Servo "));
    Serial.print(servoNumber);
    Serial.println(F(" abgeschaltet."));
    return;
  }

  Serial.println(F("Unbekanntes Kommando. 'help' zeigt die Befehle."));
}

void readTerminal()
{
  while (Serial.available() > 0)
  {
    char character = Serial.read();

    if (character == '\r' || character == '\n')
    {
      Serial.println();

      if (terminalBufferPosition > 0)
      {
        terminalBuffer[terminalBufferPosition] = '\0';
        processTerminalCommand(terminalBuffer);
        terminalBufferPosition = 0;
      }
      continue;
    }

    if (character == '\b' || character == 127)
    {
      if (terminalBufferPosition > 0)
      {
        terminalBufferPosition--;
        Serial.print(F("\b \b"));
      }
      continue;
    }

    if (terminalBufferPosition < terminalBufferSize - 1)
    {
      terminalBuffer[terminalBufferPosition++] = character;
      Serial.write(character);
    }
    else
    {
      terminalBufferPosition = 0;
      Serial.println();
      Serial.println(F("Fehler: Kommando zu lang."));
    }
  }
}

void spieleTon(int ton)
{

  // den aktuellen Winkel des Servos auslesen
  // int aktuellerWinkel = servoHorizontal.read();

  servoDrehung.write(g_arrNote[ton]);

  // delay(abs(ton - aktuellerWinkel) * 6);
  delay(150);

  // Note anschlagen
  servoSchlaegel.write(startWinkelSchlaegel - schlagWinkelSchlaegel);
  delay(200);
  servoSchlaegel.write(startWinkelSchlaegel);
  delay(20);
}

void spieleEnde()
{
  servoDrehung.write(g_arrNote[C]);
  delay(100);
  servoSchlaegel.write(startWinkelSchlaegel - 8);
  delay(100);
  for (int winkel = g_arrNote[C]; winkel >= g_arrNote[C1]; winkel--)
  {
    servoDrehung.write(winkel);
    delay(5);
  }
  delay(500);
  servoSchlaegel.write(startWinkelSchlaegel + schlagWinkelSchlaegel);
  delay(100);
  servoSchlaegel.write(startWinkelSchlaegel - schlagWinkelSchlaegel);
  delay(20);
  servoSchlaegel.write(startWinkelSchlaegel);
  delay(300);
}

/**
 * setup() wird beim Einschalten des Arduino einmalig aufgerufen
 */
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Xylophone-Bot bereit. 'help' zeigt die UART-Kommandos."));

  // initialen Servo-Wert setzen
  servoDrehung.write(pauseWinkelDrehung);
  servoSchlaegel.write(pauseWinkelSchlaegel);

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
  servoDrehung.attach(pinServoDrehung);
  servoSchlaegel.attach(pinServoSchlaegel);

  // Servos auf Startposition fahren
  servoDrehung.write(pauseWinkelDrehung);
  delay(15);
  for (int winkel = pauseWinkelSchlaegel; winkel >= startWinkelSchlaegel; winkel--)
  {
    servoSchlaegel.write(winkel);
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
  servoDrehung.write(pauseWinkelDrehung);
  for (int winkel = startWinkelSchlaegel; winkel <= pauseWinkelSchlaegel; winkel++)
  {
    servoSchlaegel.write(winkel);
    delay(15);
  }

  // Servos von den Arduino Pins trennen, damit diese nicht dauerhaft "brummen"
  // (...Impulse vom Arduino gesendet bekommen)
  servoDrehung.detach();
  servoSchlaegel.detach();
}

/**
 * loop() wird in einer Schleife aufgerufen, solange der Arduino Strom hat
 */
void loop()
{
  readTerminal();

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
