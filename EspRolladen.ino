/*
 To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
 http://192.168.11.48/update
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

const char* ssid = "ROSID";
const char* password = "ROSI29ROSI!";
const char* host = "EspRolladen";

const int ledPin = 2;
const int r1UpPin = 12; // D6
const int r1DownPin = 14; // D5
const int r1DownPin = 4; // D2
const int r1DownPin = 5; // D1
const int r1DownPin = 13; // D7
const int r1DownPin = 10; // 3D3?

static unsigned long stopTimeR1Up = 0; //currentTime + 2;
static unsigned long stopTimeR1Down = 0; // currentTime + 4;
static int r1offsetIntBack = 0;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String getServerPage() {
	String serverpage =
			String("Version 15<br>") +
					"<div id='r1' style='width: 100px; height: 300px; background-color: #eab47b'></div>" +
					"<script>document.getElementById('r1').addEventListener('click', function(e){document.location='/rolladen?id=1&offset=' + e.offsetY})</script>";
	return serverpage;
}

void setup(void) {
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);
	pinMode(r1UpPin, OUTPUT);
	pinMode(r1DownPin, OUTPUT);
	digitalWrite(r1UpPin, HIGH);
	digitalWrite(r1DownPin, HIGH);

	Serial.begin(115200);
	Serial.println();
	Serial.println("Booting Sketch...");
	WiFi.mode(WIFI_AP_STA);
	WiFi.begin(ssid, password);

	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		WiFi.begin(ssid, password);
		Serial.println("WiFi failed, retrying.");
	}

	MDNS.begin(host);

	httpUpdater.setup(&httpServer);

	httpServer.on("/rolladen", []() {
		String rolladenNr = httpServer.arg("id");
		String offset = httpServer.arg("offset");
		Serial.println(String("Rolladen") + rolladenNr + " Offset " + offset);

		unsigned long currentTime = millis();
		if(rolladenNr == "1") {
			int r1offsetInt = map(offset.toInt(), 0, 300, 0, 20);
			int delta = r1offsetInt - r1offsetIntBack;
			if(delta > 0) {
				stopTimeR1Down = currentTime + 1000L * delta;
			} else {
				stopTimeR1Up = currentTime + 1000L * delta * -1;
			}
			r1offsetIntBack = r1offsetInt;
		}

		httpServer.sendHeader("Connection", "close");
		String page = getServerPage();
		httpServer.send(200, "text/html", page);
	});

	httpServer.begin();

	MDNS.addService("http", "tcp", 80);
	Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}

void loop(void) {
	unsigned long currentTime = millis();
	if (stopTimeR1Up > currentTime) {
		// R1 up
		digitalWrite(ledPin, LOW);
		Serial.println("R1Down off");
		digitalWrite(r1DownPin, HIGH);
		delay(100);
		Serial.println("R1Up on");
		digitalWrite(r1UpPin, LOW);
	}
	else {
		if (stopTimeR1Down > currentTime) {
			// R1 down
			digitalWrite(ledPin, LOW);
			Serial.println("R1Up off");
			digitalWrite(r1UpPin, HIGH);
			delay(100);
			Serial.println("R1DOwn on");
			digitalWrite(r1DownPin, LOW);
		}
		else {
			digitalWrite(ledPin, HIGH);
			Serial.println("R1Up off");
			digitalWrite(r1UpPin, HIGH);
			Serial.println("R1Down off");
			digitalWrite(r1DownPin, HIGH);
			delay(100);
		}
	}

	httpServer.handleClient();
}
