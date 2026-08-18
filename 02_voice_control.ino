/*
 * 🚀 FASE 3: Control por Voz con IA - XIAO ESP32S3 Sense
 * 
 * 👨‍🏫 Autor: Ing. Saúl Isaí Soto Ortiz (ITSOEH)
 * 
 * Descripción: Lee ventanas exactas de audio (1 segundo), las procesa 
 * con Edge Impulse e imprime en pantalla TODOS los porcentajes para 
 * que los alumnos entiendan cómo "piensa" la red neuronal en tiempo real.
 */

// ⚠️ MODIFICA ESTA LÍNEA POR EL NOMBRE EXACTO DE TU LIBRERÍA ZIP DESCARGADA
#include <TU_PROYECTO_inferencing.h> 

#include <ESP_I2S.h>

const int LED_RED = D0;
const int LED_YELLOW = D1;
const int LED_GREEN = D2;

const int PIN_MIC_CLK = 42;
const int PIN_MIC_DATA = 41;

I2SClass I2S;

// 🛡️ CORRECCIÓN: Leemos exactamente el tamaño del audio con el que 
// entrenaste la IA (EI_CLASSIFIER_RAW_SAMPLE_COUNT = 16000 muestras = 1 segundo).
#define BUFFER_SIZE EI_CLASSIFIER_RAW_SAMPLE_COUNT
int16_t audio_buffer[BUFFER_SIZE];

// FUNCIÓN CALLBACK: El "puente" oficial para Edge Impulse
int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = (float)audio_buffer[offset + i];
  }
  return 0; 
}

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
  
  Serial.println("\n🚀 FASE 3: Sistema de IA Listo. Iniciando micrófono...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  I2S.setPinsPdmRx(PIN_MIC_CLK, PIN_MIC_DATA);
  if (!I2S.begin(I2S_MODE_PDM_RX, EI_CLASSIFIER_FREQUENCY, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("❌ ERROR: El micrófono falló.");
    while (1); 
  }
  
  Serial.println("✅ Todo correcto. ¡Comienza a hablar!");
}

void loop() {
  Serial.println("\n🎙️ Escuchando (1 segundo)...");

  // 1. LEER AUDIO (Se quedará aquí exactamente 1 segundo recopilando datos)
  for (int i = 0; i < BUFFER_SIZE; i++) {
    audio_buffer[i] = (int16_t)I2S.read();
  }

  // 2. APLICAR GANANCIA (Subir volumen)
  scaleVolume(audio_buffer, BUFFER_SIZE);

  // 3. SEÑAL PARA EDGE IMPULSE
  signal_t signal;
  signal.total_length = BUFFER_SIZE;
  signal.get_data = &microphone_audio_signal_get_data;

  // 4. EJECUTAR INFERENCIA (Clasificador estándar)
  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.printf("❌ Error de inferencia (%d)\n", res);
    return;
  }

  // 5. MOSTRAR PENSAMIENTO DE LA IA EN PANTALLA
  Serial.println("🧠 Resultados de la IA:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    
    // Imprimimos el nombre de la etiqueta y su porcentaje de probabilidad
    Serial.printf("    %s: %.0f%%\n", result.classification[ix].label, result.classification[ix].value * 100.0);

    // 6. ACCIONAR LEDS (Si está más del 80% seguro de una palabra clave)
    if (result.classification[ix].value > 0.80) {
      String palabraDetectada = result.classification[ix].label;
      
      if (palabraDetectada == "red") {
        encenderUnSoloLed(LED_RED);
      } 
      else if (palabraDetectada == "yellow") {
        encenderUnSoloLed(LED_YELLOW);
      } 
      else if (palabraDetectada == "green") {
        encenderUnSoloLed(LED_GREEN);
      }
    }
  }
  Serial.println("-------------------------");
}

void encenderUnSoloLed(int led_elegido) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  
  digitalWrite(led_elegido, HIGH);
  delay(500); 
  digitalWrite(led_elegido, LOW);
}
