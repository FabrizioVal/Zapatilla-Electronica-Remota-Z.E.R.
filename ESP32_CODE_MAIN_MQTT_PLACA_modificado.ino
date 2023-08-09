#include <WiFi.h>
#include <PubSubClient.h>
#include "BluetoothSerial.h"

// ---------  Depuracion  ---------
#define DEBUG 1 // Cambiar el 0 por un 1 si se hacen prueb  as con el esp conectado al puerto serial.

// ---------  Componentes OUTPUT & INPUT de Control  ---------
#define PIN_BOTON 22
#define PIN_LED_V 18
#define PIN_LED_R 21
#define PIN_LED_ESP 2
#define PIN_RELAY_1 16
#define PIN_RELAY_2 27
#define PIN_RELAY_3 33
#define PIN_RELAY_4 17
bool button_state;

// ---------  WiFi & Broker MQTT  ---------
WiFiClient espClient;                                         // Definir espClient como un objeto de la clase WiFiClient. c6ea7c
PubSubClient client(espClient);                               // Definir espClient como objeto de la clase PubSubClient. 271193841
const char *ssid = "Electro 5G";                                  // Nombre de la red WiFi.
const char *password = "ivanagosto";                           // Contraseña de la red WiFi.
const char *mqtt_broker = "pruebacolegio.cloud.shiftr.io";  // Direccion del broker (SHIFTR, EMQX U OTRO BROKER).
const char *mqtt_username = "pruebacolegio";                // Username para autenticacion.
const char *mqtt_password = "cFjlYtLFaBfuKsqW";               // Password para autenticacion.
const int mqtt_port = 1883;                                   // Puerto MQTT sobre TCP.
int PAYLOADINT;

// ---------  Bluetooth  ---------
BluetoothSerial SerialBluetoothTerminal;
String Nombre, Clave;   // En Nombre y Clave se va a almacenar el nombre y la clave de la nueva red WiFi a la que se desea conectar luego de haber presionado el boton. El nombre y la clave se van a pasar a traves de Bluetooth.
bool salir_nombre = 0;  // Variable que permite salir del bucle while.
bool salir_clave = 0;   // Variable que permite salir del bucle while.
int LengthN, LengthC;   // Variable que permite saber cuantos caracteres tiene el nombre y la contrseña de la nueva red WiFi.

void try_to_connect_WiFi_first(String w_name, String w_password) {

  WiFi.begin(ssid, password);  // El ESP va a intentar conectarse con las credenciales del WiFi ya establecidas en el programa.

  while (WiFi.status() != WL_CONNECTED && digitalRead(PIN_BOTON) == 1) {
    if (DEBUG) Serial.println("Connecting to WiFi..");
    digitalWrite(PIN_LED_R, HIGH);
    delay(900);
    digitalWrite(PIN_LED_R, LOW);
    delay(900);
  }
}

void try_to_connect_WiFi_second(String w_name, String w_password) {

  const char *nombre = Nombre.c_str();  // El ESP copia el nombre de la nueva red WiFi sin el caracter NEWLINE (\n) con la funcion .c_str(), almacenandola en la variable nombre.
  const char *clave = Clave.c_str();    // Lo mismo pasa con la variable clave. -- Para que la funcion WiFi.begin() reconozca estas variables, tienen que ser del tipo const char *. --
  if (DEBUG) {
    Serial.println("....");
    Serial.println(nombre);
    Serial.println("....");
    Serial.println("....");
    Serial.println(clave);
    Serial.println("....");
  }
  WiFi.begin(nombre, clave);

  while (WiFi.status() != WL_CONNECTED && digitalRead(PIN_BOTON) == 1) {
    if (DEBUG) Serial.println("Connecting to WiFi..");
    digitalWrite(PIN_LED_R, HIGH);
    delay(900);
    digitalWrite(PIN_LED_R, LOW);
    delay(900);
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (DEBUG) Serial.println("Connected to the WiFi network");
    digitalWrite(PIN_LED_V, HIGH);

  }
}

