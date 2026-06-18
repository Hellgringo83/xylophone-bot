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

const int iC = 126;
const int iC1 = 60;

int16_t g_arrNote[NOTE_LAST];

/**
 * Musikalisches Liedformat.
 *
 * Der Notenwert ist der Nenner der Notenlaenge:
 *   1 = ganze Note, 2 = halbe, 4 = Viertel, 8 = Achtel, 16 = Sechzehntel
 *
 * BPM bezieht sich auf den unteren Wert der Taktart. Bei 4/4 sind es also
 * Viertelschlaege pro Minute, bei 3/8 Achtelschlaege pro Minute.
 */
const int8_t REST = -1;

struct NoteEvent
{
  int8_t note;
  uint8_t length;
};

struct Song
{
  const char *name;
  bool nameInProgramMemory;
  uint16_t bpm;
  uint8_t beatsPerMeasure;
  uint8_t beatUnit;
  const NoteEvent *events;
  size_t eventCount;
};

/**
 * Auch die feste Playlist verwendet ausschliesslich das Terminal-Stringformat.
 * Namen und Notationen liegen im Flash (PROGMEM), damit sie auf dem Uno keinen
 * kostbaren Arbeitsspeicher belegen.
 */
struct PlaylistEntry
{
  const char *name;
  const char *notation;
};

const char NAME_ENTCHEN[] PROGMEM = "Alle meine Entchen";
const char SONG_ENTCHEN[] PROGMEM = "200,4/4,c4d4e4f4g2g2a4a4a4a4g1a4a4a4a4g1f4f4f4f4e2e2d4d4d4d4c1";
const char NAME_FREUDE[] PROGMEM = "Freude schoener Goetterfunke";
const char SONG_FREUDE[] PROGMEM = "120,4/4,e4e4f4g4g4f4e4e4d4c4c4d4e4e2d2d2r2e4e4f4g4g4f4e4d4c4c4d4e4d4c4c2";
const char NAME_THUNDERSTRUCK[] PROGMEM = "Thunderstruck";
const char SONG_THUNDERSTRUCK[] PROGMEM = "100,4/4,d8b8f8b8e8b8f8b8d8b8f8b8e8b8f8b8r4";
const char NAME_HAENSCHEN[] PROGMEM = "Haenschen Klein";
const char SONG_HAENSCHEN[] PROGMEM = "100,4/4,g4e4e2f4d4d2c4d4e4f4g4g4g2r4g4e4e2f4d4d2c4e4g4g4c2r4c4c4c2";
const char NAME_TERMINATOR[] PROGMEM = "Terminator";
const char SONG_TERMINATOR[] PROGMEM = "124,4/4,e4e8g8e8d8c4b4e4e8g8e8d8c8d8c4b4";
const char NAME_AXEL_F[] PROGMEM = "Axel F";
const char SONG_AXEL_F[] PROGMEM = "124,4/4,e4e8g8e8d8c4b4e4e8g8e8d8c8d8c4b4";
const char NAME_SEVEN_NATION_ARMY[] PROGMEM = "Seven Nation Army";
const char SONG_SEVEN_NATION_ARMY[] PROGMEM = "200,4/4,a2a4C4a4g4f1e1a2a4C4a4g4f4g4f4e1a2a4C4a4g4f1e1a2a4C4a4g4f4g4f4e4";
const char NAME_INSOMNIA[] PROGMEM = "Insomnia";
const char SONG_INSOMNIA[] PROGMEM = "200,4/4,d4d4d8c8c8c8b2";
const char NAME_NOT_HUMAN[] PROGMEM = "Not Human";
const char SONG_NOT_HUMAN[] PROGMEM = "180,4/4,c8C8d8b8e8a8f8g8C8c8b8d8a8e8g8f8c8g8C8e8b8d8a8f8C8c8a8e8b8d8g8f8";
const char NAME_MISSION_IMPOSSIBLE[] PROGMEM = "Mission Impossible";
const char SONG_MISSION_IMPOSSIBLE[] PROGMEM = "100,5/4,g8g8b8C8g8g8f8g8g8g8e8f8g8g8b8C8g8g8f8g8";
const char NAME_INDIANA_JONES[] PROGMEM = "Indiana Jones";
const char SONG_INDIANA_JONES[] PROGMEM = "100,4/4,e4f8g8C2r4d4e8f8g2r4a4b8C8f2";

