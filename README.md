# 🗣️ Laboratorio Completo: De la Recolección de Datos al Control por Voz con IA (Edge Impulse)

**👨‍🏫 Profesor:** Ing. Saúl Isaí Soto Ortiz (Titular A)  
**🏫 Institución:** Instituto Tecnológico Superior del Occidente del Estado de Hidalgo (ITSOEH)  
**🎓 Asignatura / Club:** Internet of Things (IoT) & Networking  

En esta práctica maestra implementaremos el flujo de trabajo completo de **TinyML** (Machine Learning en microcontroladores). Aprenderemos a crear un *Dataset* propio grabando nuestra voz en la placa **Seeed Studio XIAO ESP32S3 Sense**, entrenaremos una red neuronal en **Edge Impulse**, y finalmente descargaremos el modelo a la placa para controlar luces físicas (LEDs) usando comandos de voz ("red", "yellow", "green").

---

## 📋 Materiales y Requisitos Previos

1. **Hardware:**
   * Placa **XIAO ESP32S3** + Placa de expansión **Sense** (con micrófono integrado).
   * Cable USB-C (con soporte para transferencia de datos).
   * **Memoria microSD** (Formateada en FAT32, máx. 32GB).
   * 3 LEDs (Rojo, Amarillo, Verde) + 3 Resistencias (220Ω o 330Ω).
   * Protoboard y cables jumper.
2. **Software y Servicios:**
   * Arduino IDE (núcleo `esp32 by Espressif Systems` **versión 3.3.10 o superior**).
   * Cuenta gratuita en [Edge Impulse Studio](https://studio.edgeimpulse.com).

---

## 🔌 Esquema de Conexión (Hardware)

Conectaremos los LEDs a los pines digitales de la placa para la Fase 3. 

| Componente | Pin en XIAO ESP32S3 | Función |
| :--- | :--- | :--- |
| **LED Rojo 🔴** | `D0` | Se encenderá al decir "red" |
| **LED Amarillo 🟡** | `D1` | Se encenderá al decir "yellow" |
| **LED Verde 🟢** | `D2` | Se encenderá al decir "green" |
| **GND (Común)** | `GND` | Cierra el circuito de todos los LEDs |

---

## 🛠️ Fase 1: Recolección del Dataset (Grabación de Audio)

Las Inteligencias Artificiales aprenden con ejemplos. Necesitamos grabar muchos audios cortos (1 segundo) diciendo las palabras clave, y también grabar ruido de fondo.

1. Inserta la memoria microSD en tu placa Sense (antes de conectarla a la PC).
2. Sube el **Código 1 (`01_dataset_collector.ino`)** a tu placa.
3. Abre el **Monitor Serie** a `115200 baudios`.
4. **Interactúa con la Consola:**
   * Escribe **`R`** en la barra superior y **presiona Enter** para grabar un audio de 1 segundo (ej. di "red"). Se guardará como `.wav` en la SD.
   * Escribe **`D`** y **presiona Enter** para borrar el último audio si toses o te equivocas al hablar.
5. Repite este proceso hasta tener unas 30 muestras por cada palabra ("red", "yellow", "green") y unas 30 muestras de silencio/ruido ("noise").
6. Saca la microSD, conéctala a tu PC y copia todos los archivos `.wav` a una carpeta de tu computadora.

---

## 🧠 Fase 2: Entrenamiento en Edge Impulse

1. Crea un proyecto **Audio (Keyword Spotting)** en [Edge Impulse](https://studio.edgeimpulse.com).
2. En **Data Acquisition**, sube tus archivos `.wav` y etiquétalos correctamente (`red`, `yellow`, `green`, `noise`).
3. En **Impulse Design**, configura:
   * *Time series data:* Window size 1000ms.
   * *Processing block:* **Audio (MFCC)** (Extrae las características acústicas o "huella digital" de la voz).
   * *Learning block:* **Classifier**.
4. Ve a **MFCC** y haz clic en *Generate Features*.
5. Ve a **Classifier**, define 100 Epochs (ciclos de aprendizaje) y entrena tu modelo. Debe superar el 85% de precisión.
6. Ve a **Deployment**, selecciona **Arduino Library** y haz clic en **Build**. Se descargará un archivo `.ZIP`.
7. En Arduino IDE, instala este archivo `.ZIP` desde *Programa > Incluir Librería > Añadir biblioteca .ZIP...*

---

## 🚀 Fase 3: Despliegue y Control por Voz

Ahora convertiremos la placa en un dispositivo inteligente que escucha continuamente y acciona los LEDs.

1. Abre el **Código 2 (`02_voice_control.ino`)**.
2. **¡PASO CRÍTICO!** En la línea de librerías del código, cambia `#include <TU_PROYECTO_inferencing.h>` por el nombre exacto de la librería que descargaste de Edge Impulse.
3. Sube el código a la placa (ya no necesitas la microSD para esta fase).
4. Abre el Monitor Serie a `115200 baudios`. ¡Habla cerca del micrófono y mira los LEDs encenderse!

---

## ⚠️ Solución a Problemas Comunes (Troubleshooting)

**Problemas en la Fase 1 (Recolección):**
* ❌ `Error: No se detecta la memoria microSD.`
  * **Solución:** Revisa que esté insertada hasta hacer "clic" y que esté formateada en FAT32 (no exFAT ni NTFS).
* ❌ **Escribo la letra 'R' para grabar, pero no hace nada.**
  * **Solución:** En Arduino IDE 2.x, escribir la letra no es suficiente. Debes escribir `R` y obligatoriamente **presionar la tecla `Enter`** en tu teclado para enviar el comando.
* ❌ **El monitor serie me dice `⚠️ Comando no reconocido` muchas veces seguidas sin que yo escriba nada.**
  * **Solución:** Ve a la esquina inferior derecha del Monitor Serie y asegúrate de que el menú de ajustes de línea esté configurado en **"Ningún ajuste de línea"** (No line ending) o **"Ambos NL y CR"**.

**Problemas en la Fase 3 (Control por Voz):**
* ❌ `Compilation error: ESP_I2S.h: No such file`
  * **Solución:** Actualiza tu paquete de tarjetas ESP32 a la versión 3.x.x desde el Gestor de Tarjetas.
* ❌ **La placa no me hace caso o enciende los LEDs aleatoriamente.**
  * **Solución:** El micrófono Sense necesita que le hables de cerca (unos 10-15 cm). Si se activa solo por el ruido del salón, puedes subir la variable de certidumbre en el Código 2 de `> 0.80` a `> 0.90` (exigiendo un 90% de seguridad a la IA).
