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
      <style>


      a {
          text-decoration: none;
      }
      .full-width-text {
          width: 100%;
          text-align: center;
          font-family: avenir;
      }

      .title {
          padding-top: 5em;
          padding-bottom: 5em;
      }

      .container {
          /*display: flex;
          flex-wrap: wrap;*/

      }

      div.question {
          display:inline-block;
          padding-top: 5em;
          max-width: 400px;
          padding-bottom: 2em;
      }

      .item {
          /*flex-grow: 1;
          flex-basis: 500px;*/

      }

      .category {
        margin: 2em;
      }

      .categoryButton {
        border-style: solid;
        border-color: white;
        border-width: thick;
        text-align: center;
        font-family: avenir;
      }
      p.percentage {
          margin-top: -4px;
          margin-left: 7px;
          display: inline-block;
          color: white;
          float:left;
      }
      h1 {
          margin: auto;
          font-size: 3.5em;
          color: white;
      }

      h3 {
          font-size: 2.2em;
          color: white;
          margin-bottom: .25em;
      }
      form {
          width: 80%;
      }
      input[type=text] {
          font-family: futura;
          color: white;
          font-size: 24px;
          border: none;
          background-color: inherit;
          border-bottom: 6px solid white;
          width: 80%;
          float:left;
      }
      input[type=text]:focus {
          outline:none;
      }
      #submit {
          font-size: .9em;
          float:left;
          color: white;

          border: none;
          background-color: inherit;
          text-decoration: none;
          text-align: center;
          padding-left:2em;
          padding-top: 3px;
          opacity: 1;
      }

      #submit:hover {
          cursor: pointer;
          opacity: .5;
          animation: 2s ease;
          color: black;
      }
      #submit:focus {
          cursor: pointer;
          outline:0;
      }

      .white-button {
          font-size: .9em;
          float:left;
          color: white;

          border: none;
          background-color: inherit;
          text-decoration: none;
          text-align: center;
          padding-top: 3px;
          opacity: 1;
          border-style: solid;
          border-color: white;
          border-width: thick;
          font-family: futura;
      }

      .white-button:hover {
          cursor: pointer;
          opacity: .5;
          animation: 2s ease;
          color: black;
      }
      .white-button:focus {
          cursor: pointer;
          outline:0;
      }

      @keyframes buttonHover {
          0% {color: white}
          25% {}
      }
      .money-bar {
          display:inline-block;
          float:left;
          width: 0%;
          height: 1em;
          background-color: #7ED321;
          transition: width 2s;
      }
      .happiness-bar {
          display:inline-block;
          float:left;
          width: 0%;
          height: 1em;
          background-color: #F8E71C;
          transition: width 2s;
      }
      .innovation-bar {
          display:inline-block;
          float:left;
          width: 0%;
          height: 1em;
          background-color: #2DCFF6;
          transition: width 2s;
      }
      .impact-bar {
          display:inline-block;
          float:left;
          width: 0%;
          height: 1em;
          background-color: #D0021B;
          transition: width 2s;
      }
      .knowledge-bar {
          display:inline-block;
          float:left;
          width: 0%;
          height: 1em;
          background-color: #FF7F11;
          transition: width 2s;
      }
      .creativity-bar {
          display:inline-block;
          float:left;
          width: 0%;
          height: 1em;
          background-color: #A999FF;
          transition: width 2s;
      }
      .main {
          background-color: black;
      }

      .innovation {
          background-color: #49DAFD;
      }
      .question-container {
          margin-left: 15%;
          width: 80%;
          top:0;
          left: 0;
          position: absolute;
          visibility: hidden;
          font-family: futura;
      }

      .column {
        float: left;
      width: 50%;
      }

      </style>
      <script>
      var websock;
      var totalResponses;
      var money;
      var happiness;
      var innovation;
      var impact;
      var knowledge;
      var creativity;

      var background;

      var moneyBar;
      var moneyPercentage;

      var happinessBar;
      var happinessPercentage;

      var impactBar;
      var impactPercentage;

      var knowledgeBar;
      var knowledgePercentage;

      var creativityBar;
      var creativityPercentage;

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
        totalResponses = totalResponses + 1;
        if (e.id === "money_send") {
        	user_input = document.getElementById('money_entry').value;
      	msg_send = "MON&" + user_input;
        log_id = "money_log";
        money.push(user_input); // from SubmitValues()
        window.alert(money); // from SubmitValues()
        var moneyQuestion = document.getElementById("money-question"); // from SubmitValues()
        moneyQuestion.style.visibility = "hidden"; // from SubmitValues()
        } else if (e.id === "happiness_send") {
        	user_input = document.getElementById('happiness_entry').value;
      	msg_send = "HAP&" + user_input;
      	log_id = "happiness_log";
        happiness.push(user_input); // from SubmitValues()
        window.alert(happiness); // from SubmitValues()
        var happinessQuestion = document.getElementById("happiness-question"); // from SubmitValues()
        happinessQuestion.style.visibility = "hidden"; // from SubmitValues()
        } else if (e.id === "innovation_send") {
        	user_input = document.getElementById('innovation_entry').value;
      	msg_send = "INN&" + user_input;
      	log_id = "innovation_log";
        innovation.push(user_input); // from SubmitValues()
        window.alert(innovation); // from SubmitValues()
        var innovationQuestion = document.getElementById("innovation-question"); // from SubmitValues()
        innovationQuestion.style.visibility = "hidden"; // from SubmitValues()
        } else if (e.id === "impact_send") {
        	user_input = document.getElementById('impact_entry').value;
      	msg_send = "IMP&" + user_input;
      	log_id = "impact_log";
        impact.push(user_input); // from SubmitValues()
        window.alert(impact); // from SubmitValues()
        var impactQuestion = document.getElementById("impact-question"); // from SubmitValues()
        impactQuestion.style.visibility = "hidden"; // from SubmitValues()
        } else if (e.id === "knowledge_send") {
        	user_input = document.getElementById('knowledge_entry').value;
      	msg_send = "KNO&" + user_input;
      	log_id = "knowledge_log";
        knowledge.push(user_input); // from SubmitValues()
        window.alert(knowledge); // from SubmitValues()
        var knowledgeQuestion = document.getElementById("knowledge-question"); // from SubmitValues()
        knowledgeQuestion.style.visibility = "hidden"; // from SubmitValues()
        } else if (e.id === "creativity_send") {
        	user_input = document.getElementById('creativity_entry').value;
      	msg_send = "CRE&" + user_input;
      	log_id = "creativity_log";
        creativity.push(user_input); // from SubmitValues()
        window.alert(creativity); // from SubmitValues()
        var creativityQuestion = document.getElementById("creativity-question"); // from SubmitValues()
        creativityQuestion.style.visibility = "hidden"; // from SubmitValues()
        }
        log("Someone wrote: " + user_input, log_id);
        backToHome();
        websock.send(msg_send);
      }

      window.onload = function() {
          init();
          reloadBars();
      }

      function reloadBars() {
          moneyBar.style.width = Math.round((money.length / totalResponses) * 100) + '%';
          moneyPercentage.innerHTML = moneyBar.style.width;

          happinessBar.style.width = Math.round((happiness.length / totalResponses) * 100) + '%';
          happinessPercentage.innerHTML = happinessBar.style.width;

          innovationBar.style.width = Math.round((innovation.length / totalResponses) * 100) + '%';
          innovationPercentage.innerHTML = innovationBar.style.width;
          console.log(innovationBar.style.width);

          impactBar.style.width = Math.round((impact.length / totalResponses) * 100) + '%';
          impactPercentage.innerHTML = impactBar.style.width;

          knowledgeBar.style.width = Math.round((knowledge.length / totalResponses) * 100) + '%';
          knowledgePercentage.innerHTML = knowledgeBar.style.width;

          creativityBar.style.width = Math.round((creativity.length / totalResponses) * 100) + '%';
          creativityPercentage.innerHTML = creativityBar.style.width;
      }

      function init() {
          money = ["I want to make money","help"];
          happiness = ["a","b","c"];
          innovation = ["a"];
          impact = ["a","b","c","d"];
          knowledge = ["a","b","c","d","e","f"];
          creativity = ["a"];
          totalResponses = money.length + happiness.length + innovation.length + impact.length +
                              knowledge.length + creativity.length;

          moneyBar = document.getElementById("money-bar");
          moneyPercentage = document.getElementById("money-percentage");

          happinessBar = document.getElementById("happiness-bar");
          happinessPercentage = document.getElementById("happiness-percentage");

          innovationBar = document.getElementById("innovation-bar");
          innovationPercentage = document.getElementById("innovation-percentage");

          impactBar = document.getElementById("impact-bar");
          impactPercentage = document.getElementById("impact-percentage");

          knowledgeBar = document.getElementById("knowledge-bar");
          knowledgePercentage = document.getElementById("knowledge-percentage");

          creativityBar = document.getElementById("creativity-bar");
          creativityPercentage = document.getElementById("creativity-percentage");

          background = document.body;
      }

      function switchCategory(category) {
          switch(category) {
              case money:
                  background.style.visibility = "hidden";
                  background.style.backgroundColor = "#7ED321";
                  var moneyQuestion = document.getElementById("money-question");
                  moneyQuestion.style.visibility = "visible";
                  var innovationQuestion = document.getElementById("innovation-question");
                  innovationQuestion.style.visibility = "hidden";
                  break;
                  console.log(money);
              case happiness:
                  background.style.visibility = "hidden";
                  background.style.backgroundColor = "#F8E71C";
                  var happinessQuestion = document.getElementById("happiness-question");
                  happinessQuestion.style.visibility = "visible";
                  var happinessQuestion = document.getElementById("happiness-question");
                  break;
                  console.log(happiness);
              case innovation:
                  background.style.visibility = "hidden";
                  background.style.backgroundColor = "#2DCFF6";
                  var innovationQuestion = document.getElementById("innovation-question");
                  innovationQuestion.style.visibility = "visible";
                  break;
                  console.log(innovation);
              case impact:
                  background.style.visibility = "hidden";
                  background.style.backgroundColor = "#D0021B";
                  var impactQuestion = document.getElementById("impact-question");
                  impactQuestion.style.visibility = "visible";
                  break;
                  console.log(impact);
              case knowledge:
                  background.style.visibility = "hidden";
                  background.style.backgroundColor = "#FF7F11";
                  var knowledgeQuestion = document.getElementById("knowledge-question");
                  knowledgeQuestion.style.visibility = "visible";
                  break;
                  console.log(knowledge);
              case creativity:
                  background.style.visibility = "hidden";
                  background.style.backgroundColor = "#A999FF";
                  var creativityQuestion = document.getElementById("creativity-question");
                  creativityQuestion.style.visibility = "visible";
                  break;
                  console.log(creativity);

          }
      }

      function submitValues(category) {
          switch(category) {
              case money:
                  totalResponses = totalResponses + 1;
                  var response = document.getElementById("money-response");
                  console.log(response.value);
                  money.push(response.value);
                  window.alert(money);
                  var moneyQuestion = document.getElementById("money-question");
                  var div = document.createElement("div");
                  var responseParagraph = document.createElement("p");
                  responseParagraph.innerHTML = response.value;
                  div.appendChild(responseParagraph);
                  document.getElementById("main").appendChild(div);
                  moneyQuestion.style.visibility = "hidden";
                  backToHome();
                  break;
              case happiness:
                  totalResponses = totalResponses + 1;
                  var response = document.getElementById("happiness-response");
                  console.log(response.value);
                  happiness.push(response.value);
                  window.alert(happiness);
                  var happinessQuestion = document.getElementById("happiness-question");
                  happinessQuestion.style.visibility = "hidden";
                  backToHome();
                  break;
              case innovation:
                  totalResponses = totalResponses + 1;
                  var response = document.getElementById("innovation-response");
                  console.log(response.value);
                  innovation.push(response.value);
                  window.alert(innovation);
                  var innovationQuestion = document.getElementById("innovation-question");
                  innovationQuestion.style.visibility = "hidden";
                  backToHome();
                  break;
              case impact:
                  totalResponses = totalResponses + 1;
                  var response = document.getElementById("impact-response");
                  console.log(response.value);
                  impact.push(response.value);
                  window.alert(impact);
                  var impactQuestion = document.getElementById("impact-question");
                  impactQuestion.style.visibility = "hidden";
                  backToHome();
                  break;
              case knowledge:
                  totalResponses = totalResponses + 1;
                  var response = document.getElementById("knowledge-response");
                  console.log(response.value);
                  knowledge.push(response.value);
                  window.alert(knowledge);
                  var knowledgeQuestion = document.getElementById("knowledge-question");
                  knowledgeQuestion.style.visibility = "hidden";
                  backToHome();
                  break;
              case creativity:
                  totalResponses = totalResponses + 1;
                  var response = document.getElementById("creativity-response");
                  console.log(response.value);
                  creativity.push(response.value);
                  window.alert(creativity);
                  var creativityQuestion = document.getElementById("creativity-question");
                  creativityQuestion.style.visibility = "hidden";
                  backToHome();
                  break;
          }
      }

      function backToHome() {
          background.style.backgroundColor = "black";
          reloadBars();
          background.style.visibility = "visible";
      }
      </script>
      <script type="text/javascript" src="scripts/lights.js"></script>
      <meta charset="UTF-8">
  </head>
  <body class="main" id="main">
      <div class="full-width-text title">
          <h1>Which of the following is your greatest motivation for pursuing higher education?</h1>
      </div>
      <div class="container">
        <div class="column">
          <a class="item" onclick="switchCategory(money)" href="#">
              <div class="category">
                  <h3 class="categoryButton">Money</h3>
                  <div class="money-bar" id="money-bar"></div><p class="percentage" id="money-percentage">0%</p>
              </div>
          </a>
          <a class="item" onclick="switchCategory(happiness)" href="#">
              <div class="category">
                  <h3 class="categoryButton">Happiness</h3>
                  <div class="happiness-bar" id="happiness-bar"></div><p class="percentage" id="happiness-percentage">0%</p>
              </div>
          </a>
          <a class="item" onclick="switchCategory(innovation)" href="#">
              <div class="category">
                  <h3 class="categoryButton">Innovation</h3>
                  <div class="innovation-bar" id="innovation-bar"></div><p class="percentage" id="innovation-percentage">0%</p>
              </div>
          </a>
        </div>
        <div class="column">
          <a class="item" onclick="switchCategory(impact)" href="#">
          <div class="category">
              <h3 class="categoryButton">Impact</h3>
              <div class="impact-bar" id="impact-bar"></div><p class="percentage" id="impact-percentage">0%</p>
          </div>
          </a>
          <a class="item" onclick="switchCategory(knowledge)" href="#">
          <div class="category">
              <h3 class="categoryButton">Knowledge</h3>
              <div class="knowledge-bar" id="knowledge-bar"></div><p class="percentage" id="knowledge-percentage">0%</p>
          </div>
          </a>
          <a class="item" onclick="switchCategory(creativity)" href="#">
          <div class="category">
              <h3 class="categoryButton">Creativity</h3>
              <div class="creativity-bar" id="creativity-bar"></div><p class="percentage" id="creativity-percentage">0%</p>
          </div>
          </a>
        </div>
      </div>
      <div class="question-container" id="innovation-question">
        <div class="question">
          <h3 class="subtitle">What is it about innovation that drive you?</h3>
        </div>
        <div method="post" class="response">
          <button class="white-button" id="innovation_on"  type="button" onclick="buttonclick(this);">On</button>
          <button class="white-button" id="innovation_off" type="button" onclick="buttonclick(this);">Off</button>
          <br>
          <br>
          <br>
          <input type="text" id="innovation_entry">
          <button class="white-button" id="innovation_send" onclick="submitclick(this);">SUBMIT</button>
          <br>
          <br>
          <pre id="innovation_log"></pre>
        </div>

     </div>

     <div class="question-container" id="happiness-question">
       <div class="question">
         <h3 class="subtitle">What is it about happiness that drive you?</h3>
       </div>
       <div method="post" class="response">
         <button class="white-button" id="happiness_on"  type="button" onclick="buttonclick(this);">On</button>
         <button class="white-button" id="happiness_off" type="button" onclick="buttonclick(this);">Off</button>
         <br>
         <br>
         <br>
         <input type="text" id="happiness_entry">
         <button class="white-button" id="happiness_send" onclick="submitclick(this);">SUBMIT</button>
         <br>
         <br>
         <pre id="happiness_log"></pre>
       </div>
    </div>
      <div class="question-container" id="money-question">
        <div class="question">
          <h3 class="subtitle">What is it about money that drive you?</h3>
        </div>
        <div method="post" class="response">
          <button class="white-button" id="money_on"  type="button" onclick="buttonclick(this);">On</button>
          <button class="white-button" id="money_off" type="button" onclick="buttonclick(this);">Off</button>
          <br>
          <br>
          <br>
          <input type="text" id="money_entry">
          <button class="white-button" id="money_send" onclick="submitclick(this);">SUBMIT</button>
          <br>
          <br>
          <pre id="money_log"></pre>
        </div>
      </div>
      <div class="question-container" id="impact-question">
        <div class="question">
          <h3 class="subtitle">What is it about impact that drive you?</h3>
        </div>
        <div method="post" class="response">
          <button class="white-button" id="impact_on"  type="button" onclick="buttonclick(this);">On</button>
          <button class="white-button" id="impact_off" type="button" onclick="buttonclick(this);">Off</button>
          <br>
          <br>
          <br>
          <input type="text" id="impact_entry">
          <button class="white-button" id="impact_send" onclick="submitclick(this);">SUBMIT</button>
          <br>
          <br>
          <pre id="impact_log"></pre>
        </div>
      </div>
      <div class="question-container" id="knowledge-question">
        <div class="question">
          <h3 class="subtitle">What is it about knowledge that drive you?</h3>
        </div>
        <div method="post" class="response">
          <button class="white-button" id="knowledge_on"  type="button" onclick="buttonclick(this);">On</button>
          <button class="white-button" id="knowledge_off" type="button" onclick="buttonclick(this);">Off</button>
          <br>
          <br>
          <br>
          <input type="text" id="knowledge_entry">
          <button class="white-button" id="knowledge_send" onclick="submitclick(this);">SUBMIT</button>
          <br>
          <br>
          <pre id="knowledge_log"></pre>
        </div>
      </div>
      <div class="question-container" id="creativity-question">
        <div class="question">
          <h3 class="subtitle">What is it about creativity that drive you?</h3>
        </div>
        <div method="post" class="response">
          <button class="white-button" id="creativity_on"  type="button" onclick="buttonclick(this);">On</button>
          <button class="white-button" id="creativity_off" type="button" onclick="buttonclick(this);">Off</button>
          <br>
          <br>
          <br>
          <input type="text" id="creativity_entry">
          <button class="white-button" id="creativity_send" onclick="submitclick(this);">SUBMIT</button>
          <br>
          <br>
          <pre id="creativity_log"></pre>
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
