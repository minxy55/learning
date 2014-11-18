// JavaScript Document

function toPx(v)
{
	return "" + v + "px";
}

function toUrl(u)
{
	return "url(" + u + ")";
}

function back2(url)
{
	window.location.href = url;
}

function getParam(key)
{
	var parameter = new Object();
	var s = window.location.href;
	
	var i = s.indexOf('?');
	if (i < 0) {
		return null;
	}
	
	var ps = s.substr(i + 1);
	var params = ps.split('&');
	if (params === null) {
		return null;
	}
	
	console.log("length = " + params.length);
	for (var j = 0; j < params.length; j++) {
		if (params[j].indexOf('=') >= 0) {
			console.log("index in");
			var kv = params[j].split('=');
			
			if (kv[0] === key)
				return kv[1];
		}
	}
}