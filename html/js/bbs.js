function switchPanel(link) {
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

function ie6fix() {
	var nav = document.getElementById('nav');
    nav.style.height = document.documentElement.clientHeight - 10;
    var width = document.documentElement.clientWidth - 150;
    document.getElementById('hd').style.width = width;
    var div = document.getElementsByTagName('table');
    for (var i = 0; i < div.length; ++i) {
		if (div[i].className == 'content')
			div[i].style.width = width;
	}
    div = document.getElementsByTagName('div');
    for (var i = 0; i < div.length; ++i) {
		if (div[i].className == 'post')
			div[i].style.width = width;
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
	}
};

var bbs = {
	inited: false,
	interval: 300000,
	root: window.location.href.substring(0, window.location.href.lastIndexOf("/")),
	init: function () {
		if (!bbs.inited) {
			bbs.root = bbs.root.replace(/\W/g, "_");
			bbs.store = new Persist.Store(bbs.root, { swf_path: '../persist.swf' });
			bbs.inited = true;
		}
	},
	showmail: function(mail) {
		if (mail > 0) {
			document.getElementById('navnm').style.display = 'inline-block';
			document.getElementById('navmc').innerHTML = mail;
		} else {
			document.getElementById('navnm').style.display = 'none';
		}
	},
	check: function () {
		bbs.init();
		bbs.store.get('last', function(ok, val) {
			//alert(parseInt(val));
			var now = new Date().getTime();
			if (ok && val && now - parseInt(val) < bbs.interval) {
				bbs.store.get('mail', function(ok, val) {
					if (ok && val) {
						var mail = parseInt(val);
						bbs.showmail(mail);
					}
				});
				return;
			}
			var request = HTTP.newRequest();
			request.onreadystatechange = function() {
				if (request.readyState == 4) {
					if (request.status == 200) {
						var res = request.responseXML.getElementsByTagName('bbsidle')[0].getAttribute('mail');
						bbs.store.set('mail', res);
						bbs.showmail(res);
					}
				}
			}
			request.open('GET', 'idle?date=' + now, true);
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
					var nav = document.getElementById(bbs.navbar[i]);
					nav && (nav.getElementsByTagName('ul')[0].style.display = (str.charAt(i) == '0' ? 'none' :'block'));
				}
			}
		});
	},
	syncinterval: 2000,
	sync: function() { bbs.navinit(); bbs.check(); }
};

addLoadEvent(bbs.sync);
setInterval(bbs.sync, bbs.syncinterval);