#include <WiFi.h>
#include <SPI.h>

const int ledPin = 5;
const char ssid[] = "maszt_sygnalowy";
int status;

WiFiServer server(21);

void setup() {
  Serial.begin(115200);

  uint8_t networkList = WiFi.scanNetworks();

  for(uint8_t i =0;i<networkList;i++)
  Serial.printf("%d -> %s\n", i+1, WiFi.SSID(i));


    // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    exit(0);
  }

  IPAddress ip(192, 168, 1, 2);  
  WiFi.config(ip);

  WiFi.begin(ssid);
  Serial.printf("\nConnecting\n");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(100);
  }
  Serial.printf("Connection succesfull\n");
 

  server.begin();
  Serial.print("Connected to wifi. My address:");
  IPAddress myAddress = WiFi.localIP();
  Serial.println(myAddress);
}

uint8_t x =0;
void loop() {

  // listen for incoming clients

  WiFiClient client = server.available();

  if (client) {

    Serial.println("new client");

    // an http request ends with a blank line

    bool currentLineIsBlank = true;

    while (client.connected()) {

      if (client.available()) {

        char c = client.read();

        Serial.write(c);

        // if you've gotten to the end of the line (received a newline

        // character) and the line is blank, the http request has ended,

        // so you can send a reply

        if (c == '\n' && currentLineIsBlank) {

          // send a standard http response header

          client.println("HTTP/1.1 200 OK");

          client.println("Content-Type: text/html");

          client.println("Connection: close");  // the connection will be closed after completion of the response

          client.println("Refresh: 5");  // refresh the page automatically every 5 sec

          client.println();

          client.print("<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
            "<title>Increasing Numbers with Delay</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>Increasing Numbers</h1>\n"
            "<div id=\"numberDisplay\"></div>\n"
            "<script>\n"
            "const displayElement = document.getElementById('numberDisplay');\n"
            "let cspace = \"&nbsp;\";\n"
            "let cleft = \"/\";\n"
            "let cright = \"\\\\\";\n"
            "let ctop = \"^\";\n"
            "let cbottom = \"-\";\n"
            "let count = 0;\n" // Start count at 0 for the first iteration
            "const intervalId = setInterval(() => {\n"
            "    var str = \" \";\n"
            "    if (count === 0) {\n"
            "        str = cspace.repeat(20) + ctop + cspace.repeat(20);\n"
            "    } else if (count === 20) {\n"
            "        str = cbottom.repeat(41);\n"
            "    } else {\n"
            "        str = cspace.repeat(20 - count) + cleft + cspace.repeat(count * 2) + cright + cspace.repeat(20 - count);\n"
            "    }\n"
            "    displayElement.innerHTML += str + '<br>'; // Append the current count\n"
            "    count++;\n"
            "    if (count > 20) { // Stop the interval after reaching 41\n"
            "        clearInterval(intervalId);\n"
            "    }\n"
            "}, 200); // 200 milliseconds\n"
            "</script>\n"
            "</body>\n"
            "</html>\n");

          break;

        }

        if (c == '\n') {

          // you're starting a new line

          currentLineIsBlank = true;

        } else if (c != '\r') {

          // you've gotten a character on the current line

          currentLineIsBlank = false;

        }

      }

    }

    // give the web browser time to receive the data

    delay(15);

    // close the connection:

    client.stop();

    Serial.println("client disconnected");

  }
}

