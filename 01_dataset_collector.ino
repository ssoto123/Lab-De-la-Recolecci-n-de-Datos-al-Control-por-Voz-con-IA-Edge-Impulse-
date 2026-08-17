/*
 * 🎤 FASE 1: Recolector de Dataset de Audio - XIAO ESP32S3 Sense
 * 
 * 👨‍🏫 Autor: Ing. Saúl Isaí Soto Ortiz (ITSOEH)
 * 
 * Descripción: Sistema interactivo por Monitor Serie para grabar archivos .wav 
 * de 1 segundo directamente en la microSD, listos para Edge Impulse. 
 * Incluye protección contra caracteres invisibles del puerto serie.
 */

#include <ESP_I2S.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

I2SClass I2S;

const int PIN_SD_CS = 21; 
const int PIN_MIC_CLK = 42;
const int PIN_MIC_DATA = 41;

const int SAMPLE_RATE = 16000; // 16 kHz exigidos por Edge Impulse
int contador_muestras = 0;     

// Buffer de memoria para escritura fluida en la SD
const int BUFFER_SIZE = 512;
int16_t audio_buffer[BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  delay(1000);

  Serial.println("\n--- 🧠 FASE 1: RECOLECCIÓN DE DATASET ---");

  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("❌ ERROR: No se detecta la microSD. ¡Insértala y reinicia!");
    while (1); 
  }
  Serial.println("✅ microSD detectada.");

  I2S.setPinsPdmRx(PIN_MIC_CLK, PIN_MIC_DATA);
  if (!I2S.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("❌ ERROR: El micrófono falló.");
    while (1); 
  }
  Serial.println("✅ Micrófono listo a 16kHz.");
  
  mostrarMenu();
}

void loop() {
  // Verificamos si hay mensajes esperando en el puerto serie
  if (Serial.available() > 0) {
    char comando = toupper(Serial.read());

    // 🛡️ DEFENSA: Ignorar los caracteres invisibles de "salto de línea" 
    // que envía Arduino IDE al presionar la tecla Enter (\n y \r).
    if (comando == '\n' || comando == '\r') {
      return; 
    }

    // Evaluamos qué letra presionó el alumno
    if (comando == 'R') {
      grabarMuestra();
    } 
    else if (comando == 'D') {
      eliminarUltimaMuestra();
    } 
    else if (comando == 'S') {
      Serial.printf("📊 STATUS: Tienes %d muestras guardadas.\n", contador_muestras);
    } 
    else {
      // Retroalimentación visual si el alumno se equivoca de letra
      Serial.println("⚠️ Comando [" + String(comando) + "] no reconocido. Usa R, D o S.");
    }
  }
}

void grabarMuestra() {
  contador_muestras++; 
  String nombre = "/muestra_" + String(contador_muestras) + ".wav";
  File archivo = SD.open(nombre, FILE_WRITE);
  
  if (!archivo) {
    Serial.println("❌ Error de escritura SD.");
    contador_muestras--; return;
  }

  Serial.print("🔴 GRABANDO (1 segundo): ¡Habla ahora!... ");

  uint32_t total_bytes = SAMPLE_RATE * 2 * 1; // 1 segundo
  escribirCabeceraWAV(archivo, total_bytes);

  int ciclos = total_bytes / (BUFFER_SIZE * 2);
  for (int i = 0; i < ciclos; i++) {
    for (int j = 0; j < BUFFER_SIZE; j++) {
      audio_buffer[j] = (int16_t)I2S.read();
    }
    archivo.write((const uint8_t*)audio_buffer, BUFFER_SIZE * 2);
  }

  archivo.close();
  Serial.println("✅ Guardado: " + nombre);
}

void eliminarUltimaMuestra() {
  if (contador_muestras > 0) {
    String nombre = "/muestra_" + String(contador_muestras) + ".wav";
    SD.remove(nombre);
    Serial.println("🗑️ Eliminado: " + nombre + ". Puedes repetir la grabación.");
    contador_muestras--;
  } else {
    Serial.println("⚠️ No hay muestras para eliminar.");
  }
}

void mostrarMenu() {
  Serial.println("\n--- 🎛️ CONTROLES ---");
  Serial.println("[R] + Enter - Grabar 1 segundo (Record)");
  Serial.println("[D] + Enter - Borrar última muestra (Delete)");
  Serial.println("[S] + Enter - Ver contador (Status)\n");
}

void escribirCabeceraWAV(File &archivo, uint32_t tamano_datos) {
  byte header[44];
  uint32_t tamano_total = tamano_datos + 36;
  uint32_t byte_rate = SAMPLE_RATE * 2; 

  header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
  header[4] = (byte)(tamano_total & 0xFF); header[5] = (byte)((tamano_total >> 8) & 0xFF);
  header[6] = (byte)((tamano_total >> 16) & 0xFF); header[7] = (byte)((tamano_total >> 24) & 0xFF);
  header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
  header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
  header[16] = 16; header[17] = 0; header[18] = 0; header[19] = 0;
  header[20] = 1; header[21] = 0; header[22] = 1; header[23] = 0; 
  header[24] = (byte)(SAMPLE_RATE & 0xFF); header[25] = (byte)((SAMPLE_RATE >> 8) & 0xFF);
  header[26] = (byte)((SAMPLE_RATE >> 16) & 0xFF); header[27] = (byte)((SAMPLE_RATE >> 24) & 0xFF);
  header[28] = (byte)(byte_rate & 0xFF); header[29] = (byte)((byte_rate >> 8) & 0xFF);
  header[30] = (byte)((byte_rate >> 16) & 0xFF); header[31] = (byte)((byte_rate >> 24) & 0xFF);
  header[32] = 2; header[33] = 0; header[34] = 16; header[35] = 0; 
  header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
  header[40] = (byte)(tamano_datos & 0xFF); header[41] = (byte)((tamano_datos >> 8) & 0xFF);
  header[42] = (byte)((tamano_datos >> 16) & 0xFF); header[43] = (byte)((tamano_datos >> 24) & 0xFF);
  
  archivo.write(header, 44);
}
