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
    if (evt.data === 'money_on') {
      document.getElementById('money_header').style.color = 'green';
    }
    else if (evt.data === 'money_off') {
      document.getElementById('money_header').style.color = 'black';
    }
    else if (evt.data === 'happiness_on') {
      document.getElementById('happiness_header').style.color = 'yellow';
    }
    else if (evt.data === 'happiness_off') {
      document.getElementById('happiness_header').style.color = 'black';
    }
    else if (evt.data === 'innovation_on') {
      document.getElementById('innovation_header').style.color = 'blue';
    }
    else if (evt.data === 'innovation_off') {
      document.getElementById('innovation_header').style.color = 'black';
    }
    else if (evt.data === 'impact_on') {
      document.getElementById('impact_header').style.color = 'red';
    }
    else if (evt.data === 'impact_off') {
      document.getElementById('impact_header').style.color = 'black';
    }
    else if (evt.data === 'knowledge_on') {
      document.getElementById('knowledge_header').style.color = 'orange';
    }
    else if (evt.data === 'knowledge_off') {
      document.getElementById('knowledge_header').style.color = 'black';
    }
    else if (evt.data === 'creativity_on') {
      document.getElementById('creativity_header').style.color = 'purple';
    }
    else if (evt.data === 'creativity_off') {
      document.getElementById('creativity_header').style.color = 'black';
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
function log(msg, log_id) {
  document.getElementById(log_id).innerText += msg + '\n';
  console.log(msg);
}
function submitclick(e) {
  var user_input;
  var msg_send;
  var log_id;
  if (e.id === "money_send") {
  	user_input = document.getElementById('money_entry').value;
	msg_send = "MON&" + user_input;
	log_id = "money_log";
  } else if (e.id === "happiness_send") {
  	user_input = document.getElementById('happiness_entry').value;
	msg_send = "HAP&" + user_input;
	log_id = "happiness_log";
  } else if (e.id === "innovation_send") {
  	user_input = document.getElementById('innovation_entry').value;
	msg_send = "INN&" + user_input;
	log_id = "innovation_log";
  } else if (e.id === "impact_send") {
  	user_input = document.getElementById('impact_entry').value;
	msg_send = "IMP&" + user_input;
	log_id = "impact_log";
  } else if (e.id === "knowledge_send") {
  	user_input = document.getElementById('knowledge_entry').value;
	msg_send = "KNO&" + user_input;
	log_id = "knowledge_log";
  } else if (e.id === "creativity_send") {
  	user_input = document.getElementById('creativity_entry').value;
	msg_send = "CRE&" + user_input;
	log_id = "creativity_log";
  }

  log("Someone wrote: " + user_input, log_id);
  websock.send(msg_send);
}
</script>
</head>
<body onload="javascript:start();">
<h1>Passion Pillar</h1>
<h2>What drives you?</h2>

<!-- Money -->
<div id="money_header"><b>Money</b></div>
<button id="money_on"  type="button" onclick="buttonclick(this);">On</button> 
<button id="money_off" type="button" onclick="buttonclick(this);">Off</button>
<input id="money_entry">
<button id="money_send" onclick="submitclick(this);">Send</button>
<pre id="money_log"></pre>

<!-- Happiness -->
<div id="happiness_header"><b>Happiness</b></div>
<button id="happiness_on"  type="button" onclick="buttonclick(this);">On</button> 
<button id="happiness_off" type="button" onclick="buttonclick(this);">Off</button>
<input id="happiness_entry">
<button id="happiness_send" onclick="submitclick(this);">Send</button>
<pre id="happiness_log"></pre>

<!-- Innovation -->
<div id="innovation_header"><b>Innovation</b></div>
<button id="innovation_on"  type="button" onclick="buttonclick(this);">On</button> 
<button id="innovation_off" type="button" onclick="buttonclick(this);">Off</button>
<input id="innovation_entry">
<button id="innovation_send" onclick="submitclick(this);">Send</button>
<pre id="innovation_log"></pre>

<!-- Impact -->
<div id="impact_header"><b>Impact</b></div>
<button id="impact_on"  type="button" onclick="buttonclick(this);">On</button> 
<button id="impact_off" type="button" onclick="buttonclick(this);">Off</button>
<input id="impact_entry">
<button id="impact_send" onclick="submitclick(this);">Send</button>
<pre id="impact_log"></pre>

<!-- Knowledge -->
<div id="knowledge_header"><b>Knowledge</b></div>
<button id="knowledge_on"  type="button" onclick="buttonclick(this);">On</button> 
<button id="knowledge_off" type="button" onclick="buttonclick(this);">Off</button>
<input id="knowledge_entry">
<button id="knowledge_send" onclick="submitclick(this);">Send</button>
<pre id="knowledge_log"></pre>

<!-- Creativity -->
<div id="creativity_header"><b>Creativity</b></div>
<button id="creativity_on"  type="button" onclick="buttonclick(this);">On</button> 
<button id="creativity_off" type="button" onclick="buttonclick(this);">Off</button>
<input id="creativity_entry">
<button id="creativity_send" onclick="submitclick(this);">Send</button>
<pre id="creativity_log"></pre>

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
				else {
					webSocket.sendTXT(num, "money_off", 9);
				}
			}
			break;
		case WStype_TEXT:
			Serial.printf("[%u] get Text: %s\r\n", num, payload);

			if (strcmp("money_on", (const char *)payload) == 0) {
				writeLED(true, MONEY, responses_per_bar[MONEY]++);
			}
			else if (strcmp("money_off", (const char *)payload) == 0) {
				writeLED(false, MONEY, NUM_LEDS_PER_BAR);
			}
			else if (strncmp("MON", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, MONEY);
			}
			else if (strcmp("happiness_on", (const char *)payload) == 0) {
				writeLED(true, HAPPINESS, responses_per_bar[HAPPINESS]++);
			}
			else if (strcmp("happiness_off", (const char *)payload) == 0) {
				writeLED(false, HAPPINESS, NUM_LEDS_PER_BAR);
			}
			else if (strncmp("HAP", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, HAPPINESS);
			}
			else if (strcmp("innovation_on", (const char *)payload) == 0) {
				writeLED(true, INNOVATION, responses_per_bar[INNOVATION]++);
			}
			else if (strcmp("innovation_off", (const char *)payload) == 0) {
				writeLED(false, INNOVATION, NUM_LEDS_PER_BAR);
			}
			else if (strncmp("INN", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, INNOVATION);
			}
			else if (strcmp("impact_on", (const char *)payload) == 0) {
				writeLED(true, IMPACT, responses_per_bar[IMPACT]++);
			}
			else if (strcmp("impact_off", (const char *)payload) == 0) {
				writeLED(false, IMPACT, NUM_LEDS_PER_BAR);
			}
			else if (strncmp("IMP", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, IMPACT);
			}
			else if (strcmp("knowledge_on", (const char *)payload) == 0) {
				writeLED(true, KNOWLEDGE, responses_per_bar[KNOWLEDGE]++);
			}
			else if (strcmp("knowledge_off", (const char *)payload) == 0) {
				writeLED(false, KNOWLEDGE, NUM_LEDS_PER_BAR);
			}
			else if (strncmp("KNO", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, KNOWLEDGE);
			}
			else if (strcmp("creativity_on", (const char *)payload) == 0) {
				writeLED(true, CREATIVITY, responses_per_bar[CREATIVITY]++);
			}
			else if (strcmp("creativity_off", (const char *)payload) == 0) {
				writeLED(false, CREATIVITY, NUM_LEDS_PER_BAR);
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
