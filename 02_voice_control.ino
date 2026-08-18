/*
 * 🚀 FASE 3: Control por Voz con IA - XIAO ESP32S3 Sense
 * 
 * 👨‍🏫 Autor: Ing. Saúl Isaí Soto Ortiz (ITSOEH)
 * 
 * Descripción: Lee audio continuo, lo procesa con la red neuronal exportada
 * de Edge Impulse y enciende LEDs según la palabra clave detectada.
 */

// ⚠️ MODIFICA ESTA LÍNEA POR EL NOMBRE DE TU LIBRERÍA ZIP DESCARGADA
#include <TU_PROYECTO_inferencing.h> 

#include <ESP_I2S.h>

const int LED_RED = D0;
const int LED_YELLOW = D1;
const int LED_GREEN = D2;

const int PIN_MIC_CLK = 42;
const int PIN_MIC_DATA = 41;

I2SClass I2S;

#define BUFFER_SIZE 512
int16_t audio_buffer[BUFFER_SIZE];

// Función para subir el volumen digitalmente (Ganancia x16)
void scaleVolume(int16_t* audioData, size_t sampleCount) {
  const float gain = 16.0;
  for (size_t i = 0; i < sampleCount; i++) {
    audioData[i] = (int16_t)constrain(audioData[i] * gain, INT16_MIN, INT16_MAX);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  
  Serial.println("\n🚀 FASE 3: Sistema de IA Escuchando...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  I2S.setPinsPdmRx(PIN_MIC_CLK, PIN_MIC_DATA);
  if (!I2S.begin(I2S_MODE_PDM_RX, EI_CLASSIFIER_FREQUENCY, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("❌ ERROR: El micrófono falló.");
    while (1); 
  }
  
  run_classifier_init(); // Iniciar motor de Inteligencia Artificial
}

void loop() {
  // 1. LEER AUDIO
  for (int i = 0; i < BUFFER_SIZE; i++) {
    audio_buffer[i] = (int16_t)I2S.read();
  }

  // 2. APLICAR GANANCIA (Subir volumen)
  scaleVolume(audio_buffer, BUFFER_SIZE);

  // 3. ENVIAR A EDGE IMPULSE
  signal_t signal;
  if (numpy::audio_to_signal(audio_buffer, BUFFER_SIZE, &signal) != 0) return;

  // 4. INFERENCIA CONTINUA
  ei_impulse_result_t result = { 0 };
  if (run_classifier_continuous(&signal, &result, false) != EI_IMPULSE_OK) return;

  // 5. EVALUAR Y ACCIONAR
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    
    // Si la IA está más de un 80% segura
    if (result.classification[ix].value > 0.80) {
      
      String palabraDetectada = result.classification[ix].label;
      
      if (palabraDetectada == "red") {
        Serial.println("🔴 Comando detectado: RED -> LED Rojo ON");
        encenderUnSoloLed(LED_RED);
      } 
      else if (palabraDetectada == "yellow") {
        Serial.println("🟡 Comando detectado: YELLOW -> LED Amarillo ON");
        encenderUnSoloLed(LED_YELLOW);
      } 
      else if (palabraDetectada == "green") {
        Serial.println("🟢 Comando detectado: GREEN -> LED Verde ON");
        encenderUnSoloLed(LED_GREEN);
      }
    }
  }
}

void encenderUnSoloLed(int led_elegido) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  
  digitalWrite(led_elegido, HIGH);
  delay(500); 
  digitalWrite(led_elegido, LOW);
}
