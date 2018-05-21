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
