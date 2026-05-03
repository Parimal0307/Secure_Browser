window.api.onKeyEvent((data) => {
    document.getElementById("output").innerText =
        "Key Event: " + data;
});

// ----------------------------
// 🔥 HANDLE SECURITY EVENTS
// ----------------------------
window.api.onSecurityEvent((event) => {

    console.log("Security Event:", event);

    const box = document.getElementById("alerts");

    switch (event) {

        case "CLOSE_BLOCKED":
            box.innerText = "🚫 Attempt to close app blocked";
            break;

        case "MINIMIZE_BLOCKED":
            box.innerText = "🚫 Minimize is not allowed";
            break;

        case "FOCUS_LOST":
            box.innerText = "⚠️ You switched away from exam!";
            break;

        case "FOCUS_GAINED":
            box.innerText = "✅ Back to exam";
            break;
    }
});