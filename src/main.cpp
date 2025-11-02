#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ThingSpeak.h>

// --- INCLUSIONES DE BIBLIOTECAS (CORREGIDO) ---
// Estas bibliotecas son necesarias para el DHT y el Display
#include <Wire.h>                  // --- CORREGIDO --- Para la comunicación I2C del display
#include <Adafruit_Sensor.h>       // --- CORREGIDO --- Requerido por la biblioteca DHT
#include <DHT.h>
#include <Adafruit_SH110X.h>      // --- CORREGIDO --- La biblioteca para tu display (SH1106/SH110X)


// --- DEFINICIONES DE PINES Y CONSTANTES (CORREGIDO) ---
// Todas las definiciones DEBEN ir antes de crear los objetos

// Pines de LEDs
const int pinLEDAzul = 2;
const int pinLEDVerde = 23;

// Pines de Sensores
const int pinDHT = 33;      // Sensor H y T
const int pinADC = 32;      // Potenciometro

// Pines del Display I2C
const int pinSCL = 22;      // display
const int pinSDA = 21;      // display

// Constantes del Sensor DHT
#define DHTTYPE DHT22       // Define el TIPO de sensor
// --- FIN DE LA SECCIÓN MOVIDA ---


// --- CREACIÓN DE OBJETOS (CORREGIDO) ---
// Ahora que los pines están definidos, podemos crear los objetos
DHT dht(pinDHT, DHTTYPE); // Objeto DHT

// --- CORREGIDO --- 
// Objeto del display. SH1106G es el más común para 1.3" 128x64
// El -1 significa que no usamos pin de Reset.
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);


// --- CONFIGURACIÓN DE RED Y BOT ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* botToken = "8312206932:AAHm2N1lDyhAjhgEXM2IrMaXnwBy7jp8t-k";
const unsigned long SCAN_TIME = 500;

//se necesita cifrar la comunicación porque telegram trabaja con https
WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);
unsigned long previous;

const char* WriteAPIKey = "KTCSTHIBOLXOP5Q2";     //Write API Key de vuestro canal.

WiFiClient clienteThingSpeak; // <-- ¡Añade esta línea!

