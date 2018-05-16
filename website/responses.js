function createArray() {
  // creates responses array in localStorage if it doesn't already exist
  if (!localStorage.getItem("responses")) {
    localStorage.setItem("responses", JSON.stringify([]));
  }
}

function submitResponse() {
  let responses = JSON.parse(localStorage.getItem("responses"));
  responses.push(document.getElementById("response").value);
  localStorage.setItem("responses", JSON.stringify(responses)); // localStorage only stores strings so we store the array in JSON format
  console.log(responses);
}
