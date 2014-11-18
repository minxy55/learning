// JavaScript Document

function messageHandler(e) {
	var result = 3;
	var ts = JSON.parse(e.data);
	switch (ts["sig"]) {
		case '+':
			result = ts["t1"] + ts["t2"];
			break;
		case '-':
			result = ts["t1"] - ts["t2"];
			break;
		case '*':
			result = ts["t1"] * ts["t2"];
			break;
		case '/':
			result = ts["t1"] / ts["t2"];
			break;
		case '%':
			result = ts["t1"] % ts["t2"];
			break;
		case '^':
			result = ts["t1"] ^ ts["t2"];
			break;
		default:
			result = "unknown";
			break;
	}
	postMessage("worker says: " + ts["t1"] + " " + ts["sig"] + " " + ts["t2"] + " = " + result);
}

addEventListener("message", messageHandler, true);
