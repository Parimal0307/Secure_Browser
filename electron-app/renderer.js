window.api.onKeyEvent((data) => {
    document.getElementById("output").innerText = "Key Event: " + data;
});