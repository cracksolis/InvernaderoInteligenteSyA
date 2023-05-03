#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"
#include <ESP32Servo.h> // Incluimos la biblioteca del servo motor


// Se define pin para DHT11(sensor de humedad y temperatura)
#define DHTPIN 4     


//Se define los pines para los leds de emergencia
#define ledVerde 5
#define ledRojo 19
#define ledAmarillo 18

//Pin digital luz led
int luzled = 2;

//Se define sensor de luz
int sensorluz = 34;


//Se define pin para servomotor
#define servomotor 21

//Se define sensor humedad tierra
int humTierra = 32;

//Se define pin para alarma
const int bocina = 13;

//Se define ventilador
int ventilador = 12;

//Se define bomba de agua
extern const int bomba = 22;

String chatId = "5101914898"; // ID del chat de destino

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

//Se crean las funciones para telegram
// Wifi network station credentials
#define WIFI_SSID "iPhone de Alejandro"//nombre de la red
#define WIFI_PASSWORD "alejandro" //contrasena del wifi
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "6176346107:AAGTPGjWfdg9SN2UzsgFl9i16PahDQztETQ"
const unsigned long BOT_MTBS = 1000; // mean time between scan messages
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (from_name == "")

    from_name = "Guest";
    if (text == "/ledon")
    {
    
    bot.sendMessage(chat_id, "Led is ON", "");
    }
    if (text == "/ledoff")
    {
    
    bot.sendMessage(chat_id, "Led is OFF", "");
    }


    //condiciones para  temperatura, humedad, y humedadtierra

    if (text == "/temp")
    {
    float t = dht.readTemperature();
    char cadena[50];
    sprintf(cadena,"La temperatura es %0.2f", t);
    bot.sendMessage(chat_id, cadena);
    }
    if (text == "/hum")
    {
    float h = dht.readHumidity();
    char cadena[50];
    sprintf(cadena,"La humedad del aire es %0.2f", h);
    bot.sendMessage(chat_id, cadena);
    }

    if (text == "/humT")
    {
    float sensorValue = map(analogRead(humTierra), 4090, 0, 0, 100);
    char cadena[50];
    sprintf(cadena,"La humedad de la tierra es %0.2f", sensorValue);
    bot.sendMessage(chat_id, cadena);
    }
    //fin de condiciones
    if (text == "/start")
    {
      String welcome = "Bienvenido a invernadero inteligente de Santiago y Alejandro.\n";
      welcome += "selecciona: .\n\n";
      welcome += "/temp : Visualiza temperatura Temp\n";
      welcome += "/hum : Visualiza humedad Hum\n";
      welcome += "/humT : Visualiza humedad de la tierra HumT\n";

      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}
//----------------------------------

// Creamos un objeto Servo
Servo myservo;

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));

  dht.begin();

   // Adjuntamos el objeto myservo al pin de salida
  myservo.attach(servomotor);

  // Configurar el pin del relé como salida
  pinMode(ventilador, OUTPUT); 

 // Configurar el pin del rele como salida
   pinMode(bomba, OUTPUT); 
   
//configuramos los led de emergencia
   pinMode(ledVerde, OUTPUT);
   pinMode(ledRojo, OUTPUT);
   pinMode(ledAmarillo, OUTPUT);
    pinMode(luzled, OUTPUT);
//configuramos la bocina 
   pinMode(bocina, OUTPUT);   

//configuramos el sensor de humedad tierra
  pinMode(humTierra, INPUT);     

//telegram
// attempt to connect to Wifi network:
Serial.print("Connecting to Wifi SSID ");
Serial.print(WIFI_SSID);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
while (WiFi.status() != WL_CONNECTED)
{
Serial.print(".");
delay(500);
}
Serial.print("\nWiFi connected. IP address: ");
Serial.println(WiFi.localIP());
Serial.print("Retrieving time: ");
configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
time_t now = time(nullptr);
while (now < 24 * 3600)
{
Serial.print(".");
delay(100);
now = time(nullptr);
}
Serial.println(now);
}