void try_to_connect_MQTT(String mqtt_url_broker, int mqtt_puerto) {

  if (WiFi.status() == WL_CONNECTED) {
    if (DEBUG) Serial.println("Connected to the WiFi network");  // Si el ESP logro conectarse a la red WiFi, intntara conectarse al broker MQTT.
    digitalWrite(PIN_LED_V, HIGH);

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      if (DEBUG) Serial.printf("The client ------ %s ------ connects to the public mqtt broker\n", client_id.c_str());  // here the macAddress of the esp32 (which is a unique identifier for the esp32) is printed in the string variable client_id. then client_id is passed to a char arrays with the function c_str().

      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {  // here the esp32 will try to connect with the broker
        if (DEBUG) Serial.println("Public emqx mqtt broker connected");
      } else {
        if (DEBUG) {
          Serial.print("failed with state ");
          Serial.print(client.state());
        }
        delay(2000);
      }
    }

    // --- Funciones Publish & Subscribe ---
    client.publish("TOMA1", "Hola Shiftr y Celular soy ESP32 :] , escribiendo en TOMA1.");
    client.subscribe("TOMA1");
    client.subscribe("TOMA2");
    client.subscribe("TOMA3");
    client.subscribe("TOMA4");
  }
}

String recibir_nombre_redwifi_bluetoothterminal() {
  if (SerialBluetoothTerminal.available()) {
    salir_nombre = 1;
    Nombre = SerialBluetoothTerminal.readStringUntil('\n');
    if (DEBUG) {
      Serial.println("-------");
      Serial.println(Nombre);
      Serial.println("-------");
    }
  }
  return Nombre;
}

String recibir_clave_redwifi_bluetoothterminal() {
  if (SerialBluetoothTerminal.available()) {
    salir_clave = 1;
    Clave = SerialBluetoothTerminal.readStringUntil('\n');
    if (DEBUG) {
      Serial.println("-------");
      Serial.println(Clave);
      Serial.println("-------");
    }
  }
  return Clave;
}

void output_input_pins_setup() {
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_RELAY_3, OUTPUT);
  pinMode(PIN_RELAY_4, OUTPUT);
  digitalWrite(PIN_RELAY_1, HIGH);  // RELAY 1 APAGADO.
  digitalWrite(PIN_RELAY_2, HIGH);  // RELAY 2 APAGADO.
  digitalWrite(PIN_RELAY_3, HIGH);  // RELAY 3 APAGADO.
  digitalWrite(PIN_RELAY_4, HIGH);  // RELAY 4 APAGADO.
  pinMode(PIN_LED_V, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_ESP, OUTPUT);
  pinMode(PIN_BOTON, INPUT);
}

void setup() {
  // --- Setup pines Input & Output ---
  output_input_pins_setup();
  Serial.begin(115200);
  if (DEBUG) Serial.println("Estoy aca");
  // --- Conectar a WiFi & Conectar a Broker MQTT ---
  try_to_connect_WiFi_first(ssid, password);
  try_to_connect_MQTT(mqtt_broker, mqtt_port);
}

