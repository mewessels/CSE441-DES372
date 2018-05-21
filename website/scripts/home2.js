
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
