
function getText(url, callback) {
	var req = XMLHttpRequest();
	req.open("GET", url);
	req.onreadystatechange = function() {
		if (req.readystate == 4 || req.readystate == 200) {
			var type = req.getResponseHeader("Content-type");
			if (type.match(/^text/)) {
				callback(req.responseText);
			}
		}
	}
}

function onTextReady(text) {
	console.log(text);
}

getText("http://192.168.1.102/good.txt", onTextReady);

