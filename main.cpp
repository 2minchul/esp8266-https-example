// An arduino example code for http and https request using ESP8266 core
//
// Based on the example below:
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/BearSSL_CertStore/BearSSL_CertStore.ino
// Feb 2020 by Lee Minchul

#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <ctime>
#include <FS.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "Tenda_87DAF0"
#define STAPSK  "1e345678"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

// A single, global CertStore which can be used by all
// connections.  Needs to stay live the entire time any of
// the WiFiClientBearSSLs are present.
BearSSL::CertStore certStore;

// Set time via NTP, as required for x.509 validation
void setClock() {
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    while (time(nullptr) < 8 * 3600 * 2) {
        delay(500);
    }
}


std::unique_ptr<BearSSL::WiFiClientSecure> makeHttpsClient() {
    auto *bear = new BearSSL::WiFiClientSecure();
    bear->setCertStore(&certStore);
    return std::unique_ptr<BearSSL::WiFiClientSecure>(bear);
}

std::unique_ptr<WiFiClient> makeHttpClient() {
    return std::unique_ptr<WiFiClient>();
}


void fetchHttpsURL(const String &url) {
    auto client = makeHttpsClient(); // make sure to define the `client` before making `http`
    HTTPClient http;;

    http.begin(*client, url);

    int httpCode = http.GET();

    Serial.print("Code: ");
    Serial.println(httpCode);

    Serial.println(http.getString());

    http.end();
    Serial.printf("\n-------\n");
}

void fetchHttpURL(const String &url) {
    WiFiClient client; // make sure to define the `client` before making `http`
    HTTPClient http;

    http.begin(client, url);

    int httpCode = http.GET();

    Serial.print("Code: ");
    Serial.println(httpCode);

    Serial.println(http.getString());

    http.end();
    Serial.printf("\n-------\n");
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    SPIFFS.begin();
    // If using a SD card or LittleFS, call the appropriate ::begin instead

    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    setClock();

    Serial.println("Read CertStores from SPIFFS ...");
    // Before running, you must upload the generated .AR file to SPIFFS or SD.
    // See: https://www.instructables.com/id/Using-ESP8266-SPIFFS/
    // You do not need to generate the ".IDX" file listed below, it is generated automatically
    int numCerts = certStore.initCertStore(SPIFFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    Serial.printf("Number of CA certs read: %d\n", numCerts);
    if (numCerts == 0) {
        Serial.printf(
                "Error: No certs found. Did you run certs-from-mozilla.py and upload the SPIFFS directory before running?\n");
        return; // Can't connect to anything w/o certs!
    }

    Serial.printf("Attempting to fetch https://api.ipify.org/?format=json ...\n");
    fetchHttpsURL("https://api.ipify.org/?format=json");
}

void loop() {
    Serial.printf("\nPlease enter a website url (https://example.com) to connect to: ");
    String site;
    do {
        site = Serial.readString();
    } while (site == "");
    // Strip newline if present
    site.replace(String("\r"), emptyString);
    site.replace(String("\n"), emptyString);
    Serial.println(site);

    if (site.startsWith("https://")) {
        Serial.println("https");
        fetchHttpsURL(site);
    } else {
        Serial.println("http");
        fetchHttpURL(site);
    }

    delay(1000);
}
