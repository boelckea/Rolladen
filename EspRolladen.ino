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

const int upPin[3] = {13 /*D7*/, 
		      12 /*D6*/, 
		      5 /*D1*/};
const int downPin[3] = {15 /*D8 nur mit 600 Ohm Pulldow extern!*/, 
			14 /*D5*/, 
			4 /*D2*/};

static unsigned long stopTimeUp[3] = {0,0,0};
static unsigned long stopTimeDown[3] = {0,0,0};
static int offsetIntBack[3] = {0,0,0};

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String getServerPage() {
	String serverpage =
			String("<!DOCTYPE html><head><meta name='viewport' content='width = device-width, initial-scale = 1.0'><title>RosiRolläden</title></head><body>") +
			"<style> .win{width: 100px; float:left; background-color: #23c5de; margin:5px;} .shutter{background-color: #83c5de; border-bottom-style: solid;border-bottom-width: 3px;}</style>" +
			"<a href='/rolladen'>Reload Page</a><br>" +
			"<div id='r3' class='win' style='height: 300px;'><div class='shutter' style='height: " + String(offsetIntBack[2]) +"px;'></div></div>" +
			"<script>document.getElementById('r3').addEventListener('click', function(e){document.location='/rolladen?id=r3&offset=' + e.offsetY})</script>" +
			"<div id='r1' class='win' style='height: 300px;'><div class='shutter' style='height: " + String(offsetIntBack[0]) +"px;'></div></div>" +
			"<script>document.getElementById('r1').addEventListener('click', function(e){document.location='/rolladen?id=r1&offset=' + e.offsetY})</script>" +
			"<div id='r2' class='win' style='height: 200px;'><div class='shutter' style='height: " + String(offsetIntBack[1]) +"px;'></div></div>" +
			"<script>document.getElementById('r2').addEventListener('click', function(e){document.location='/rolladen?id=r2&offset=' + e.offsetY})</script>" +
			"<br style='clear:both'> Version 7"
			"</body>";
	return serverpage;
}

void setup(void) {
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);

	for (int rolIndex = 0; rolIndex < 3; rolIndex++) {
		pinMode(upPin[rolIndex], OUTPUT);
		pinMode(downPin[rolIndex], OUTPUT);
		digitalWrite(upPin[rolIndex], HIGH);
		digitalWrite(downPin[rolIndex], HIGH);
	}

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

		int rolladenNrIndex = rolladenNr.substring(1, 1).toInt() - 1;
		unsigned long currentTime = millis();
		int offsetInt = offset.toInt();
		int delta = offsetInt - offsetIntBack[rolladenNrIndex];
		if(delta > 0) {
			stopTimeDown[rolladenNrIndex] = currentTime + 1000L * map(delta, 0, 300, 0, 25) * 1.1;
		} else {
			stopTimeUp[rolladenNrIndex] = currentTime + 1000L * map(delta, 0, 300, 0, 25) * 1.1 * -1 * 1.1;
		}
		offsetIntBack[rolladenNrIndex] = offsetInt;

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
	for (int rolIndex = 0; rolIndex < 3; rolIndex++) {
		if (stopTimeUp[rolIndex] > currentTime) {
			// Shutter up
			digitalWrite(ledPin, LOW);
			Serial.print(rolIndex);
			Serial.println(" down off");
			digitalWrite(downPin[rolIndex], HIGH);
			delay(100);
			Serial.print(rolIndex);
			Serial.println(" up on");
			digitalWrite(upPin[rolIndex], LOW);
		}
		else {
			if (stopTimeDown[rolIndex] > currentTime) {
				// Shutter down
				digitalWrite(ledPin, LOW);
				Serial.print(rolIndex);
				Serial.println(" up off");
				digitalWrite(upPin[rolIndex], HIGH);
				delay(100);
				Serial.print(rolIndex);
				Serial.println(" dOwn on");
				digitalWrite(downPin[rolIndex], LOW);
			}
			else {
				digitalWrite(ledPin, HIGH);
				Serial.print(rolIndex);
				Serial.println(" up off");
				digitalWrite(upPin[rolIndex], HIGH);
				Serial.print(rolIndex);
				Serial.println(" down off");
				digitalWrite(downPin[rolIndex], HIGH);
				delay(100);
			}
		}
	}

	httpServer.handleClient();
}