const PlaylistEntry PLAYLIST[] PROGMEM = {
    {NAME_ENTCHEN, SONG_ENTCHEN},
    {NAME_FREUDE, SONG_FREUDE},
    {NAME_THUNDERSTRUCK, SONG_THUNDERSTRUCK},
    {NAME_HAENSCHEN, SONG_HAENSCHEN},
    {NAME_TERMINATOR, SONG_TERMINATOR},
    {NAME_AXEL_F, SONG_AXEL_F},
    {NAME_SEVEN_NATION_ARMY, SONG_SEVEN_NATION_ARMY},
    {NAME_INSOMNIA, SONG_INSOMNIA},
    {NAME_NOT_HUMAN, SONG_NOT_HUMAN},
    {NAME_MISSION_IMPOSSIBLE, SONG_MISSION_IMPOSSIBLE},
    {NAME_INDIANA_JONES, SONG_INDIANA_JONES}};

const size_t PLAYLIST_SIZE = sizeof(PLAYLIST) / sizeof(PLAYLIST[0]);
size_t g_iCurrentSong = 0;

/**
 * dynamische Werte
 */
const int pauseWinkelSchlaegel = 180;
const int pauseWinkelDrehung = 90;

const int startWinkelSchlaegel = 98;
const int schlagWinkelSchlaegel = 22;
const uint16_t notePositionTimeMs = 150;
const uint16_t hammerDownTimeMs = 80;
const uint16_t hammerReturnTimeMs = 20;
const uint16_t noteStrikeTimeMs =
    notePositionTimeMs + hammerDownTimeMs + hammerReturnTimeMs;

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

// Platz fuer laengere Play-Folgen, z.B. komplette Melodieabschnitte.
const size_t terminalBufferSize = 128;
char terminalBuffer[terminalBufferSize];
size_t terminalBufferPosition = 0;
const size_t terminalEventCapacity = 56;
NoteEvent terminalEvents[terminalEventCapacity];

/**
 * Eigene Funktionen
 */

void printTerminalHelp()
{
  Serial.println(F("UART-Diagnose:"));
  Serial.println(F("  d <0-180>         Drehservo (Pin 10)"));
  Serial.println(F("  s <0-180>         Schlaegelservo (Pin 9)"));
  Serial.println(F("  play BPM,TAKT,NOTEN"));
  Serial.println(F("    z.B. play 120,4/4,c4d4e8r8C2"));
  Serial.println(F("    Kurzform: p120,4/4,c4d4e8r8C2"));
  Serial.println(F("    Noten: c d e f g a b/h, hohes C: C, Pause: r"));
  Serial.println(F("    Laengen: 1, 2, 4, 8 oder 16"));
  Serial.println(F("  status             Servozustand anzeigen"));
  Serial.println(F("  detach <d|s|all>   Servo(s) abschalten"));
  Serial.println(F("  help               Hilfe anzeigen"));
}

void printServoStatus(char servoCommand, Servo &servo, int pin)
{
  Serial.print(F("Servo "));
  Serial.print(servoCommand);
  Serial.print(F(": Pin "));
  Serial.print(pin);
  Serial.print(F(", Winkel "));
  Serial.print(servo.read());
  Serial.print(F(" Grad, "));
  Serial.println(servo.attached() ? F("aktiv") : F("abgeschaltet"));
}

Servo *getServo(char servoCommand, int &pin)
{
  if (servoCommand == 'd')
  {
    pin = pinServoDrehung;
    return &servoDrehung;
  }

  if (servoCommand == 's')
  {
    pin = pinServoSchlaegel;
    return &servoSchlaegel;
  }

  return NULL;
}

void spieleTon(int ton);
void playSong(const Song &song);
bool playPlaylistSong(size_t playlistIndex);

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

bool isValidNoteLength(long length)
{
  return length == 1 || length == 2 || length == 4 ||
         length == 8 || length == 16;
}

