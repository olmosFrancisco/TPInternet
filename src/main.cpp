#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ThingSpeak.h>

// --- INCLUSIONES DE DHT ---
#include <DHT.h>   // Librería principal del sensor DHT
// ...
DHT dht(pinDHT, DHTTYPE); // Objeto creado correctamente

// --- DEFINICIONES DE DHT ---
#define DHTTYPE DHT22 // O DHT11, dependiendo del sensor que uses
const int pinDHT = 33; // Sensor H y T
DHT dht(pinDHT, DHTTYPE); // Crea el objeto DHT
const char* ssid = "mi wifi";
const char* password = "contraseña";
const char* botToken = "7959938424:AAE_4uKccKkv_ZgOWJ9gbgkmisJK8MN9lXI";
const unsigned long SCAN_TIME = 1000;

const int pinLEDAzul = 2;
const int pinLEDVerde = 23;
const int pinDHT = 33; //Sensor H y T
const int pinADC = 32; // Potenciometro
const int pinSCL = 22; //display
const int pinSDA = 21; //display


//se necesita cifrar la comunicación porque telegram trabaja con https

WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);
unsigned long previous;

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
            json += "[\"Estado Verde\", \"Estado Azul\", \"Estado Pot\"]";

            json += "[\"Estado sensor\",\"Enviar HyT\"]";
            json += "]";
      //manda como texto lo que dice el boton json por lo que se debe cambiar las opciones
      bot.sendMessageWithReplyKeyboard(chat_id, "Elija una opción", "", json);
      //mensaje de bienvenida + mostrar opciones de MENU:
      //el mensaje va para un chat_id concreto
      bot.sendMessage(chat_id, "Chat iniciado! \n Comandos /ledOn: enciende LED \n /ledOff: apagar LED");
      
    }
    else if(text == "Verde ON" || text == "led23on"){
      digitalWrite(pinLED23, HIGH);
      bot.sendMessage(chat_id, "\n LED verde encendido");
    }
    else if(text == "Verde OFF" || text == "led23off"){
      digitalWrite(pinLED23, LOW);
      bot.sendMessage(chat_id, "\n LED verde apagado");
    }
    else if(text == "Azul ON" || text == "led2on"){
      digitalWrite(pinLED2, HIGH);
      bot.sendMessage(chat_id, "\n LED azul encendido");
    }
    else if(text == "Azul OFF" || text == "led2off"){
      digitalWrite(pinLED2, LOW);
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
      
    }

    else if(text == "Estado Verde" || text == "/displayled23"){
            
            // 1. Leer el estado actual del pin
            int estado = digitalRead(pinLEDVerde);
            String mensaje = (estado == HIGH) ? "LED Verde: ON" : "LED Verde: OFF";

            // 2. Limpiar la pantalla y configurar el texto
            display.clearDisplay();
            display.setTextSize(2); // Usar tamaño 2 para que sea más visible
            display.setTextColor(SSD1306_WHITE);
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
            int estado = digitalRead(pinLEDAzul);
            String mensaje = (estado == HIGH) ? "LED Azul: ON" : "LED Azul: OFF";

            // 2. Limpiar la pantalla y configurar el texto
            display.clearDisplay();
            display.setTextSize(2); // Tamaño 2 (más visible)
            display.setTextColor(SSD1306_WHITE);
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
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0); 
            display.println("--- POTENCIOMETRO ---");

            // 5. Mostrar los valores en la pantalla
            display.setTextSize(2); // Aumenta el tamaño para el valor principal
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
                display.setTextSize(2);
                display.setTextColor(SSD1306_WHITE);
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
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0); 
            display.println("--- DHT22 ---");

            // 5. Mostrar los valores de T y H
            display.setTextSize(2); // Usar tamaño 2 para los valores principales
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





void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinLEDAzul, OUTPUT);
  pinMode(pinLEDVerde, OUTPUT);
  
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi");
  //necesita un certificado para cifrar la comunicación https 
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); //la clave se la da la libreria de telegram
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  //estado es: CONNECT:
  Serial.println("\nConectado a la red WiFi");
  Serial.println("Direccion IP: "+ WiFi.localIP().toString());

  ThingSpeak.begin(secured_client);
  // INICIALIZAR I2C CON PINES PERSONALIZADOS
  Wire.begin(pinSDA, pinSCL); 

  // INICIALIZAR LA PANTALLA
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C es la dirección I2C común
      Serial.println(F("Error: No se pudo encontrar el display SSD1306."));
      // Si no se encuentra, detiene la ejecución 
      for (;;);
  }

  dht.begin();

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