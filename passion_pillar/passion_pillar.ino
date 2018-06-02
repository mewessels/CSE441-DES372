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

enum responses { MONEY, INNOVATION, KNOWLEDGE, CREATIVITY, IMPACT, HAPPINESS, MAX_RESPONSES };

// NUM_LEDS_PER_BAR strings for each option (MONEY, HAPPINESS, etc.) allows user to enter a sentence
static char user_input[MAX_RESPONSES][NUM_LEDS_PER_BAR][128];

static uint8_t responses_per_bar[] = {0,0,0,0,0,0};

static const char ssid[] = "Passion Pillar";
static const char password[] = "";
MDNSResponder mdns;

static void writeLED(uint8_t selected_bar, float number_responses);

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="utf-8">
    <title>UW Community Pillar</title>
    <style>
    * {
      font-family: Helvetica;
    }

    body {
      background-color: #F1F2F1;
      height: 100%;
    }

    .container {
      text-align: center;
    }

    #title {
      text-align: center;
      letter-spacing: .20em;
      font-weight: 600;
      color: #9A9B9A;
    }

    h1 {
      color: #494A49;
      text-align: center;
    }

    .column {
      float: left;
      width: 50%;
      margin-top: 2%;
      margin-bottom: 2%;
    }

    input[type="radio"] {
      position:fixed;
      opacity:0;
    }

    .button {
      font-size: 2em;
      font-weight: bold;
      background-color: white;
      border-style: solid;
      border-color: white;
      border-width: thick;
      border-radius: 50px;
      padding-top: 4%;
      padding-bottom: 4%;
      margin-bottom: 5%;
      margin-left: 18%;
      margin-right: 18%;
    }

    .money {color: #7BD400;}
    .innovation {color: #17CEF8;}
    .knowledge {color: #F6A602;}
    .creativity {color: #A896FF;}
    .impact {color: #F18674;}
    .happiness {color: #FFD43B;}
    :checked + .money {border-color: #7BD400;}
    :checked + .innovation {border-color: #17CEF8;}
    :checked + .knowledge {border-color: #F6A602;}
    :checked + .creativity {border-color: #A896FF;}
    :checked + .impact {border-color: #F18674;}
    :checked + .happiness {border-color: #FFD43B;}

    textarea {
      resize: none;
      width: 60%;
      font-size: 1.2em;
      font-weight: lighter;
      border-color: #D8D7D7;
      border-radius: 5px;
      padding: 1.5%;
    }

    #entry_send {
      color: #999A99;
      font-size: 1em;
      font-weight: bold;
      background-color: white;
      border-color: white;
      border-style: solid;
      border-radius: 50px;
      padding-top: 1%;
      padding-bottom: 1%;
      padding-left: 3%;
      padding-right: 3%;
      margin-top: 2%;
      position: relative;
      left: 25%;
    }

    #entry_send:hover {
      background-color: #D8D7D7;
      border-color: #D8D7D7;
    }

    #switch_page {
      position: absolute;
      top: 0;
      right: 0;
      width: 25px;
      height: 25px;
    }
    
    #reset_switch {
      position: absolute;
      top: 0;
      left: 0;
      width: 25px;
      height: 25px;
    }

    #responses {
      display: none;
    }

    .message__bubble {
      margin: auto;
      width: 75%;
      min-width: 300px;
      border-radius: 30px;
      background-color: white;
      padding: 20px;
      margin-bottom: 10px;
    }

    .message__category {
      text-transform: uppercase;
      font-size: .75em;
      font-weight: 600;
      letter-spacing: .2em
    }

    .message__text {
      color: #494A49;
      font-size: 2em;
      font-weight: 100;
      word-wrap:break-word;
    }
    </style>
    <script>
    function log(msg, log_id) {
      // document.getElementById("log").innerText += log_id + '\n' + msg + '\n';
      var message__bubble = document.createElement("div");
      message__bubble.className = "message__bubble";
      var category = document.createElement("div");
      category.className = log_id + " message__category";
      category.innerHTML = log_id;
      message__bubble.appendChild(category);
      var message = document.createElement("div");
      message.className = "message__text";
      message.innerHTML = msg;
      message__bubble.appendChild(message);
      var log = document.getElementById("log");
      log.appendChild(message__bubble);
    }
    function parse_new_message(msg_header, msg) {
      switch (msg_header) {
        case 'MON':
        log(msg, "money");
        break;
        case 'HAP':
        log(msg, "happiness");
        break;
        case 'INN':
        log(msg, "innovation");
        break;
        case 'IMP':
        log(msg, "impact");
        break;
        case 'KNO':
        log(msg, "knowledge");
        break;
        case 'CRE':
        log(msg, "creativity");
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
        alert("Please select a category before submitting.");
        break;
      }
      // parse_new_message(msg_send.substring(0, 3), msg_send.substring(4, msg_send.length));
      websock.send(msg_send);
      websock.send(selected_button);
      document.getElementById("entry_form").reset();
    }

    function switchPages() {
      console.log('switch');
      var prompt = document.getElementById("prompt");
      var responses = document.getElementById("responses");
      if (prompt.style.display === "none") {
        prompt.style.display = "block";
        responses.style.display = "none";
      } else {
        prompt.style.display = "none";
        responses.style.display = "block";
      }
    }

    function resetLEDs() {
      console.log('reset LEDs');
      websock.send("reset");
    }
    </script>
  </head>
  <body onload="javascript:start();">
    <p id="title">UW COMMUNITY PILLAR</p>
    <span id="switch_page" onclick="switchPages()"></span>
    <span id="reset_switch" onclick="resetLEDs()"></span>
    <div id="prompt">
      <div class="container" id="header">
        <h1>What drives you most as a member of the UW community?</h1>
      </div>
      <div class="container" id="form">
        <form id="entry_form">
          <div class="column">
            <label class="test">
              <input type="radio" name="category" id="money" onclick="buttonclick(this);"/>
              <div class="button money">Money</div>
            </label>
            <label class="test">
              <input type="radio" name="category" id="innovation" onclick="buttonclick(this);"/>
              <div class="button innovation">Innovation</div>
            </label>
            <label class="test">
              <input type="radio" name="category" id="knowledge" onclick="buttonclick(this);"/>
              <div class="button knowledge">Knowledge</div>
            </label>
          </div>
          <div class="column">
            <label class="test">
              <input type="radio" name="category" id="creativity" onclick="buttonclick(this);"/>
              <div class="button creativity">Creativity</div>
            </label>
            <label class="test">
              <input type="radio" name="category" id="impact" onclick="buttonclick(this);"/>
              <div class="button impact">Impact</div>
            </label>
            <label class="test">
              <input type="radio" name="category" id="happiness" onclick="buttonclick(this);"/>
              <div class="button happiness">Happiness</div>
            </label>
          </div>
          <textarea id="entry_field" rows="5" cols="50" placeholder="Tell us why!"></textarea>
          <div id="submit">
            <button id="entry_send" onclick="submitclick(this);" type='button' value="Submit">Submit</button>
          </div>
        </form>
      </div>
    </div>
    <div id="responses">
      <h1>Here's what others have said:</h1>
      <div id="log">
        <!--<div class="message__bubble">
          <div class="money message__category">money</div>
          <div class="message__text">I like money because it lets me hang out with friends.</div>
        </div>
        <!--<div class="message__bubble">
          <div class="happiness message__category">happiness</div>
          <div class="message__text">Smile everyday!</div>-->
        </div>
        <!--<div class="message__bubble">
          <div class="knowledge message__category">knowledge</div>
          <div class="message__text">Knowledge = Power</div>-->
        </div>
        <!--<div class="message__bubble">
          <div class="impact message__category">impact</div>
          <div class="message__text">I want to make an impact with my life to give back to those who had an impact on me.</div>-->
        </div>
        <!--<div class="message__bubble">
          <div class="creativity message__category">creativity</div>
          <div class="message__text">Making things is awesome!</div>-->
        </div>
        <!--<div class="message__bubble">
          <div class="innovation message__category">innovation</div>
          <div class="message__text">We should always be innovating.</div>-->
        </div>
      </div>
    </div>
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

float total_responses = 0;

static void update_LED_strip(void) {
  CRGB led_fill_color = CRGB::Black;
  
  // reset the LED strip
  fill_solid(leds, NUM_LEDS, led_fill_color);
  FastLED.show();
  
  for (int response = MONEY; response < MAX_RESPONSES; response++) {
    writeLED(response, responses_per_bar[response]);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  bool need_to_update_strip = false;
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
				responses_per_bar[MONEY]++;
        need_to_update_strip = true;
			}
			else if (strncmp("MON", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, MONEY);
			}
			else if (strcmp("happiness", (const char *)payload) == 0) {
				responses_per_bar[HAPPINESS]++;
        need_to_update_strip = true;
			}
			else if (strncmp("HAP", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, HAPPINESS);
			}
			else if (strcmp("innovation", (const char *)payload) == 0) {
				responses_per_bar[INNOVATION]++;
        need_to_update_strip = true;
			}
			else if (strncmp("INN", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, INNOVATION);
			}
			else if (strcmp("impact", (const char *)payload) == 0) {
				responses_per_bar[IMPACT]++;
        need_to_update_strip = true;
			}
			else if (strncmp("IMP", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, IMPACT);
			}
			else if (strcmp("knowledge", (const char *)payload) == 0) {
				responses_per_bar[KNOWLEDGE]++;
        need_to_update_strip = true;
			}
			else if (strncmp("KNO", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, KNOWLEDGE);
			}
			else if (strcmp("creativity", (const char *)payload) == 0) {
				responses_per_bar[CREATIVITY]++;
        need_to_update_strip = true;
			}
			else if (strncmp("CRE", (const char*)payload, 3) == 0) {
				process_payload((char*)payload, CREATIVITY);
			}
      else if (strncmp("reset", (const char *)payload, 5) == 0) {
        update_LED_strip();
      }
			else {
				Serial.println("Unknown command");
			}

      if (need_to_update_strip) {
        total_responses++;
        update_LED_strip();
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

static void writeLED(uint8_t selected_bar, float number_responses)
{
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

  uint8_t number_leds_to_light = round((number_responses/total_responses) * NUM_LEDS_PER_BAR);

  Serial.print("number_responses_for_category = ");
  Serial.println(number_responses);
  Serial.print("total_responses = ");
  Serial.println(total_responses);

	Serial.print("Selected bar (0 = Money, 1 = Happy, etc.): ");
	Serial.println(selected_bar);

	number_leds_to_light = ((number_leds_to_light >= NUM_LEDS_PER_BAR) ? NUM_LEDS_PER_BAR : number_leds_to_light);
  Serial.print("number_leds_to_light = ");
	Serial.println(number_leds_to_light);

	uint8_t starting_idx = selected_bar * NUM_LEDS_PER_BAR;
	for (uint8_t led_idx = starting_idx; led_idx < (number_leds_to_light + starting_idx); led_idx++) {
			Serial.print("led_idx = ");
			Serial.println(led_idx);
			leds[led_idx] = led_fill_color;
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
