function createArray() {
  if (!localStorage.getItem("responses")) {
    localStorage.setItem("responses", JSON.stringify([]));
  }
}

function submitResponse() {
  let responses = JSON.parse(localStorage.getItem("responses"));
  responses.push(document.getElementById("response").value);
  localStorage.setItem("responses", JSON.stringify(responses));
  console.log(responses);
}
