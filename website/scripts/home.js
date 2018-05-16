
var totalResponses;
var money;
var happiness;
var innovation;
var impact;
var love;
var creativity;

var background;

var moneyBar;
var moneyPercentage;

var happinessBar;
var happinessPercentage;

var impactBar;
var impactPercentage;

var loveBar;
var lovePercentage;

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

    loveBar.style.width = Math.round((love.length / totalResponses) * 100) + '%';
    lovePercentage.innerHTML = loveBar.style.width;

    creativityBar.style.width = Math.round((creativity.length / totalResponses) * 100) + '%';
    creativityPercentage.innerHTML = creativityBar.style.width;
}

function init() {
    money = ["I want to make money","help"];
    happiness = ["a","b","c"];
    innovation = ["a"];
    impact = ["a","b","c","d"];
    love = ["a","b","c","d","e","f"];
    creativity = ["a"];
    totalResponses = money.length + happiness.length + innovation.length + impact.length +
                        love.length + creativity.length;

    moneyBar = document.getElementById("money-bar");
    moneyPercentage = document.getElementById("money-percentage");

    happinessBar = document.getElementById("happiness-bar");
    happinessPercentage = document.getElementById("happiness-percentage");

    innovationBar = document.getElementById("innovation-bar");
    innovationPercentage = document.getElementById("innovation-percentage");

    impactBar = document.getElementById("impact-bar");
    impactPercentage = document.getElementById("impact-percentage");

    loveBar = document.getElementById("love-bar");
    lovePercentage = document.getElementById("love-percentage");

    creativityBar = document.getElementById("creativity-bar");
    creativityPercentage = document.getElementById("creativity-percentage");

    background = document.body;
}

function switchCategory(category) {
    switch(category) {
        case money:

        case happiness:

        case innovation:
            background.style.visibility = "hidden";
            background.style.backgroundColor = "#2DCFF6";
            var innovationQuestion = document.getElementById("innovation-question");
            innovationQuestion.style.visibility = "visible";
        case impact:

        case love:

        case creativity:

    }
}

function submitValues(category) {
    switch(category) {
        case innovation:
            totalResponses = totalResponses + 1;
            var response = document.getElementById("innovation-response");
            console.log(response.value);
            innovation.push(response.value);
            window.alert(innovation);
            var innovationQuestion = document.getElementById("innovation-question");
            innovationQuestion.style.visibility = "hidden";
            backToHome();
    }
}

function backToHome() {
    background.style.backgroundColor = "black";
    background.style.visibility = "visible";
    reloadBars();
}
