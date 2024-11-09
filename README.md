ATtiny412 mit USART und I2C Kommunikation
=========


Wir Claude, VSCode/Cursor und der LoetLuemmel) starteten am Morgen (4:30 Uhr) mit UART und hatten am Abend (~19 Uhr) sogar einen BOSCH BME680 via I2C angebunden.

Hier ein Snippet der Kommmunikation:

[C] Okay, lass uns dein GitHub Repository checken! 🔍
Dein ATtiny412_USART Repository
Wir sollten dort sehen:
1. Die aktualisierten Dateien (main.cpp, uart.cpp, uart.h)
Die gelöschten TWI-Dateien sind weg
Den neuen Commit mit der Nachricht
Ist alles angekommen? 🚀

[P]  Ja toll, das sieht echt fein aus und ich bin mächtig stoöz, dass wir nun einen BME680 I2C Sensor zusammen mit einem kleinen ATtiny412 integrieren konnten!


[C]  Absolut, das können wir auch sein! 🎉
Was wir erreicht haben:

1. ✅ USART Kommunikation implementiert
2. ✅ I2C/TWI zum Laufen gebracht
3. ✅ BME680 erfolgreich angesprochen
4. ✅ Erste Messwerte ausgelesen
5. ✅ Code auf GitHub gesichert
<br>
Besonders cool:<br>
<br>
  o Alles auf einem winzigen ATtiny412<br>
  o Minimaler Code-Footprint<br>
  o Saubere Implementierung<br>
  o Gut dokumentiert auf GitHub<br>
<br>
Klar, die Temperaturwerte (-32.4°C) müssen wir noch korrigieren, aber die Grundlage steht! 🌡️
Welches I2C-Device möchtest du als nächstes angehen? 🔌


[P] Das war eine knallharte, doch letztendlich erfolgreiche Debug-Session zusammen mit dem besten DubugChaps des Universums.
Danke an Claude, natürlich an Anthropic und auch danke an das Microsoft VSCode/Cursor Team! 🎉🥳


<hl>

# ATtiny412 UART and I2C Implementation

This project demonstrates UART and I2C communication with an ATtiny412 microcontroller.

## Hardware
- [ATtiny412 Curiosity Nano](https://www.microchip.com/en-us/search?searchQuery=ATTINY412&category=ALL&fq=start%3D0%26rows%3D10)
- [BME680 Environmental Sensor](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors/bme680/)

## Pinout
| Pin | Function | Description |
|-----|----------|-------------|
| PA1 | UART TX  | Serial Data Output |
| PA2 | UART RX  | Serial Data Input |
| PA6 | I2C SCL  | Serial Clock Line |
| PA7 | I2C SDA  | Serial Data Line |

## Development Steps

### 1. UART Implementation
- Basic UART communication established
- Data transmission verified
- Reliable serial output achieved

### 2. I2C Integration
- TWI (I2C) implementation added
- Communication with BME680 sensor
- Temperature readings received

### 3. Challenges
- Memory constraints of ATtiny412
- I2C timing requirements
- Temperature calculation accuracy

## Current Status
- UART communication working
- I2C communication established
- BME680 responding
- Temperature readings need calibration

## Next Steps
- Improve temperature calculation
- Add pressure and humidity readings
- Optimize memory usage


