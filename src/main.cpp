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

// --- FUNCIÓN PARA MANEJAR MENSAJES (CORREGIDO) ---

// Función que muestra un mensaje temporal y vuelve al mensaje de espera
void showTempMessage(String line1, String line2 = "", int tiempo = 2000) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  if (line2 != "") display.println(line2);
  display.display();

  delay(tiempo); // espera el tiempo que queramos mostrar el mensaje

  // Vuelve al mensaje de sistema listo
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  display.println(F("Sistema listo"));
  display.setCursor(2, 35);
  display.println(F("Esperando comandos..."));
  display.display();
}

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
      else if(text == "Verde ON" || text == "/led23on"){
      showTempMessage("Encendiendo Luz Verde", "", 1500); // mensaje inicial con 1.5s
      digitalWrite(pinLEDVerde, HIGH);
      showTempMessage("Luz Verde Encendida", "", 2000);  // mensaje encendido 2s
      bot.sendMessage(chat_id, "LED verde encendido");
      Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Verde OFF" || text == "led23off"){
    showTempMessage("Apagando Luz Verde", "", 1500);
    digitalWrite(pinLEDVerde, LOW);
    showTempMessage("Luz Verde Apagada", "", 2000);
    bot.sendMessage(chat_id, "LED verde apagado");
    Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Azul ON" || text == "/led2on"){
        showTempMessage("Encendiendo Luz Azul", "", 1500);
        digitalWrite(pinLEDAzul, HIGH);
        showTempMessage("Luz Azul Encendida", "", 2000);
        bot.sendMessage(chat_id, "LED azul encendido");
        Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Azul OFF" || text == "/led2off"){
        showTempMessage("Apagando Luz Azul", "", 1500);
        digitalWrite(pinLEDAzul, LOW);
        showTempMessage("Luz Azul Apagada", "", 2000);
        bot.sendMessage(chat_id, "LED azul apagado");
        Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Leer Temp/Hum" || text == "/dht22"){
        showTempMessage("Leyendo Temp y Humedad", "", 1500);
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
            bot.sendMessage(chat_id, "Error al leer del sensor DHT!");
            return;
        }
        showTempMessage("Temp: " + String(t,1) + " C", "Hum: " + String(h,1) + " %", 2000);
        bot.sendMessage(chat_id, "Temperatura: " + String(t) + " °C\nHumedad: " + String(h) + " %");
    }

    else if(text == "Leer Potenciometro" || text == "/pote"){
        showTempMessage("Leyendo Potenciometro", "", 1500);
        int valorADC = analogRead(pinADC);
        showTempMessage("Pot: " + String(valorADC), "", 2000);
        bot.sendMessage(chat_id, "Valor ADC del Potenciometro: " + String(valorADC));
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
            
      showTempMessage("Consultando Estado...", "", 2000); // mensaje inicial temporal

      // Leer el estado actual del pin
      int estado = digitalRead(pinLEDVerde);
      String mensaje = (estado == HIGH) ? "Led Verde: ON" : "Led Verde: OFF";

      // Mostrar el estado en pantalla temporalmente
      showTempMessage("-- ESTADO LED VERDE --", mensaje, 4000);  // mensaje resultado

      // Confirmación por Telegram
      bot.sendMessage(chat_id, "Mostrando estado del LED Verde en OLED.");
      Serial.println("Mensaje recibido: [" + text + "]");
    }

        else if(text == "Estado Azul" || text == "/displayled2"){
        // Mensaje inicial temporal
        showTempMessage("Consultando Estado LED Azul", "", 2000);

        // Leer el estado actual del pin
        int estado = digitalRead(pinLEDAzul);
        String mensaje = (estado == HIGH) ? "LED Azul: ON" : "LED Azul: OFF";

        // Mostrar el estado en pantalla temporalmente
        showTempMessage("-- ESTADO LED AZUL --", mensaje, 4000);

        // Confirmación por Telegram y registro en Serial
        bot.sendMessage(chat_id, "Mostrando estado del LED Azul en OLED.");
        Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Estado Pot" || text == "/displaypote"){
        // Mensaje inicial temporal
        showTempMessage("Consultando Potenciometro", "", 2000);

        // Leer valor del ADC y convertir a porcentaje
        int valorADC = analogRead(pinADC);
        long porcentaje = map(valorADC, 0, 4095, 0, 100);

        // Crear mensaje
        String mensaje1 = "Potenciometro:";
        String mensaje2 = String(porcentaje) + " %";

        // Mostrar el valor en pantalla temporalmente
        showTempMessage(mensaje1, mensaje2, 4000);

        // Confirmación por Telegram
        bot.sendMessage(chat_id, "Potenciómetro: " + mensaje2 + " (" + String(valorADC) + " ADC)");
        Serial.println("Mensaje recibido: [" + text + "]");
    }


        else if(text == "Estado sensor" || text == "/displaydht22"){
        // Mensaje inicial temporal
        showTempMessage("Consultando Sensor DHT22", "", 2000);

        // Leer los valores del sensor
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        // Comprobar si la lectura es válida
        if (isnan(h) || isnan(t)) {
            showTempMessage("ERROR!", "Fallo DHT22", 2000);
            bot.sendMessage(chat_id, "ERROR: No se pudo leer el sensor DHT22.");
            return;
        }

        // Formatear los mensajes (1 decimal)
        String tempStr = "T: " + String(t, 1) + " C";
        String humStr  = "H: " + String(h, 1) + " %";

        // Mostrar los valores en pantalla temporalmente
        showTempMessage("--- DHT22 ---", tempStr + "  " + humStr, 4000);

        // Confirmación por Telegram
        bot.sendMessage(chat_id, "Datos DHT22: " + tempStr + ", " + humStr);
        Serial.println("Mensaje recibido: [" + text + "]");
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