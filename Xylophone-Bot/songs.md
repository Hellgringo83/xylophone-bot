# Liedformat im Arduino-Code

Ein Lied besteht aus Tempo, Taktart und einer Liste aus Noten beziehungsweise
Pausen:

```cpp
const NoteEvent MEIN_LIED_NOTEN[] = {
    NOTE(C, 4),     // Viertelnote C
    NOTE(D, 8),     // Achtelnote D
    PAUSE(8),       // Achtelpause
    NOTE(E, 2),     // halbe Note E
    NOTE(C1, 1)     // ganze Note, hohes C
};

const Song MEIN_LIED = {
    "Mein Lied",
    120,            // BPM
    4, 4,           // 4/4-Takt
    MEIN_LIED_NOTEN,
    EVENT_COUNT(MEIN_LIED_NOTEN)
};
```

Die erlaubten Notenwerte sind:

- `1`: ganze Note
- `2`: halbe Note
- `4`: Viertelnote
- `8`: Achtelnote
- `16`: Sechzehntelnote

Die BPM beziehen sich auf den unteren Wert der Taktart. Im 4/4-Takt sind
120 BPM also 120 Viertelnoten pro Minute. Im 3/8-Takt wären es 120 Achtelnoten
pro Minute.

Der obere Wert der Taktart beschreibt, wie viele Schläge zu einem Takt gehören.
Er fügt nicht automatisch Pausen ein; Noten und Pausen werden vollständig in
der Ereignisliste notiert.

Ein Anschlag benötigt mit den aktuellen Servo-Verzögerungen mindestens 250 ms.
Ist eine Note bei dem gewählten Tempo kürzer, erscheint eine Warnung im
seriellen Monitor. Beispielsweise sind Sechzehntelnoten bis 60 BPM exakt
spielbar; für schnellere Sechzehntel müsste die Servobewegung beschleunigt
werden.

# UART-/Terminal-Melodien

Das Format lautet `BPM,Takt,Noten`. Hinter jeder Note steht ihr Notenwert.
`r` bezeichnet eine Pause und `C` das hohe C.

Beispiel: `play 120,4/4,c4d4e8r8C2`

## Alle meine Entchen
play 200,4/4,c4d4e4f4g2g2a4a4a4a4g1a4a4a4a4g1f4f4f4f4e2e2d4d4d4d4c1

## Freude schöner Götterfunke
play 120,4/4,e4e4f4g4g4f4e4e4d4c4c4d4e4e2d2d2r2e4e4f4g4g4f4e4d4c4c4d4e4d4c4c2

## Thunderstruck?
play 100,4/4,d8b8f8b8e8b8f8b8d8b8f8b8e8b8f8b8r4

## Hänschen Klein
play 100,4/4,g4e4e2f4d4d2c4d4e4f4g4g4g2r4g4e4e2f4d4d2c4e4g4g4c2r4c4c4c2

## Terminator
play 124,4/4,e4e8g8e8d8c4b4e4e8g8e8d8c8d8c4b4

## Axel F
play 124,4/4,e4e8g8e8d8c4b4e4e8g8e8d8c8d8c4b4

## 7 Army
play 200,4/4,a2a4C4a4g4f1e1a2a4C4a4g4f4g4f4e1a2a4C4a4g4f1e1a2a4C4a4g4f4g4f4e4

# Insomnia
play 200,4/4,d4d4d8c8c8c8b2

# Not Human
play 180,4/4,c8C8d8b8e8a8f8g8C8c8b8d8a8e8g8f8c8g8C8e8b8d8a8f8C8c8a8e8b8d8g8f8

# Mission Impossible
play 100,5/4,g8g8b8C8g8g8f8g8g8g8e8f8g8g8b8C8g8g8f8g8

# Indiana Jones
play 160,4/4,e4f8g4C2e4f4g4

# Fast Boy
play 260,4/4,c16e16g16c16e16g16d16f16a16d16f16a16c16e16g16c16e16g16
