function expandFav() {
	document.getElementById('fav').style.display = 'block';
	document.getElementById('fave').style.display = 'none';
	document.getElementById('favc').style.display = 'inline-block';
}

function collapseFav() {
	document.getElementById('fav').style.display = 'none';
	document.getElementById('fave').style.display = 'inline-block';
	document.getElementById('favc').style.display = 'none';
}

function addLoadEvent(func) {
	if (typeof window.onload != 'function') {
		window.onload = func;
	} else {
		var old = window.onload;
		window.onload = function() {
			old();
			func();
		}
	}
}

function autoHideFav() {
	var fav = document.getElementById('fav');
	var brds = fav.getElementsByTagName("a");
	if (brds.length > 5)
		collapseFav();
}

addLoadEvent(autoHideFav);