void callback(char *topic, byte *payload, unsigned int length) {  // here the message from the broker is trasmitted to the esp32 with the variable payload

  if (strcmp(topic, "TOMA1") == 0) {
    digitalWrite(PIN_LED_ESP, HIGH);
    delay(500);
    digitalWrite(PIN_LED_ESP, LOW);
    if (DEBUG) {
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message: ");
    }
    for (int i = 0; i < length; i++) {
      if (DEBUG) Serial.print((char)payload[i]);
    }
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0'; // se le agrega un NULL TERMINATED al final de la string, ya que de esa forma el programa sabe en donde termina la string.
    PAYLOADINT = atoi(payload_string);
    if (DEBUG) {
      Serial.println();
      Serial.print("Payload as a integer:");
      Serial.print(PAYLOADINT);
      Serial.println();
      Serial.println("-----------------------");
    }

    if (PAYLOADINT == 1) {
      digitalWrite(PIN_RELAY_1, LOW);  // PRENDER RELAY 1
      client.publish("NOTIFICACION", "TOMA 1 ACTIVADO - ESP32");
    } else if (PAYLOADINT == 0) {
      digitalWrite(PIN_RELAY_1, HIGH);  // APAGAR RELAY 1
      client.publish("NOTIFICACION", "TOMA 1 DESACTIVADO - ESP32");
    }
  }

  else if (strcmp(topic, "TOMA2") == 0) {
    digitalWrite(PIN_LED_ESP, HIGH);
    delay(500);
    digitalWrite(PIN_LED_ESP, LOW);
    if (DEBUG) {
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message:");
    }
    for (int i = 0; i < length; i++) {
      if (DEBUG) Serial.print((char)payload[i]);
    }
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    PAYLOADINT = atoi(payload_string);
    if (DEBUG) {
      Serial.println();
      Serial.print("Payload as a integer:");
      Serial.print(PAYLOADINT);
      Serial.println();
      Serial.println("-----------------------");
    }

    if (PAYLOADINT == 1) {
      digitalWrite(PIN_RELAY_2, LOW);  // PRENDER RELAY 2.
      client.publish("NOTIFICACION", "TOMA 2 ACTIVADO - ESP32");
    } else if (PAYLOADINT == 0) {
      digitalWrite(PIN_RELAY_2, HIGH);  // APAGAR RELAY 2.
      client.publish("NOTIFICACION", "TOMA 2 DESACTIVADO - ESP32");
    }
  }

  else if (strcmp(topic, "TOMA3") == 0) {
    digitalWrite(PIN_LED_ESP, HIGH);
    delay(500);
    digitalWrite(PIN_LED_ESP, LOW);
    if (DEBUG) {
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message:");
    }
    for (int i = 0; i < length; i++) {
      if (DEBUG) Serial.print((char)payload[i]);
    }
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    PAYLOADINT = atoi(payload_string);
    if (DEBUG) {
      Serial.println();
      Serial.print("Payload as a integer:");
      Serial.print(PAYLOADINT);
      Serial.println();
      Serial.println("-----------------------");
    }

    if (PAYLOADINT == 1) {
      digitalWrite(PIN_RELAY_3, LOW);  // PRENDER RELAY 3.
      client.publish("NOTIFICACION", "TOMA 3 ACTIVADO - ESP32");
    } else if (PAYLOADINT == 0) {
      digitalWrite(PIN_RELAY_3, HIGH);  // APAGAR RELAY 3.
      client.publish("NOTIFICACION", "TOMA 3 DESACTIVADO - ESP32");
    }
  }

  else if (strcmp(topic, "TOMA4") == 0) {
    digitalWrite(PIN_LED_ESP, HIGH);
    delay(500);
    digitalWrite(PIN_LED_ESP, LOW);
    if (DEBUG) {
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message:");
    }
    for (int i = 0; i < length; i++) {
      if (DEBUG) Serial.print((char)payload[i]);
    }
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    PAYLOADINT = atoi(payload_string);
    if (DEBUG) {
      Serial.println();
      Serial.print("Payload as a integer:");
      Serial.print(PAYLOADINT);
      Serial.println();
      Serial.println("-----------------------");
    }

    if (PAYLOADINT == 1) {
      digitalWrite(PIN_RELAY_4, LOW);  // PRENDER RELAY 4.
      client.publish("NOTIFICACION", "TOMA 4 ACTIVADO - ESP32");
    } else if (PAYLOADINT == 0) {
      digitalWrite(PIN_RELAY_4, HIGH);  // APAGAR RELAY 4.
      client.publish("NOTIFICACION", "TOMA 4 DESACTIVADO - ESP32");
    }
    
  }
}

