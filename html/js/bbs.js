function switchPanel() {
	var item = $(this).next();
	item.toggle();

	var expand = item.is(':visible');
	var id = item.parent().attr('id');

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

// for bbsmail
function checkAll() {
	$('form[name="list"] input:checkbox').attr('checked', true);
	return false;
}
function checkReverse() {
	$('form[name="list"] input:checkbox').attr('checked', function() {
		return !this.checked;
	});
	return false;
}
function delSelected() {
	document.list.mode.value = 1;
	document.list.submit();
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
			$('#navnm').show();
			$('#navmc').html(mail);
		} else {
			$('#navnm').hide();
		}
	},
	check: function () {
		bbs.init();
		bbs.store.get('last', function(ok, val) {
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
			$.get('idle', { date: now },
				function(data) {
					var res = $(data).find('bbsidle').attr('mail');
					bbs.store.set('mail', res);
					bbs.showmail(res);
				});
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
					$('#' + bbs.navbar[i] + ' ul').toggle(str.charAt(i) != '0');
				}
			}
		});
	},
	syncinterval: 2000,
	sync: function() { bbs.navinit(); bbs.check(); }
};

$(document).ready(function() {
	$('#navnm').hide();

	for (var i = 0; i < bbs.navbar.length; ++i) {
		$('#' + bbs.navbar[i] + '>a').click(switchPanel);
	}

	bbs.sync();
	setInterval(bbs.sync, bbs.syncinterval);

	document.onkeydown = function(evt) {
		document.onkeypress = function() { return true; };
		var form = $('#postform');
		if (!form.length)
			return true;
		evt = evt ? evt : event;
		if ((evt.keyCode == 87) && evt.ctrlKey) { // Ctrl-W
			document.onkeypress = function() { return false; };
			form.submit();
			return false;
		} else {
			return true;
		}
	};
});
