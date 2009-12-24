// navigation
function switchPanel(link)
{
	var item = link.nextSibling;
	var expand = false;
	if (item.style.display == 'block') {
		item.style.display = 'none';
	} else {
		item.style.display = 'block';
		expand = true;
	}

	var id = link.parentNode.id;
	bbs.store.get('navbar', function(ok, val) {
		var str = '0000000';
		if (ok && val && val.toString().length == 7)
			str = val.toString();
		for (var i = 0; i < bbs.navbar.length; ++i) {
			if (bbs.navbar[i] == id)
				str = str.substring(0, i) + (expand ? 1 : 0) + str.substring(i + 1);
		}
		bbs.store.set('navbar', str);
	});
	return false;
}

// for bbspst
function preUpload()
{
	var mywin = window.open('preupload?board=' + document.getElementById('brd').value, '_blank', 'width=600,height=300,scrollbars=yes');
	if ((document.window != null) && (!mywin.opener))
		mywin.opener = document.window;
	mywin.focus();
	return false;
}
document.onkeydown = function(evt) {
	document.onkeypress = function() {return true;};
	if (!document.getElementById('postform'))
		return true;
	evt = evt ? evt : event;
	if ((evt.keyCode == 87) && evt.ctrlKey) { // Ctrl-W
		document.onkeypress = function() {return false;};
		document.postform.submit();
		return false;
	}
};

// for bbsmail
function checkAll() {
	var inputs = document.getElementsByName('list')[0].getElementsByTagName('input');
	for (var i = 0; i < inputs.length; i++) {
		if (inputs[i].type == 'checkbox') {
			inputs[i].checked = true;
		}
	}
	return false;
}
function checkReverse() {
	var inputs = document.getElementsByName('list')[0].getElementsByTagName('input');
	for (var i = 0; i < inputs.length; i++) {
		if (inputs[i].type == 'checkbox')
			inputs[i].checked = !(inputs[i].checked);
	}
	return false;
}
function delSelected() {
	document.list.mode.value = 1;
	document.list.submit();
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

var HTTP = {
	_factories: [
		function() { return new XMLHttpRequest(); },
		function() { return new ActiveXObject("Msxml2.XMLHTTP"); },
		function() { return new ActiveXObject("Microsoft.XMLHTTP"); }
	],
	_factory: null,
	newRequest: function() {
		if (HTTP._factory != null)
			return HTTP._factory();
		for(var i = 0; i < HTTP._factories.length; i++) {
			try {
				var factory = HTTP._factories[i];
				var request = factory();
				if (request != null) {
					HTTP._factory = factory;
					return request;
				}
			}
			catch(e) {
				continue;
			}
		}
		HTTP._factory = function() {
			throw new Error("XMLHttpRequest not supported");
		}
		HTTP._factory();
	},
};

var bbs = {
	inited: false,
	interval: 10000,
	root: window.location.href.substring(0, window.location.href.lastIndexOf("/")),
	init: function () {
		if (!bbs.inited) {
			bbs.root = bbs.root.replace(/\W/g, "_");
			bbs.store = new Persist.Store(bbs.root);
			bbs.inited = true;
		}
	},
	check: function () {
		bbs.init();
		bbs.store.get('last', function(ok, val) {
			var now = new Date().getTime();
			setTimeout(bbs.check, bbs.interval);
			if (ok && val && now - parseInt(val) < bbs.interval)
				return;
			var request = HTTP.newRequest();
			request.onreadystatechange = function() {
				if (request.readyState == 4) {
					if (request.status == 200)
						var res = request.responseXML.getElementsByTagName('bbsidle')[0].getAttribute('mail');
						if (res != 0) {
							document.getElementById('navnm').style.display = 'inline-block';
							document.getElementById('navmc').innerHTML = res;
							
						}
				}
			}
			request.open('GET', 'idle', true);
			request.send(null);
			bbs.store.set('last', now);
		});
	},
	navbar: [ 'navb', 'navt', 'navf', 'navc', 'navm', 'navco', 'navs' ],
	navinit: function() {
		bbs.init();
		bbs.store.get('navbar', function(ok, val) {
			if (ok && val) {
				var str = val.toString();
				for (var i = 0; i < bbs.navbar.length; i++) {
					if (str[i] != '0')
						document.getElementById(bbs.navbar[i]).getElementsByTagName('ul')[0].style.display = 'block';
				}
			}
		});
	}
};

addLoadEvent(bbs.navinit);
addLoadEvent(bbs.check);