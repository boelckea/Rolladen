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
const int r1UpPin = 13; // D7
const int r1DownPin = 15; // D8 nur mit 600 Ohm Pulldow extern!
const int r2UpPin = 12; // D6
const int r2DownPin = 14; // D5
const int r3UpPin = 4; // D2
const int r3DownPin = 5; // D1

static unsigned long stopTimeR1Up = 0;
static unsigned long stopTimeR1Down = 0;
static int r1offsetIntBack = 0;
static unsigned long stopTimeR2Up = 0;
static unsigned long stopTimeR2Down = 0;
static int r2offsetIntBack = 0;
static unsigned long stopTimeR3Up = 0;
static unsigned long stopTimeR3Down = 0;
static int r3offsetIntBack = 0;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String getServerPage() {
//<div id='r3' style='width: 100px;height: 300px;float:left;background-color: #23c5de;margin:5px;'><div style='height: 200px;background-color: #83c5de;border-bottom-style: solid;border-bottom-width: 3px;'></div></div>
	String serverpage =
			String("Version 5 <a href='/rolladen'>Reload Page</a><br>") +
			"<div id='r3' style='width: 100px; height: 300px; float:left; background-color: #ea347b'></div>" +
			"<script>document.getElementById('r3').addEventListener('click', function(e){document.location='/rolladen?id=3&offset=' + e.offsetY})</script>" +
			"<div id='r1' style='width: 100px; height: 300px; float:left; background-color: #eab47b'></div>" +
			"<script>document.getElementById('r1').addEventListener('click', function(e){document.location='/rolladen?id=1&offset=' + e.offsetY})</script>" +
			"<div id='r2' style='width: 100px; height: 200px; float:left; background-color: #eab42b'></div>" +
			"<script>document.getElementById('r2').addEventListener('click', function(e){document.location='/rolladen?id=2&offset=' + e.offsetY})</script>";
	return serverpage;
}

void setup(void) {
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);
	pinMode(r1UpPin, OUTPUT);
	pinMode(r1DownPin, OUTPUT);
	digitalWrite(r1UpPin, HIGH);
	digitalWrite(r1DownPin, HIGH);
	pinMode(r2UpPin, OUTPUT);
	pinMode(r2DownPin, OUTPUT);
	digitalWrite(r2UpPin, HIGH);
	digitalWrite(r2DownPin, HIGH);
	pinMode(r3UpPin, OUTPUT);
	pinMode(r3DownPin, OUTPUT);
	digitalWrite(r3UpPin, HIGH);
	digitalWrite(r3DownPin, HIGH);

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
			int r1offsetInt = map(offset.toInt(), 0, 300, 0, 25);
			int delta = r1offsetInt - r1offsetIntBack;
			if(delta > 0) {
				stopTimeR1Down = currentTime + 1000L * delta;
			} else {
				stopTimeR1Up = currentTime + 1000L * delta * 1.1 * -1;
			}
			r1offsetIntBack = r1offsetInt;
		}
		if(rolladenNr == "2") {
			int r2offsetInt = map(offset.toInt(), 0, 300, 0, 25);
			int delta = r2offsetInt - r2offsetIntBack;
			if(delta > 0) {
				stopTimeR2Down = currentTime + 1000L * delta;
			} else {
				stopTimeR2Up = currentTime + 1000L * delta * 1.1 * -1;
			}
			r2offsetIntBack = r2offsetInt;
		}
		if(rolladenNr == "3") {
			int r3offsetInt = map(offset.toInt(), 0, 300, 0, 25);
			int delta = r3offsetInt - r3offsetIntBack;
			if(delta > 0) {
				stopTimeR3Down = currentTime + 1000L * delta;
			} else {
				stopTimeR3Up = currentTime + 1000L * delta * 1.1 * -1;
			}
			r3offsetIntBack = r3offsetInt;
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

	if (stopTimeR2Up > currentTime) {
		// R2 up
		digitalWrite(ledPin, LOW);
		Serial.println("R2Down off");
		digitalWrite(r2DownPin, HIGH);
		delay(100);
		Serial.println("R2Up on");
		digitalWrite(r2UpPin, LOW);
	}
	else {
		if (stopTimeR2Down > currentTime) {
			// R2 down
			digitalWrite(ledPin, LOW);
			Serial.println("R2Up off");
			digitalWrite(r2UpPin, HIGH);
			delay(100);
			Serial.println("R2DOwn on");
			digitalWrite(r2DownPin, LOW);
		}
		else {
			digitalWrite(ledPin, HIGH);
			Serial.println("R2Up off");
			digitalWrite(r2UpPin, HIGH);
			Serial.println("R2Down off");
			digitalWrite(r2DownPin, HIGH);
			delay(100);
		}
	}

	if (stopTimeR3Up > currentTime) {
		// R3 up
		digitalWrite(ledPin, LOW);
		Serial.println("R3Down off");
		digitalWrite(r3DownPin, HIGH);
		delay(100);
		Serial.println("R3Up on");
		digitalWrite(r3UpPin, LOW);
	}
	else {
		if (stopTimeR3Down > currentTime) {
			// R3 down
			digitalWrite(ledPin, LOW);
			Serial.println("R3Up off");
			digitalWrite(r3UpPin, HIGH);
			delay(100);
			Serial.println("R3DOwn on");
			digitalWrite(r3DownPin, LOW);
		}
		else {
			digitalWrite(ledPin, HIGH);
			Serial.println("R3Up off");
			digitalWrite(r3UpPin, HIGH);
			Serial.println("R3Down off");
			digitalWrite(r3DownPin, HIGH);
			delay(100);
		}
	}

	httpServer.handleClient();
}
