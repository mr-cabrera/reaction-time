# Reaction Time Tester

Dispositivo electrónico para medir el **tiempo de reacción humana** frente a estímulos visuales y/o auditivos.  
El proyecto está inspirado en implementaciones académicas (como la tesis de ESPOL, Ecuador) y adaptado a microcontroladores modernos.

---

## Características
- Mide el tiempo entre un estímulo (LED/buzzer) y la respuesta del usuario (botón).  
- Precisión en el rango de **milisegundos** usando temporizadores del microcontrolador.  
- Registro de **tiempos válidos**, **falsos arranques** y **timeouts**.  
- Posibilidad de extender a múltiples jugadores o distintos tipos de estímulos.  

---

## Hardware
- **Microcontrolador:** ESP32.  
- **Estímulos:** LED para estímulo visual y buzzer piezoeléctrico opcional para estímulo auditivo.  
- **Respuesta:** Pulsador con resistencia pull-up interna.  
- **Otros:** Resistencias de 220–330 Ω para LEDs, alimentación USB 5V.  
