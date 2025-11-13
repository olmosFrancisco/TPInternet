#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ThingSpeak.h>

// --- INCLUSIONES DE BIBLIOTECAS  ---
#include <Wire.h>                  
#include <Adafruit_Sensor.h>       
#include <DHT.h>
#include <Adafruit_SH110X.h>      


// --- DEFINICIONES DE PINES Y CONSTANTES ---

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



// --- CREACIÓN DE OBJETOS ---
DHT dht(pinDHT, DHTTYPE); //

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);


// --- CONFIGURACIÓN DE RED Y BOT ---
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
const char* botToken = "8312206932:AAHm2N1lDyhAjhgEXM2IrMaXnwBy7jp8t-k";
const unsigned long SCAN_TIME = 500;

//se necesita cifrar la comunicación porque telegram trabaja con https
WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);
unsigned long previous;

const char* WriteAPIKey = "KTCSTHIBOLXOP5Q2";     //Write API Key de vuestro canal.

WiFiClient clienteThingSpeak;

// --- FUNCIÓN PARA MANEJAR MENSAJES ---

// Función que muestra un mensaje temporal y vuelve al mensaje de espera
void showTempMessage(String line1, String line2 = "", int tiempo = 2000) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(10, 20);
  if (line2 != "") display.println(line2);
  display.display();

  delay(tiempo); 

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
    String chat_id = bot.messages[i].chat_id; 
    //dame todos los mensajes
    String text = bot.messages[i].text; 
    
    if(text == "/start"){
    
      String json = "[";
            json += "[\"Verde ON\", \"Verde OFF\"],";
            json += "[\"Azul ON\", \"Azul OFF\"],";
            json += "[\"Leer Temp/Hum\", \"Leer Potenciometro\"],";
            json += "[\"Estado Verde\", \"Estado Azul\", \"Estado Pot\"],"; 
            json += "[\"Estado sensor\",\"Enviar HyT\"]";
            json += "]";
      
      bot.sendMessageWithReplyKeyboard(chat_id, "Elija una opción", "", json);
  
      
    }
      else if(text == "Verde ON" || text == "/led23on"){
      showTempMessage("Encendiendo Luz Verde", "", 2000); 
      digitalWrite(pinLEDVerde, HIGH);
      showTempMessage("Luz Verde Encendida", "", 4000); 
      bot.sendMessage(chat_id, "LED verde encendido");
      Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Verde OFF" || text == "led23off"){
    showTempMessage("Apagando Luz Verde", "", 2000);
    digitalWrite(pinLEDVerde, LOW);
    showTempMessage("Luz Verde Apagada", "", 4000);
    bot.sendMessage(chat_id, "LED verde apagado");
    Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Azul ON" || text == "/led2on"){
        showTempMessage("Encendiendo Luz Azul", "", 2000);
        digitalWrite(pinLEDAzul, HIGH);
        showTempMessage("Luz Azul Encendida", "", 4000);
        bot.sendMessage(chat_id, "LED azul encendido");
        Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Azul OFF" || text == "/led2off"){
        showTempMessage("Apagando Luz Azul", "", 200);
        digitalWrite(pinLEDAzul, LOW);
        showTempMessage("Luz Azul Apagada", "", 4000);
        bot.sendMessage(chat_id, "LED azul apagado");
        Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Leer Temp/Hum" || text == "/dht22"){
        showTempMessage("Leyendo Temperatura", "y Humedad", 2000);
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
            bot.sendMessage(chat_id, "Error al leer del sensor DHT!");
            return;
        }
        showTempMessage("Temp: " + String(t,1) + " C", "Hum: " + String(h,1) + " %", 4000);
        bot.sendMessage(chat_id, "Temperatura: " + String(t) + " °C\nHumedad: " + String(h) + " %");
    }

    else if(text == "Leer Potenciometro" || text == "/pote"){
        showTempMessage("Leyendo Potenciometro", "", 2000);
        int valorADC = analogRead(pinADC);
        showTempMessage("Potenciometro: " + String(valorADC), "", 4000);
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
            
      showTempMessage("Consultando Estado...", "", 2000); 

      // Leer el estado actual del pin
      int estado = digitalRead(pinLEDVerde);
      String mensaje = (estado == HIGH) ? "Led Verde: ON" : "Led Verde: OFF";

      // Mostrar el estado en pantalla temporalmente
      showTempMessage("- ESTADO LED VERDE -", mensaje, 4000); 

      // Confirmación por Telegram
      bot.sendMessage(chat_id, "Mostrando estado del LED Verde en OLED.");
      Serial.println("Mensaje recibido: [" + text + "]");
    }

        else if(text == "Estado Azul" || text == "/displayled2"){
        
        showTempMessage("Consultando Estado...", "", 2000);

        
        int estado = digitalRead(pinLEDAzul);
        String mensaje = (estado == HIGH) ? "LED Azul: ON" : "LED Azul: OFF";

       
        showTempMessage("-- ESTADO LED AZUL --", mensaje, 4000);

        
        bot.sendMessage(chat_id, "Mostrando estado del LED Azul en OLED.");
        Serial.println("Mensaje recibido: [" + text + "]");
    }

    else if(text == "Estado Pot" || text == "/displaypote"){
        
        showTempMessage("Consultando", "Potenciometro", 2000);

        // Leer valor del ADC y convertir a porcentaje
        int valorADC = analogRead(pinADC);
        long porcentaje = map(valorADC, 0, 4095, 0, 100);

        
        showTempMessage("Potenciometro:", String(porcentaje) + " %", 4000);

        
         bot.sendMessage(chat_id, "Potenciómetro: " + String(porcentaje) + " % (" + String(valorADC) + " ADC)");
          Serial.println("Mensaje recibido: [" + text + "]");
    }

       else if (text == "Estado sensor" || text == "/displaydht22") {
        
        showTempMessage("Consultando", "Sensor DHT22", 2000);
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            showTempMessage("Sensor", "APAGADO", 3000);
            bot.sendMessage(chat_id, "El sensor DHT22 está APAGADO");
        } else {
            showTempMessage("Sensor", "ENCENDIDO", 3000);
            bot.sendMessage(chat_id, "El sensor DHT22 está ENCENDIDO");
        }

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
  display.setTextSize(1); 
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 50); 
  display.println("GRUPO 6 - TP02 IoT");


  display.display();
  delay(5000); 
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
  int textoX = (anchoDisplay - texto.length() * 6) / 2; 
  int textoY = 20; 

  // --- Parámetros de los puntos ---
  int numPuntos = 5;
  int radio = 4;
  int espacio = 12;
  int anchoTotal = (numPuntos - 1) * espacio;
  int xBase = (anchoDisplay - anchoTotal) / 2;
  int yBase = 45; 

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
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

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
  Wire.begin(pinSDA, pinSCL);

  if (!display.begin(0x3C, true)) {
    Serial.println(F("Error: No se pudo inicializar el display SH110X."));
    while (true);
  }

  display.clearDisplay();
  display.display();

  // --- MENSAJE DE BIENVENIDA ---
  mostrarInicio();               
  mostrarAnimacionEspera(3000);  


  // --- INICIALIZAR SENSOR DHT ---
  dht.begin();
  Serial.println(F("Sensor DHT listo."));


  // --- INICIALIZAR THINGSPEAK (CLIENTE PERSONALIZADO) ---
  ThingSpeak.begin(clienteThingSpeak); 
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
    int n = bot.getUpdates(bot.last_message_received + 1);
    
    while(n){
      handleMessages(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }

    previous = millis();
  }
}