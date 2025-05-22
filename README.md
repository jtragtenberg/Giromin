# Giromin — Instrumento Digital de Dança e Música a partir de Gestos Livres para controlar parâmetros sonoros por OSC ou MIDI

Este guia explica como compilar e executar o sistema **Giromin**

---

## Requisitos

### Hardware:
- Placa com microcontrolador ESP32
- IMU MPU6886
- Botões físicos (opcional)
- Bateria para uso sem fios (opcional)

---

### Software:

 **Arduino IDE 1.8+** ou **Arduino IDE 2.x**  

---

## Bibliotecas necessárias

Instale via **Gerenciador de Bibliotecas** do Arduino:

- OSCMessage (CNMAT)
- U8g2
- Bounce2

---

## Compilando com Arduino IDE
Abra o Giromin.ino.

Verifique se as bibliotecas citadas estão instaladas.

Selecione a placa:
→ Ferramentas → Placa → ESP32 Dev Module

(se ela não estiver configurada, acesse este tutorial: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)

Selecione a porta serial correta.

Clique em Upload.



### Após a inicialização, o sistema conecta-se ao WiFi especificado:

const std::string ssid = "giromin0";
const std::string pass = "dervishmaria";

### O Giromin envia dados via OSC para o IP e porta configurados:

const IPAddress outIp(192, 168, 0, 140);
constexpr unsigned int outPort = 1333;

Pressionar os botões físicos enviará mensagens OSC.
Movimento do Giromin enviará dados IMU em tempo real.

--- 

## Funcionalidades
- Comunicação OSC via WiFi
- Leitura de sensores da IMU
- Detecção de botões com debounce
- Calibração automática da IMU
- Orientação (quaternions) a partir da fusão dos sensores da IMU
- Dados do IMU calibrados (Acelerômetros e Giroscópios)