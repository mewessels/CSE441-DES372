#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <FastLED.h>
FASTLED_USING_NAMESPACE

// GPIO#5 is connected to the DO pin of the WS2812B LEDs
#define DATA_PIN      5
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define NUM_LEDS      150
#define NUM_LEDS_PER_BAR (9+8+8)

#define MILLI_AMPS         10000     // IMPORTANT: set the max milli-Amps of your power supply (10A = 10000mA)

CRGB leds[NUM_LEDS];

enum responses { MONEY, HAPPINESS, INNOVATION, IMPACT, LOVE, CREATIVITY, MAX_RESPONSES }; 

// NUM_LEDS_PER_BAR strings for each option (MONEY, HAPPINESS, etc.) allows user to enter a sentence
static char user_input[MAX_RESPONSES][NUM_LEDS_PER_BAR][128];

static uint8_t responses_per_bar[] = {0,0,0,0,0,0};

static const char ssid[] = "Passion Pillar";
static const char password[] = "";
MDNSResponder mdns;

static void writeLED(bool LEDon, responses selected_bar, uint8_t number_leds_selected);

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Passion Pillar</title>
    <meta charset="UTF-8">
</head>
<body class="main" id="main">
<style>
"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
</style>
<script>
var websock;
function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock open'); };
  websock.onclose = function(evt) { console.log('websock close'); };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    console.log(evt);
    if (evt.data === 'ledon') {
      document.getElementById('ledstatus').style.color = 'green';
    }
    else if (evt.data === 'ledoff') {
      document.getElementById('ledstatus').style.color = 'black';
    }
  };
}
function buttonclick(e) {
  // @TODO: When the user presses this button (makes selection),
  //		redirect the user to a new page which
  //		asks them to submit some thoughts about that
  //		-- or redirects the user ot the same page
  //		but lower down with an anchor
  websock.send(e.id);
}
function log(msg) {
  document.getElementById('money_log').innerText += msg + '\n';
  console.log(msg);
}
function submitclick(e) {
  var user_input = document.getElementById('entry').value;
  var msgSend = "MSG&" + user_input;
  log("Someone wrote: " + user_input);
  websock.send(msgSend);
}
</script>
</head>
<body onload="javascript:start();">
<h1>Passion Pillar</h1>
<h2>What drives you?</h2>
<div id="ledstatus"><b>Money</b></div>
<button id="ledon"  type="button" onclick="buttonclick(this);">On</button> 
<button id="ledoff" type="button" onclick="buttonclick(this);">Off</button>
<input id="entry">
<button id="send" onclick="submitclick(this);">Send</button>
<pre id="money_log"></pre>
</body>
</html>
)rawliteral";

// 0->255 where 255 is max brightness
static const uint8_t brightness = 30;

// Current LED status
bool LEDStatus;

// Commands sent through Web Socket
const char LEDON[] = "ledon";
const char LEDOFF[] = "ledoff";
const char MSG[] = "MSG";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
	Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
	switch (type) {
		case WStype_DISCONNECTED:
			Serial.printf("[%u] Disconnected!\r\n", num);
			break;
		case WStype_CONNECTED:
			{
				IPAddress ip = webSocket.remoteIP(num);
				Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				// Send the current LED status
				if (LEDStatus) {
					webSocket.sendTXT(num, LEDON, strlen(LEDON));
				}
				else {
					webSocket.sendTXT(num, LEDOFF, strlen(LEDOFF));
				}
			}
			break;
		case WStype_TEXT:
			Serial.printf("[%u] get Text: %s\r\n", num, payload);

			if (strcmp(LEDON, (const char *)payload) == 0) {
				writeLED(true, MONEY, responses_per_bar[MONEY]++);
			}
			else if (strcmp(LEDOFF, (const char *)payload) == 0) {
				writeLED(false, MONEY, NUM_LEDS_PER_BAR);
			}
			else if (strncmp(MSG, (const char*)payload, 1) == 0) {
				Serial.println("User wrote: ");
				char *split = strtok((char*)payload, "&");
				split = strtok(NULL, "&");
				Serial.print(split);
				strcpy(user_input[MONEY][responses_per_bar[MONEY]], split);
			}
			else {
				Serial.println("Unknown command");
			}
			// send data to all connected clients
			webSocket.broadcastTXT(payload, length);
			break;
		case WStype_BIN:
			Serial.printf("[%u] get binary length: %u\r\n", num, length);
			hexdump(payload, length);

			// echo data back to browser
			webSocket.sendBIN(num, payload, length);
			break;
		default:
			Serial.printf("Invalid WStype [%d]\r\n", type);
			break;
	}
}

void handleRoot()
{
	server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i<server.args(); i++){
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}

static void writeLED(bool LEDon, responses selected_bar, uint8_t number_leds_selected)
{
	LEDStatus = LEDon;
	CRGB led_fill_color;	

	if (selected_bar == MONEY) {
		led_fill_color = CRGB::Green;	
	} else {
		led_fill_color = CRGB::Black;
	}

	number_leds_selected = ((number_leds_selected >= NUM_LEDS_PER_BAR) ? NUM_LEDS_PER_BAR : number_leds_selected);
	if (LEDon) { // Turn on the LEDs
		for (int led_idx = selected_bar * NUM_LEDS_PER_BAR; led_idx < number_leds_selected; led_idx++) {
			leds[led_idx] = led_fill_color;
		}
	}
	else { // Turn off the LEDs
		for (int led_idx = selected_bar * NUM_LEDS_PER_BAR; led_idx < number_leds_selected; led_idx++) {
			leds[led_idx] = CRGB::Black;
		}
	}
	FastLED.show();
}

void setup()
{
	Serial.begin(115200);
	delay(100);
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);         // for WS2812 (Neopixel)
	FastLED.setDither(false);
	FastLED.setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(brightness);
	FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
	fill_solid(leds, NUM_LEDS, CRGB::Black);
	FastLED.show();

	Serial.println();
	Serial.println();
	Serial.println();

	for (uint8_t t = 4; t > 0; t--) {
		Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
		Serial.flush();
		delay(1000);
	}

	WiFi.softAP(ssid, password);
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (mdns.begin("espWebSock", WiFi.localIP())) {
		Serial.println("MDNS responder started");
		mdns.addService("http", "tcp", 80);
		mdns.addService("ws", "tcp", 81);
	}
	else {
		Serial.println("MDNS.begin failed");
	}
	server.on("/", handleRoot);
	server.onNotFound(handleNotFound);

	server.begin();

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
}

void loop()
{
	webSocket.loop();
	server.handleClient();
}