void loop(){
  // Wait a few seconds between measurements.
  delay(2000);

  //telegram
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
  {
    Serial.println("got response");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  bot_lasttime = millis();
  }

  //Se almacena el valor del sensor y convertimos a porcentaje
  int sensorValue = map(analogRead(humTierra), 4090, 0, 0, 100);

  //Se almacena el valor del sensor de luz
  // variable para almacenar el valor de la fotocelda
  int celdaValue = analogRead(sensorluz);
  

  // ¡Leer la temperatura o la humedad toma alrededor de 250 milisegundos!
  //Las lecturas del sensor también pueden tener hasta 2 segundos de antigüedad (es un sensor muy lento)
  float h = dht.readHumidity();
  // Leer la temperatura como grados Celsius (el valor predeterminado)
  float t = dht.readTemperature();
  // Lea la temperatura como Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Compruebe si alguna lectura falló y salga antes (para volver a intentarlo).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("¡Error al leer sensor DHT!"));
    return;
  }

 

  Serial.print(F("Humedad: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("%  Humedad tierra: "));
  Serial.print(sensorValue);
  Serial.print(F("%"));
  Serial.print(F("%  luz: "));
  Serial.print(celdaValue);
  
  
  
  
  //condicion para temperatura
  if (t>30){
      // Encender el relé durante 1 segundo
      digitalWrite(ventilador, HIGH);
      Serial.println("Relé encendido ventilador"); 
      //delay(1000);
      
      //encendemos el led rojo
      digitalWrite(ledRojo, HIGH);
      digitalWrite(ledAmarillo, LOW); 

      //tone(bocina,523,300);
      digitalWrite(bocina, HIGH);
      delay(500);     

        // Enviar mensaje a un chat
        
        // Mensaje a enviar
        char cadena[50];
        sprintf(cadena,"¡ALERTA: TEMPERATURA ALTA! %0.2f", t);
        if (bot.sendMessage(chatId,cadena)) {
          Serial.println("Mensaje enviado correctamente");
        } else {
          Serial.println("Error al enviar el mensaje");
        }

        delay(5000); // Esperar 5 segundos antes de enviar otro mensaje              
      
  }else{
      if(t>20 & t<30){
        //encendemos led amarillo
         digitalWrite(ledAmarillo, HIGH);
         digitalWrite(ventilador, LOW);
         digitalWrite(ledRojo, LOW);
         digitalWrite(ledVerde, LOW);      
         digitalWrite(bocina, LOW);             
      }else{
      // Apagar el relé durante 1 segundo
            digitalWrite(ventilador, LOW);
            Serial.println("Relé apagado ventilador");
            //delay(1000);      
            digitalWrite(ledVerde, HIGH);
            digitalWrite(ledRojo, LOW);
            digitalWrite(ledAmarillo, LOW);  
            digitalWrite(bocina, LOW);         
      }           
  }
  
  //condicional para humedad 
    if (sensorValue<0){
         
        //servomotor
        for (int pos = 0; pos <= 180; pos++) {
          myservo.write(pos); // Escribimos la posición en el servomotor
          delay(15); // Esperamos 15 milisegundos para que se mueva
        }
  
        // Giramos el servomotor de 180 a 0 grados en incrementos de 1 grado
        for (int pos = 180; pos >= 0; pos--) {
          myservo.write(pos); // Escribimos la posición en el servomotor
          delay(15); // Esperamos 15 milisegundos para que se mueva
        }  
        //BOMBA DE AGUA    
        digitalWrite(bomba, HIGH);
        Serial.println("bomba Encendida"); 

        // Enviar mensaje a un chat
        
         // Mensaje a enviar
        char cadena[50];
        sprintf(cadena,"¡ALERTA: HUMEDAD EN UN PORCENTAJE BAJO! %0.2f", h);
        if (bot.sendMessage(chatId,cadena)) {
          Serial.println("Mensaje enviado correctamente");
        } else {
          Serial.println("Error al enviar el mensaje");
        }

        //delay(5000); // Esperar 5 segundos antes de enviar otro mensaje 
                    
    }else{
        digitalWrite(bomba, LOW);
      Serial.println("bomba apagado");
      //delay(1000);            
  }
 
  //condicion para fotocelda
   if(celdaValue>2000){
      digitalWrite(luzled, HIGH);
    }else{
       digitalWrite(luzled, LOW); 
    }
  
}