void loop() {
  if (DEBUG) Serial.println("Estoy en el vloop");

  while (digitalRead(PIN_BOTON) == 1 && WiFi.status() == WL_CONNECTED) {  // EL PROGRAMA VA ENTRAR Y REPETIR EL WHILE SIMPRE Y CUANDO NO SE ESTE PRESIONANDO EL BOTON Y EL ESP32 ESTE CONECTADO A UNA RED WIFI.
    client.loop();
  }

  while (digitalRead(PIN_BOTON) == 0) {  // CUANDO SE PRESIONA EL BOTON, EL MODULO WIFI DEL ESP32 SE DESCONECTA Y COMIENZA LA CONFIGURACION POR BLUETOOTH.
    delay(500);
    if (DEBUG) Serial.println("WiFi desconectado, iniciando confg. Bluetooth..");
    WiFi.disconnect(true);  // TURN OFF WIFI
    WiFi.mode(WIFI_OFF);
    delay(900);
    SerialBluetoothTerminal.begin("ESP32test");  // Bluetooth device name
    if (DEBUG) Serial.println("Bluetooth activado, ahora puede emparejarlo con un dispositivo.");
    digitalWrite(PIN_LED_R, HIGH);
    delay(900);
    digitalWrite(PIN_LED_R, LOW);
  }

  while (!SerialBluetoothTerminal.connected()) {
    if (DEBUG) Serial.println("Connecting bluetooth..");
    digitalWrite(PIN_LED_R, HIGH);
    delay(900);
    digitalWrite(PIN_LED_R, LOW);
    delay(900);
    if (SerialBluetoothTerminal.connected()) {
      digitalWrite(PIN_LED_V, HIGH);

      if (DEBUG) Serial.println("BLUETOOTH CONECTADO");
      SerialBluetoothTerminal.println("ESP32 CONECTADO");
    }
  }

  SerialBluetoothTerminal.println("Ingresar NOMBRE de la red WiFi: ");
  salir_nombre = 0;
  while (salir_nombre == 0) {  // CUANDO EL NOMBRE DE LA RED LLEGUE AL ESP, ESTE while VA A DAR FALSO ---> HASTA QUE EL ESP NO RECIBA ALGUN MENSAJE DEL Bluetooth Terminal, EL PROGRAMA NO VA A SALIR DEL while.
    Nombre.remove(0);
    recibir_nombre_redwifi_bluetoothterminal();
  }

  SerialBluetoothTerminal.println("Ingresar CLAVE de la red WiFi: ");
  salir_clave = 0;
  while (salir_clave == 0) {  // CUANDO LA CLAVE DE LA RED LLEGUE AL ESP, ESTE while VA A DAR FALSO ---> HASTA QUE EL ESP NO RECIBA ALGUN MENSAJE DEL Bluetooth Terminal, EL PROGRAMA NO VA A SALIR DEL while.
    Clave.remove(0);
    recibir_clave_redwifi_bluetoothterminal();
  }

  SerialBluetoothTerminal.print("NOMBRE red WiFi: ");
  SerialBluetoothTerminal.println(Nombre);
  SerialBluetoothTerminal.print("CLAVE red WiFi: ");
  SerialBluetoothTerminal.println(Clave);
  SerialBluetoothTerminal.println();
  SerialBluetoothTerminal.print("Configuracion completada.");
  delay(1000);
  SerialBluetoothTerminal.end();
  if (DEBUG) Serial.println("Bluetooth desconectado");
  delay(1000);
  // THE PROBLEM IS THAT ALL MESSAGES FROM THE Bluetooth Terminal App HAVE AT THE END OF EVERY STRING A "\n", SO IN ORDER TO HANDLE THE Nombre AND Clave VARIABLE, THE ESP HAVE TO REMOVE THAT \n FROM THE MESSAGE.
  LengthN = Nombre.length();                  // FIRST, THE ESP NEEDS TO KNOW HOW LONG OR HOW MANY CHARACTERS THE VARIABLE Nombre HAS WITH THE String Object FUNCTION CALLED .length() .
  Nombre = Nombre.substring(0, LengthN - 1);  // HERE THE ESP TAKE OUT THE PART OF THE MASSAGE THAT IS NEEDED TO CONNECT TO THE NEW WIFI NETWORK THANKS TO THE .substring() FUNCTION AND ITS PARAMETERS --- .substring(point_where_it_should_start, point_where_it_should_end) ---.
                                              // HERE, THE point_where_it_should_end (LengthN) IT IS THE LENGTH PARAMETER THE IT HAVE BEEN TAKEN BEFORE MINUS ONE (- 1) BECAUSE IN THE END OF THE MESSAGE WE KNOW THERE IS A \n, SO WE JUST WANT TO TAKE THE WHOLE MESSAGE BEFORE THAT \n.
  LengthC = Clave.length();                   // HERE, THE ESP REPEATS THE SAME PROCESS BUT WITH Clave MESSAGE.
  Clave = Clave.substring(0, LengthC - 1);

  try_to_connect_WiFi_second(Nombre, Clave);
  delay(1000);
  try_to_connect_MQTT(mqtt_broker, mqtt_port);
  delay(1000);
}