void handleMessages(int n){
  for (size_t i = 0; i < n; i++){
    //dame el id de los mensajes nuevos
    String chat_id = bot.messages[i].chat_id; //informativo
    //dame todos los mensajes
    String text = bot.messages[i].text; //contenido de cada mensaje
    
    if(text == "/start"){
      // al estar dentro de una cadena se deben usar los caracteres de escape para las comillas
      String json = "[";
            // Fila 1: LEDs Verde
            json += "[\"Verde ON\", \"Verde OFF\"],";
            // Fila 2: LEDs Azul
            json += "[\"Azul ON\", \"Azul OFF\"],";
            // Fila 3: Lectura de Sensores
            json += "[\"Leer Temp/Hum\", \"Leer Potenciometro\"],";
            // Fila 4: Comandos de Display
            json += "[\"Estado Verde\", \"Estado Azul\", \"Estado Pot\"],"; // --- CORREGIDO --- Faltaba una coma aquí
            // Fila 5: Comandos de Display
            json += "[\"Estado sensor\",\"Enviar HyT\"]";
            json += "]";
      //manda como texto lo que dice el boton json por lo que se debe cambiar las opciones
      bot.sendMessageWithReplyKeyboard(chat_id, "Elija una opción", "", json);
      //mensaje de bienvenida + mostrar opciones de MENU:
      //el mensaje va para un chat_id concreto
      
    }
    // --- CORREGIDO --- Nombres de variables (pinLEDVerde en lugar de pinLED23)
    else if(text == "Verde ON" || text == "/led23on"){
      digitalWrite(pinLEDVerde, HIGH);
      bot.sendMessage(chat_id, "\n LED verde encendido");
      Serial.println("Mensaje recibido: [" + text + "]");
    }
    else if(text == "Verde OFF" || text == "led23off"){
      digitalWrite(pinLEDVerde, LOW);
      bot.sendMessage(chat_id, "\n LED verde apagado");
    }
    // --- CORREGIDO --- Nombres de variables (pinLEDAzul en lugar de pinLED2)
    else if(text == "Azul ON" || text == "/led2on"){
      digitalWrite(pinLEDAzul, HIGH);
      bot.sendMessage(chat_id, "\n LED azul encendido");
    }
    else if(text == "Azul OFF" || text == "/led2off"){
      digitalWrite(pinLEDAzul, LOW);
      bot.sendMessage(chat_id, "\n LED azul apagado");
    }
    else if(text == "Leer Temp/Hum" || text == "/dht22"){
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        bot.sendMessage(chat_id, "Error al leer del sensor DHT!");
        return;
      }
      String mensaje = "Temperatura: " + String(t) + " °C\nHumedad: " + String(h) + " %";
      bot.sendMessage(chat_id, mensaje);
    }

    else if(text == "Leer Potenciometro" || text == "/pote"){
      int valorADC = analogRead(pinADC);
      String mensaje = "Valor ADC del Potenciometro: " + String(valorADC);
      bot.sendMessage(chat_id, mensaje);
    }

    else if(text == "Enviar HyT" || text == "/platiot"){
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        bot.sendMessage(chat_id, "Error al leer del sensor DHT!");
        return;
      }
      String mensaje = "Temperatura: " + String(t) + " °C\nHumedad: " + String(h) + " %";
      bot.sendMessage(chat_id, mensaje);

      ThingSpeak.setField (1,t);
      ThingSpeak.setField (2,h);

      ThingSpeak.writeFields(3132022,WriteAPIKey);
    }

    else if(text == "Estado Verde" || text == "/displayled23"){
            
            // 1. Leer el estado actual del pin
            int estado = digitalRead(pinLEDVerde); // --- CORREGIDO --- (era pinLEDVerde)
            String mensaje = (estado == HIGH) ? "LED Verde: ON" : "LED Verde: OFF";

            // 2. Limpiar la pantalla y configurar el texto
            display.clearDisplay();
            display.setTextSize(1); // Usar tamaño 2 para que sea más visible
            display.setTextColor(SH110X_WHITE); // --- CORREGIDO --- (era SSD1306_WHITE)
            display.setCursor(0, 0); // Posiciona el cursor

            // 3. Escribir el mensaje de estado en el buffer
            display.println("--- ESTADO ---");
            display.setCursor(0, 20); // Mueve el cursor a la siguiente línea
            display.println(mensaje);
            
            // 4. Mostrar el contenido del buffer en el display
            display.display();
            
            // Opcional: Enviar también la confirmación por Telegram
            bot.sendMessage(chat_id, "Mostrando estado del LED Verde en OLED.");
    }

    else if(text == "Estado Azul" || text == "/displayled2"){
          
            // 1. Leer el estado actual del pin
            int estado = digitalRead(pinLEDAzul); // --- CORREGIDO --- (era pinLEDAzul)
            String mensaje = (estado == HIGH) ? "LED Azul: ON" : "LED Azul: OFF";

            // 2. Limpiar la pantalla y configurar el texto
            display.clearDisplay();
            display.setTextSize(1); // Tamaño 2 (más visible)
            display.setTextColor(SH110X_WHITE); // --- CORREGIDO ---
            display.setCursor(0, 0); 

            // 3. Escribir el mensaje de estado en el buffer
            display.println("--- ESTADO ---");
            display.setCursor(0, 20); 
            display.println(mensaje);
            
            // 4. Mostrar el contenido del buffer en el display
            display.display();
            
            // Opcional: Enviar también la confirmación por Telegram
            bot.sendMessage(chat_id, "Mostrando estado del LED Azul en OLED.");
    }

    else if(text == "Estado Pot" || text == "/displaypote"){
            
            // 1. Leer el valor crudo del ADC (0 a 4095 por defecto en ESP32)
            int valorADC = analogRead(pinADC);
            
            // 2. Convertir el valor a un rango de 0 a 100 (para mejor visualización)
            // map(value, fromLow, fromHigh, toLow, toHigh)
            long porcentaje = map(valorADC, 0, 4095, 0, 100);
            
            // 3. Crear el mensaje
            String mensaje1 = "Potenciometro:";
            String mensaje2 = String(porcentaje) + " %"; // Convierte el número a String
            
            // 4. Limpiar la pantalla y configurar el texto
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SH110X_WHITE); // --- CORREGIDO ---
            display.setCursor(0, 0); 
            display.println("--- POTENCIOMETRO ---");

            // 5. Mostrar los valores en la pantalla
            display.setTextSize(1); // Aumenta el tamaño para el valor principal
            display.setCursor(0, 20); 
            display.println(mensaje1);
            
            display.setCursor(0, 40); 
            display.println(mensaje2);
            
            // 6. Mostrar el contenido del buffer en el display
            display.display();
            
            // Opcional: Enviar también la confirmación por Telegram
            bot.sendMessage(chat_id, "Potenciómetro: " + mensaje2 + " (" + String(valorADC) + " ADC)");
    }

    else if(text == "Estado Sensor" || text == "/displaydht22"){
            
            // 1. Leer los valores del sensor
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            
            // 2. Comprobar si la lectura es válida
            if (isnan(h) || isnan(t)) {
                // Manejo de error en el OLED
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SH110X_WHITE); // --- CORREGIDO ---
                display.setCursor(0, 0);
                display.println("ERROR!");
                display.println("Fallo DHT22");
                display.display();
                
                bot.sendMessage(chat_id, "ERROR: No se pudo leer el sensor DHT22.");
                return; // Sale del bloque para no continuar con valores inválidos
            }
            
            // 3. Formatear los mensajes (usando 1 decimal)
            String tempStr = "T: " + String(t, 1) + " C";
            String humStr = "H: " + String(h, 1) + " %";
            
            // 4. Limpiar la pantalla y configurar el texto
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SH110X_WHITE); // --- CORREGIDO ---
            display.setCursor(0, 0); 
            display.println("--- DHT22 ---");

            // 5. Mostrar los valores de T y H
            display.setTextSize(1); // Usar tamaño 2 para los valores principales
            display.setCursor(0, 16); // Primera línea de valor
            display.println(tempStr);
            
            display.setCursor(0, 40); // Segunda línea de valor
            display.println(humStr);
            
            // 6. Mostrar el contenido del buffer en el display
            display.display();
            
            // Opcional: Enviar también los datos por Telegram
            bot.sendMessage(chat_id, "Datos DHT22: " + tempStr + ", " + humStr);
        }
    else {
      bot.sendMessage(chat_id, "Comando no reconocido");
    }
  }
}

