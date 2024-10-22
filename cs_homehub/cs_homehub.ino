// #include <Arduino_JSON.h>

#include <WiFiManager.h>
#include <strings_en.h>
#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

//#include <apikey.h>

WiFiManager wm;

// Inputs to be tested in Serial Monitor
const int ERASE_WIFI_CREDENTIALS = -1;
const int CONNECT_HH = 5;
const int REGISTER_HH = 6;
const int GETLOCATION = 20;

// Latitude & Longitude
String lat;
String lon;

// Customized routes for the Captive Portal
void bindServerCallback() {
  wm.server->on("/", handleSetupRoute);
  //wm.server->on("/register", handleRegister);
  wm.server->on("/api/register", handleRegisterRequest);
}

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);  // 115200 badios

  wm.setWebServerCallback(bindServerCallback);
}

void loop() {
  // Getting value from Serial Monitor for debugging.
  // By default, debug_value will keep receiving zero.
  if (Serial.available() > 0) {
    int debug_value = Serial.parseInt();

    switch (debug_value) {
      case ERASE_WIFI_CREDENTIALS:
        wm.resetSettings();
        break;

      case CONNECT_HH:
        if (!establishWiFiConnection()) {
          Serial.println("Couldn't connect to the network");
        } else {
          Serial.println("Connected!");
          printNetworkInfo();
        }
        break;

      case REGISTER_HH:
        onDemandPortal();
        break;

      default:
        Serial.print("Se ha seleccionado el: ");
        Serial.println(debug_value);
    }
  }
}

/*
* This function works for any or generic APIs, It is not intended for sending data to the MIT Servers.
*/
String postData(String endpoint, String requestBody) {
  HTTPClient http;
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(requestBody);
  String response = "default string";
  String error;

  Serial.print("Endpoint: ");
  Serial.println(endpoint);

  if (httpResponseCode > 0) {
    response = http.getString();
  } else {
    error = "Error occurred while sending HTTP POST: " + String(http.errorToString(httpResponseCode));
    Serial.println(error);
    return error;
  }

  http.end();
  return response;
}

// JsonDocument retrieveLocation() {
//   StaticJsonDocument<200> doc;
//   JsonDocument json;

//   doc["homeMobileCountryCode"] = "334";
//   doc["homeMobileNetworkCode"] = "020";
//   doc["radioType"] = "lte";
//   doc["carrier"] = "Telcel";
//   doc["considerIp"] = "true";

//   String requestBody;
//   serializeJson(doc, requestBody);
//   //String endpoint = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + String(APIKEY);

//   String response = postData(endpoint, requestBody);
//   Serial.println("response");
//   Serial.println(response);

//   deserializeJson(json, response);

//   return json;
// }

/*
* API routes for handling requests
*/
void handleRegisterRequest() {
  // Getting query variables from the request.
  String homehubName = wm.server->arg("homehub_name");
  String homehubOwner = wm.server->arg("homehub_owner");
  String endpoint = "http://192.168.1.64/axol/homehub.php";

  String data = "type=5"
                "&hh_name="
                + homehubName
                + "&hh_owner="
                + homehubOwner
                + "&mac_address="
                + WiFi.macAddress()
                + "&lat="
                + lat
                + "&lon="
                + lon;

  // String data;
  // serializeJson(doc, data);

  String response = postData(endpoint, data);
  Serial.println(response);

  String page = HTTP_HEAD_START
                + String(HTTP_STYLE)
                + "</head>"
                + "<body>"
                + "<h1>Register HomeHub</h1>"
                + "<h2>"
                + response
                + "</h2>"
                + "<p> Restarting... </p>"
                + HTTP_END;

  wm.server->send(200, "text/html", page);
  delay(2000);
  ESP.restart();
}

/*
* Captive Portal routes for pages
*/
void handleSetupRoute() {
  String page = HTTP_HEAD_START
                + String(HTTP_STYLE)
                + "</head>"
                + "<body>"
                + "<h1>Axol HomeHub Configuration</h1>"
                + "<form action='/wifi' method='get'><button type='submit'>Configure WiFi</button></form><br/>"
                + "<form action='/register' method='get'><button type='submit'>Register</button></form><br/>"
                + "<form action='/info' method='get'><button type='submit'>Info</button></form><br/>"
                + "<form action='/exit' method='get'><button type='submit'>Exit</button></form><br/>"
                + HTTP_END;
  wm.server->send(200, "text/html", page);
}

// void handleRegister() {

//   JsonDocument location = retrieveLocation();
//   double locLatitude = location["location"]["lat"];
//   double locLongitude = location["location"]["lng"];

//   lat = String(locLatitude, 6);
//   lon = String(locLongitude, 6);

//   String page = HTTP_HEAD_START
//                 + String(HTTP_STYLE)
//                 + "<style>"

//                 + "input{"
//                 + "   border: 1px #C1BDBD solid;"
//                 + "   line-height: 2em;"
//                 + "}"
//                 + ".textbox{"
//                 + "   display: flex;"
//                 + "   flex-direction: column;"
//                 + "   align-items: flex-start;"
//                 + "   gap: 0.5rem;"
//                 + "}"
//                 + "form{"
//                 + "   gap: 1.5rem;"
//                 + "}"

//                 + "</style>"
//                 + "</head>"
//                 + "<body>"
//                 + "<h1>Register HomeHub</h1>"
//                 + "<form action='/api/register' method='post'>"

//                 + "<div class='textbox'>"
//                 + "   <span>Name</span>"
//                 + "   <input type='text' name='homehub_name' />"
//                 + "</div>"

//                 + "<div class='textbox'>"
//                 + "   <span>Owner</span>"
//                 + "   <input type='text' name='homehub_owner' />"
//                 + "</div>"
//                 + "<button type='submit'>Register</button>"
//                 + "</form>"
//                 + HTTP_END;

//   wm.server->send(200, "text/html", page);
// }

void onDemandPortal() {
  // Check connection...
  if (!establishWiFiConnection()){
    Serial.println("Not connected, opening Captive Portal...");
    return;
  }

  const int timeout = 300;
  wm.setConfigPortalTimeout(timeout);

  if (!wm.startConfigPortal("AxolOnDemand")) {
    Serial.println("Failed to connect");
  }

  Serial.println("onDemandPortal: Connected!");
}
//c
bool establishWiFiConnection() {
  return wm.autoConnect("Axol");
}

void printNetworkInfo() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());  // Nombre de la red WiFi

  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());  // Dirección IP del ESP32

  Serial.print("Intensidad de señal (RSSI): ");
  Serial.println(WiFi.RSSI());  // Intensidad de la señal en dBm

  Serial.print("Dirección MAC: ");
  Serial.println(WiFi.macAddress());  // Dirección MAC del ESP32

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());  // Dirección IP del gateway

  Serial.print("Máscara de Subred: ");
  Serial.println(WiFi.subnetMask());  // Máscara de subred

  Serial.print("DNS Primario: ");
  Serial.println(WiFi.dnsIP());  // Dirección IP del DNS primario

  Serial.print("MAC del router: ");
  Serial.println(WiFi.BSSIDstr());
}
