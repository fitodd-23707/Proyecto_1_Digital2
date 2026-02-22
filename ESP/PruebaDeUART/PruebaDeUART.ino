#include "config.h"

#define PUB_STEP_MS 2000
unsigned long lastPub = 0;
int pubIdx = 0;
#define RXD2 16 // Pin RX del ESP32 (UART2)
#define TXD2 17 // Pin TX del ESP32 (UART2)
#define separador '!'

unsigned long lastUpdate = 0;

char recieved = '0';
char humedad_presente = 0;
char estado_motor = 0;
char datam = '0';

String data;
String NewData;
String campo;
String DatatoAdafruit[7];

int inicio, fin;
int IntTemp;

AdafruitIO_Feed *hume_suelo = io.feed("smartgarden.hume-suelo");
AdafruitIO_Feed *bomba_agua = io.feed("smartgarden.bomba-agua");
AdafruitIO_Feed *temp_garden = io.feed("smartgarden.tem-garden");
AdafruitIO_Feed *uv_garden = io.feed("smartgarden.uv-garden");
AdafruitIO_Feed *techo = io.feed("smartgarden.techo");
AdafruitIO_Feed *window = io.feed("smartgarden.window");
//
AdafruitIO_Feed *mn_bomba = io.feed("smartgarden.mn-bomba");
AdafruitIO_Feed *mn_techo = io.feed("smartgarden.mn-techo");
AdafruitIO_Feed *mn_window = io.feed("smartgarden.mn-window");
AdafruitIO_Feed *modo = io.feed("smartgarden.modo");


void setup() {
  
  // start the serial connection
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  modo->onMessage(handleMessage);
  mn_bomba->onMessage(handleMessage);
  mn_techo->onMessage(handleMessage);
  mn_window->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  modo->get();
  mn_bomba->get();
  mn_techo->get();
  mn_window->get();

  inicio = 0;

}

void loop() {

  io.run();

  if (Serial2.available()) {
    data = Serial2.readStringUntil('\n'); // Lee hasta encontrar nueva línea
    Serial.print("Recivido");
    Serial.println(data); // Muestra en el monitor serie de la PC

    for (int o = 0; o < 6; o++){
      Serial.println("Entramos al for");
      fin = data.indexOf("!");
      DatatoAdafruit[o] = data.substring(inicio, fin);

      inicio = fin + 1;
      data = data.substring(inicio);

      inicio = 0;
    }
  }

  if (millis() - lastPub >= PUB_STEP_MS) {
  lastPub = millis();
  Serial.println("Se publica");

  switch (pubIdx) {
    case 0: temp_garden->save(DatatoAdafruit[0].toInt()); break;
    case 1: hume_suelo->save(DatatoAdafruit[1]); break;
    case 2: uv_garden->save(DatatoAdafruit[2]); break;
    case 3: bomba_agua->save(DatatoAdafruit[3]); break;
    case 4: techo->save(DatatoAdafruit[4]); break;
    case 5: window->save(DatatoAdafruit[5]); break;  // ← agregado
  }

  pubIdx = (pubIdx + 1) % 6;
}

}

// this function is called whenever a 'counter' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.
void handleMessage(AdafruitIO_Data *datam) {

  Serial.print("received <- ");
  Serial.println(datam->value());
  Serial2.println(datam->value());

}