bool parseTerminalSong(const char *notation, Song &song)
{
  if (notation == NULL || *notation == '\0')
  {
    return false;
  }

  char *end;
  long bpm = strtol(notation, &end, 10);
  if (end == notation || *end != ',' || bpm < 1 || bpm > 999)
  {
    return false;
  }

  const char *position = end + 1;
  long beatsPerMeasure = strtol(position, &end, 10);
  if (end == position || *end != '/' ||
      beatsPerMeasure < 1 || beatsPerMeasure > 32)
  {
    return false;
  }

  position = end + 1;
  long beatUnit = strtol(position, &end, 10);
  if (end == position || *end != ',' || !isValidNoteLength(beatUnit))
  {
    return false;
  }

  position = end + 1;
  size_t eventCount = 0;

  while (*position != '\0')
  {
    if (eventCount >= terminalEventCapacity)
    {
      return false;
    }

    bool isRest = *position == 'r' || *position == 'R';
    int note = isRest ? REST : noteFromCharacter(*position);
    if (!isRest && note < 0)
    {
      return false;
    }

    position++;
    long length = strtol(position, &end, 10);
    if (end == position || !isValidNoteLength(length))
    {
      return false;
    }

    terminalEvents[eventCount].note = note;
    terminalEvents[eventCount].length = length;
    eventCount++;
    position = end;
  }

  if (eventCount == 0)
  {
    return false;
  }

  song.name = "Terminal";
  song.nameInProgramMemory = false;
  song.bpm = bpm;
  song.beatsPerMeasure = beatsPerMeasure;
  song.beatUnit = beatUnit;
  song.events = terminalEvents;
  song.eventCount = eventCount;
  return true;
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
    printServoStatus('d', servoDrehung, pinServoDrehung);
    printServoStatus('s', servoSchlaegel, pinServoSchlaegel);
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
    Song terminalSong;
    if (!parseTerminalSong(melody, terminalSong))
    {
      Serial.println(F("Fehler: Format BPM,TAKT,NOTEN erwartet."));
      Serial.println(F("Beispiel: play 120,4/4,c4d4e8r8C2"));
      return;
    }

    playSong(terminalSong);
    return;
  }

  if (strcmp(commandName, "play") == 0 ||
      (commandName[0] == 'p' && commandName[1] != '\0'))
  {
    Serial.println(F("Fehler: Verwendung: play 120,4/4,c4d4e8r8C2"));
    return;
  }

  if (strcmp(commandName, "d") == 0 || strcmp(commandName, "s") == 0)
  {
    char *angleText = strtok(NULL, " \t");
    char *extraText = strtok(NULL, " \t");

    if (angleText == NULL || extraText != NULL)
    {
      Serial.println(F("Fehler: Verwendung: d <0-180> oder s <0-180>"));
      return;
    }

    char *angleEnd;
    long angle = strtol(angleText, &angleEnd, 10);

    int pin = 0;
    Servo *servo = getServo(commandName[0], pin);

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
    Serial.print(commandName);
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
      Serial.println(F("Fehler: Verwendung: detach <d|s|all>"));
      return;
    }

    if (strcmp(servoText, "all") == 0)
    {
      servoDrehung.detach();
      servoSchlaegel.detach();
      Serial.println(F("OK: Beide Servos abgeschaltet."));
      return;
    }

    int pin = 0;
    Servo *servo = strlen(servoText) == 1 ? getServo(servoText[0], pin) : NULL;
    if (servo == NULL)
    {
      Serial.println(F("Fehler: Servo muss d oder s sein."));
      return;
    }

    servo->detach();
    Serial.print(F("OK: Servo "));
    Serial.print(servoText);
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

  servoDrehung.write(g_arrNote[ton]);

  // delay(abs(ton - aktuellerWinkel) * 6);
  delay(notePositionTimeMs);

  // Note anschlagen
  servoSchlaegel.write(startWinkelSchlaegel - schlagWinkelSchlaegel);
  delay(hammerDownTimeMs);
  servoSchlaegel.write(startWinkelSchlaegel);
  delay(hammerReturnTimeMs);
}

