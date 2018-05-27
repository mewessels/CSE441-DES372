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

enum responses { MONEY, HAPPINESS, INNOVATION, IMPACT, KNOWLEDGE, CREATIVITY, MAX_RESPONSES }; 

// NUM_LEDS_PER_BAR strings for each option (MONEY, HAPPINESS, etc.) allows user to enter a sentence
static char user_input[MAX_RESPONSES][NUM_LEDS_PER_BAR][128];

static uint8_t responses_per_bar[] = {1,1,1,1,1,1};

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

function log(msg, log_id) {
  document.getElementById(log_id).innerText += msg + '\n';
  console.log(msg);
}

function parse_new_message(msg_header, msg) {
  switch (msg_header) {
     case 'MON':
       log(msg, "money_log");
       break;
     case 'HAP':
       log(msg, "happiness_log");
       break;
     case 'INN':
       log(msg, "innovation_log");
       break;
     case 'IMP':
       log(msg, "impact_log");
       break;
     case 'KNO':
       log(msg, "knowledge_log");
       break;
     case 'CRE':
       log(msg, "creativity_log");
       break;
  }
}

var websock;
function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock open'); };
  websock.onclose = function(evt) { console.log('websock close'); };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    console.log(evt);
    parse_new_message(evt.data.substring(0, 3), evt.data.substring(3, evt.data.length));
  };
}
var selected_button;

function buttonclick(e) {
  selected_button = e.id;
}

function submitclick(e) {
  var user_input;
  var msg_send;
  var log_id;
  switch (selected_button) {
    case 'money':
      user_input = document.getElementById('entry_field').value;
      msg_send = "MON&" + user_input;
      log_id = "money_log";
      break;
    case 'happiness':
      user_input = document.getElementById('entry_field').value;
      msg_send = "HAP&" + user_input;
      log_id = "happiness_log";
      break;
    case 'innovation':
      user_input = document.getElementById('entry_field').value;
      msg_send = "INN&" + user_input;
      log_id = "innovation_log";
      break;
    case 'impact':
      user_input = document.getElementById('entry_field').value;
      msg_send = "IMP&" + user_input;
      log_id = "impact_log";
      break;
    case 'knowledge':
      user_input = document.getElementById('entry_field').value;
      msg_send = "KNO&" + user_input;
      log_id = "knowledge_log";
      break;
    case 'creativity':
      user_input = document.getElementById('entry_field').value;
      msg_send = "CRE&" + user_input;
      log_id = "creativity_log";
      break;
    default:
      alert("Please select an answer before submitting");
      break;
  }
  websock.send(msg_send);
  websock.send(selected_button);
}
</script>
</head>
<body onload="javascript:start();">
<h1>Passion Pillar</h1>
<h2>What drives you?</h2>

<!-- Money -->
<button id="money"  type="button" onclick="buttonclick(this);">Money</button> 
<pre id="money_log"></pre>

<!-- Happiness -->
<button id="happiness"  type="button" onclick="buttonclick(this);">Happiness</button> 
<pre id="happiness_log"></pre>

<!-- Innovation -->
<button id="innovation"  type="button" onclick="buttonclick(this);">Innovation</button> 
<pre id="innovation_log"></pre>

<!-- Impact -->
<button id="impact"  type="button" onclick="buttonclick(this);">Impact</button> 
<pre id="impact_log"></pre>

<!-- Knowledge -->
<button id="knowledge"  type="button" onclick="buttonclick(this);">Knowledge</button> 
<pre id="knowledge_log"></pre>

<!-- Creativity -->
<button id="creativity"  type="button" onclick="buttonclick(this);">Creativity</button> 
<pre id="creativity_log"></pre>

<input id="entry_field">
<button id="entry_send" onclick="submitclick(this);">Send</button>

</body>
</html>
)rawliteral";

// 0->255 where 255 is max brightness
static const uint8_t brightness = 30;

// Current LED status
bool LEDStatus;

void process_payload(char* payload, responses selection) {
	Serial.println("User wrote: ");
	char *split = strtok(payload, "&");
	split = strtok(NULL, "&");
	Serial.print(split);
	strcpy(user_input[selection][responses_per_bar[selection]], split);
}

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
					webSocket.sendTXT(num, "money_on", 8);
				}
			}
			break;
		case WStype_TEXT:
			Serial.printf("[%u] get Text: %s\r\n", num, payload);

			if (strcmp("money", (const char *)payload) == 0) {
				writeLED(true, MONEY, responses_per_bar[MONEY]++);
			}
			else if (strncmp("MON", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, MONEY);
			}
			else if (strcmp("happiness", (const char *)payload) == 0) {
				writeLED(true, HAPPINESS, responses_per_bar[HAPPINESS]++);
			}
			else if (strncmp("HAP", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, HAPPINESS);
			}
			else if (strcmp("innovation", (const char *)payload) == 0) {
				writeLED(true, INNOVATION, responses_per_bar[INNOVATION]++);
			}
			else if (strncmp("INN", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, INNOVATION);
			}
			else if (strcmp("impact", (const char *)payload) == 0) {
				writeLED(true, IMPACT, responses_per_bar[IMPACT]++);
			}
			else if (strncmp("IMP", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, IMPACT);
			}
			else if (strcmp("knowledge", (const char *)payload) == 0) {
				writeLED(true, KNOWLEDGE, responses_per_bar[KNOWLEDGE]++);
			}
			else if (strncmp("KNO", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, KNOWLEDGE);
			}
			else if (strcmp("creativity", (const char *)payload) == 0) {
				writeLED(true, CREATIVITY, responses_per_bar[CREATIVITY]++);
			}
			else if (strncmp("CRE", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, CREATIVITY);
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
	CRGB led_fill_color = CRGB::Black;	

	if (selected_bar == MONEY) {
		led_fill_color = CRGB::Green;	
	} else if (selected_bar == HAPPINESS) {
		led_fill_color = CRGB::Yellow;
	} else if (selected_bar == INNOVATION) {
		led_fill_color = CRGB::Blue;	
	} else if (selected_bar == IMPACT) {
		led_fill_color = CRGB::Red;	
	} else if (selected_bar == KNOWLEDGE) {
		led_fill_color = CRGB::Orange;
	}  else if (selected_bar == CREATIVITY) {
		led_fill_color = CRGB::Purple;
	}

	Serial.print("Selected bar (0 = Money, 1 = Happy, etc.): ");
	Serial.println(selected_bar);

	number_leds_selected = ((number_leds_selected >= NUM_LEDS_PER_BAR) ? NUM_LEDS_PER_BAR : number_leds_selected);
        Serial.print("number_leds_selected = ");
	Serial.println(number_leds_selected);

	uint8_t starting_idx = selected_bar * NUM_LEDS_PER_BAR;
	if (LEDon) { // Turn on the LEDs
		for (uint8_t led_idx = starting_idx; led_idx < (number_leds_selected + starting_idx); led_idx++) {
			Serial.print("led_idx = ");
			Serial.println(led_idx);
			leds[led_idx] = led_fill_color;
		}
	}
	else { // Turn off the LEDs
		for (uint8_t led_idx = starting_idx; led_idx < (number_leds_selected + starting_idx); led_idx++) {
			Serial.print("led_idx = ");
			Serial.println(led_idx);
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