// --- BLOQUE PARA PANTALLA DE INICIO ---
void mostrarInicio() {
  display.clearDisplay();

  // --- LOGO FACULTAD  ---
  const unsigned char logoUTN[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x81, 0xe0, 0x78, 0x03, 0xf0, 0x00, 0x7e, 0x0f, 0xff, 0xff, 0xf0, 0x7f, 0x80, 0x07, 0xe0, 
	0x07, 0xc1, 0xf0, 0xf8, 0x07, 0xf8, 0x00, 0x7f, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0xc0, 0x0f, 0xe0, 
	0x07, 0xc1, 0xf0, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0xc0, 0x0f, 0xe0, 
	0x07, 0xc1, 0xf0, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0xe0, 0x0f, 0xe0, 
	0x03, 0xe1, 0xf1, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0xf0, 0x0f, 0xe0, 
	0x03, 0xf1, 0xf1, 0xf0, 0x07, 0xf0, 0x00, 0x7f, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0xf0, 0x0f, 0xe0, 
	0x01, 0xf9, 0xe3, 0xf0, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xff, 0xf8, 0x0f, 0xe0, 
	0x01, 0xff, 0xff, 0xe0, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0xff, 0xfc, 0x0f, 0xe0, 
	0x00, 0xff, 0xff, 0xc0, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xff, 0xfc, 0x0f, 0xe0, 
	0x00, 0x7f, 0xff, 0x80, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0xfe, 0x0f, 0xe0, 
	0x00, 0x3f, 0xff, 0x00, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0xfe, 0x0f, 0xe0, 
	0x00, 0x07, 0xfc, 0x00, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x7f, 0x0f, 0xe0, 
	0x00, 0x03, 0xf8, 0x00, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x3f, 0x8f, 0xe0, 
	0x07, 0xff, 0xff, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x3f, 0x87, 0xe0, 
	0x07, 0xff, 0xff, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x1f, 0xc7, 0xe0, 
	0x07, 0xff, 0xff, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x1f, 0xc7, 0xe0, 
	0x07, 0xff, 0xff, 0xf8, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x0f, 0xe7, 0xe0, 
	0x03, 0xe3, 0xf1, 0xf0, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x0f, 0xe7, 0xe0, 
	0x00, 0x0f, 0xfc, 0x00, 0x07, 0xf8, 0x00, 0x7f, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x07, 0xf7, 0xe0, 
	0x00, 0x3f, 0xff, 0x00, 0x03, 0xf8, 0x00, 0xff, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x03, 0xff, 0xe0, 
	0x00, 0x7f, 0xff, 0x80, 0x03, 0xf8, 0x00, 0xfe, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x03, 0xff, 0xe0, 
	0x00, 0xff, 0xff, 0xc0, 0x03, 0xfc, 0x00, 0xfe, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x01, 0xff, 0xe0, 
	0x01, 0xff, 0xff, 0xe0, 0x03, 0xfc, 0x01, 0xfe, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x00, 0xff, 0xe0, 
	0x03, 0xf9, 0xe7, 0xf0, 0x01, 0xff, 0x03, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x00, 0xff, 0xe0, 
	0x03, 0xf1, 0xf3, 0xf0, 0x01, 0xff, 0xff, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x00, 0x7f, 0xe0, 
	0x03, 0xe1, 0xf1, 0xf8, 0x00, 0xff, 0xff, 0xf8, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x00, 0x7f, 0xe0, 
	0x07, 0xc1, 0xf0, 0xf8, 0x00, 0x7f, 0xff, 0xf0, 0x00, 0x0f, 0xe0, 0x00, 0xfe, 0x00, 0x3f, 0xe0, 
	0x07, 0xc1, 0xf0, 0xf8, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x0f, 0xf0, 0x00, 0xfe, 0x00, 0x1f, 0xe0, 
	0x07, 0xc1, 0xf0, 0xf8, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x0f, 0xf0, 0x00, 0x7e, 0x00, 0x1f, 0xe0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  display.drawBitmap(0, 0, logoUTN, 128, 39, SH110X_WHITE);

  // --- TEXTO GRUPO ---
  display.setTextSize(1); // tamaño normal
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 50); 
  display.println("GRUPO 6 - TP02 IoT");


  display.display();
  delay(5000); // Mostrar 5 segundos antes de limpiar
  display.clearDisplay();
  display.display();
}