void spieleEnde()
{
  servoDrehung.write(g_arrNote[C]);
  delay(100);
  int iBlub = 8;
  iBlub = 14;
  servoSchlaegel.write(startWinkelSchlaegel - iBlub);
  delay(100);
  for (int winkel = g_arrNote[C]; winkel >= g_arrNote[C1]; winkel--)
  {
    servoDrehung.write(winkel);
    delay(5);
  }
  servoSchlaegel.write(startWinkelSchlaegel);
  delay(100);
  spieleTon(C1);
  // delay(500);
  // servoSchlaegel.write(startWinkelSchlaegel + schlagWinkelSchlaegel);
  // delay(100);
  // servoSchlaegel.write(startWinkelSchlaegel - schlagWinkelSchlaegel);
  // delay(80);
  // servoSchlaegel.write(startWinkelSchlaegel);
  // delay(300);
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

uint32_t noteDurationMs(const Song &song, uint8_t noteLength)
{
  if (song.bpm == 0 || noteLength == 0)
  {
    return 0;
  }

  // Dauer = Dauer eines Taktschlags * Verhaeltnis Taktnote zu Notenwert.
  return (60000UL * song.beatUnit) / ((uint32_t)song.bpm * noteLength);
}

void waitUntilEventEnds(uint32_t eventStart, uint32_t duration)
{
  uint32_t elapsed = millis() - eventStart;
  if (elapsed < duration)
  {
    delay(duration - elapsed);
  }
}

void playSong(const Song &song)
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

  delay(500);

  Serial.print(F("Spiele \""));
  if (song.nameInProgramMemory)
  {
    Serial.print(reinterpret_cast<const __FlashStringHelper *>(song.name));
  }
  else
  {
    Serial.print(song.name);
  }
  Serial.print(F("\" mit "));
  Serial.print(song.bpm);
  Serial.print(F(" BPM im "));
  Serial.print(song.beatsPerMeasure);
  Serial.print('/');
  Serial.print(song.beatUnit);
  Serial.println(F("-Takt."));

  bool timingWarningPrinted = false;
  for (size_t position = 0; position < song.eventCount; position++)
  {
    const NoteEvent &event = song.events[position];
    uint32_t duration = noteDurationMs(song, event.length);
    uint32_t eventStart = millis();

    if (event.note != REST)
    {
      if (!timingWarningPrinted && duration < noteStrikeTimeMs)
      {
        Serial.print(F("Warnung: Eine Note dauert nur "));
        Serial.print(duration);
        Serial.print(F(" ms, die Servos brauchen mindestens "));
        Serial.print(noteStrikeTimeMs);
        Serial.println(F(" ms. BPM senken oder laengere Noten verwenden."));
        timingWarningPrinted = true;
      }

      spieleTon(event.note);
    }

    // spieleTon() braucht selbst Zeit. Nur die noch fehlende Zeit warten,
    // damit die Abstaende zwischen den Anschlaegen musikalisch korrekt sind.
    waitUntilEventEnds(eventStart, duration);
  }

  delay(noteDurationMs(song, song.beatUnit));

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

bool playPlaylistSong(size_t playlistIndex)
{
  if (playlistIndex >= PLAYLIST_SIZE)
  {
    return false;
  }

  const char *name = reinterpret_cast<const char *>(
      pgm_read_ptr(&PLAYLIST[playlistIndex].name));
  const char *notation = reinterpret_cast<const char *>(
      pgm_read_ptr(&PLAYLIST[playlistIndex].notation));

  size_t notationLength = strlen_P(notation);
  if (notationLength >= terminalBufferSize)
  {
    Serial.println(F("Fehler: Playlist-Eintrag ist zu lang."));
    return false;
  }

  strcpy_P(terminalBuffer, notation);

  Song song;
  if (!parseTerminalSong(terminalBuffer, song))
  {
    Serial.println(F("Fehler: Ungueltiger Playlist-Eintrag."));
    return false;
  }

  song.name = name;
  song.nameInProgramMemory = true;
  playSong(song);
  return true;
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
    playPlaylistSong(g_iCurrentSong);
    g_iCurrentSong = (g_iCurrentSong + 1) % PLAYLIST_SIZE;
  }
}