void mostrarAnimacionEspera(int duracion_ms) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  const int anchoDisplay = 128;
  const int altoDisplay = 64;

  // --- Texto ---
  String texto = "Esperando...";
  int textoX = (anchoDisplay - texto.length() * 6) / 2; // 6 px por caracter aprox
  int textoY = 20; // más arriba para dejar espacio abajo

  // --- Parámetros de los puntos ---
  int numPuntos = 5;
  int radio = 4;
  int espacio = 12;
  int anchoTotal = (numPuntos - 1) * espacio;
  int xBase = (anchoDisplay - anchoTotal) / 2;
  int yBase = 45; // más abajo que el texto

  unsigned long start = millis();
  int step = 0;

  // --- Animación ---
  while (millis() - start < duracion_ms) {
    display.clearDisplay();

    // Texto centrado
    display.setCursor(textoX, textoY);
    display.println(texto);

    // Puntos animados
    for (int i = 0; i < numPuntos; i++) {
      int x = xBase + i * espacio;
      if (i == step)
        display.fillCircle(x, yBase, radio, SH110X_WHITE); // punto activo
      else
        display.drawCircle(x, yBase, radio, SH110X_WHITE); // punto inactivo
    }

    display.display();
    delay(200);
    step = (step + 1) % numPuntos;
  }

  display.clearDisplay();
  display.display();
}


void setup() {
  // --- COMUNICACIÓN SERIAL ---
  Serial.begin(115200);
  Serial.println(F("Iniciando sistema..."));
  

  // --- CONFIGURACIÓN DE PINES ---
  pinMode(pinLEDAzul, OUTPUT);
  pinMode(pinLEDVerde, OUTPUT);


  // --- CONEXIÓN WiFi ---
  Serial.print(F("Conectando a WiFi"));
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Certificado raíz para HTTPS (Telegram)

  // Esperar hasta conectar
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println();
  Serial.println(F("Conectado a la red WiFi"));
  Serial.print(F("Dirección IP: "));
  Serial.println(WiFi.localIP());


  // --- INICIALIZAR THINGSPEAK ---
  ThingSpeak.begin(secured_client);


  // --- INICIALIZAR DISPLAY OLED (SH110X) ---
  // Dirección I2C por defecto: 0x3C
  Wire.begin(pinSDA, pinSCL);

  if (!display.begin(0x3C, true)) {
    Serial.println(F("Error: No se pudo inicializar el display SH110X."));
    while (true); // Detiene la ejecución
  }

  display.clearDisplay();
  display.display();

  // --- MENSAJE DE BIENVENIDA ---
  mostrarInicio();                // Logo + texto inicial
  mostrarAnimacionEspera(3000);   // Animación de carga (3 segundos)


  // --- INICIALIZAR SENSOR DHT ---
  dht.begin();
  Serial.println(F("Sensor DHT listo."));


  // --- INICIALIZAR THINGSPEAK (CLIENTE PERSONALIZADO) ---
  ThingSpeak.begin(clienteThingSpeak); // Si usás un cliente diferente
  Serial.println(F("ThingSpeak inicializado correctamente."));

  // --- MENSAJE FINAL: SISTEMA LISTO ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  display.println(F("Sistema listo"));
  display.setCursor(2, 35);
  display.println(F("Esperando comandos..."));
  display.display();
}




void loop() {
  if (millis() - previous > SCAN_TIME ){
    //dame la cantidad de mensajes nuevo que llegaron desde el ultimo SCAN_TIME
    int n = bot.getUpdates(bot.last_message_received + 1);
    
    //mientras que haya mensajes nuevos, los manejo
    while(n){
      handleMessages(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }

    previous = millis();
  }
}