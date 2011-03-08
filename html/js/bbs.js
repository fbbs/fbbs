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

$.fn.selectRange = function(b, e) {
	return this.each(function() {
			if (this.setSelectionRange) {
				this.setSelectionRange(b, e);
			} else if (this.createTextRange) {
				var range = this.createTextRange();
				range.collapse(true);
				range.moveEnd('character', b);
				range.moveStart('character', e);
				range.select();
			}
		});
}

function replyButton() {
	var div = $(this).parent().parent();
	var f = $('form', div);
	var action = $('.reply').attr('href').replace(/^pst/, 'snd') + '&utf8=1';
	if (!f.length) {
		$(this).parent().after($('#quick_reply').clone(false).removeAttr('id').attr('action', action));
		f = $('form', div);

		var title = $('.ptitle', div).text().replace(/\xa0/g, ' ');
		if (title.substring(0, 4) == 'Re: ') {
			title = 'Re: ' + title.substring(4);
		} else {
			title = 'Re: ' + title;
		}
		$('[name="title"]', f).val(title);

		var q = [ '', '【 在 ' + $('.powner', div).text() + ' 的大作中提到: 】' ];
		var p = [];
		$('p', div).each(function() { p.push($(this).text()) });
		p.splice(0, 3);
		var quoted = 0;
		for (var i = 0; i < p.length; ++i) {
			p[i] = p[i].replace(/\xa0/g, ' ');
			if (!p[i].length || p[i].substring(0, 4) == ': 【 ' || p[i].substring(0, 4) == ': : ')
				continue;
			if (p[i].substring(0, 2) == '--')
				break;
			if (++quoted >= 10) {
				q.push(': ' + ': .................（以下省略）');
				break;
			}
			q.push(': ' + p[i]);
		}
		$('[name="text"]', f).val(q.join('\n'));
		$('.cancel', f).click(function() { f.hide(); $('.plink', div).show(); });
		$('.confirm', f).click(replyFormSubmit);
	}
	if (f.is(':visible')) {
		f.hide();
	} else {
		$('.plink', div).hide();
		f.show();
		$('textarea', f).focus().selectRange(0, 0);
	}
	return false;
}

function replyFormSubmit() {
	var button = $(this);
	button.attr('disabled', true);
	var form = $(this).parent().parent();
	$('.loading', form).show();
	$.ajax({
		type: 'POST', url: form.attr('action'), data: form.serialize(),
		success: function(data) {
			if ($(data).filter('a').text() == '快速返回') {
				alert($(data).filter('div').text());
			} else {
				form.after("<div class='preply'></div>");
				form.next().text($('textarea', form).val().replace(/【 [\s\S]+/, " ...")).prepend('<span class="success">回复成功</span>');
				form.slideUp().prev().slideDown();
			}
		},
		error: function() {
			alert('发送失败，请稍候重试');
		},
		complete: function() {
			$('.loading', form).hide();
			button.attr('disabled', false);
		}
	});
	return false;
}

function signatureOption()
{
	var form = $(this).next();
	$('.cancel', form).unbind('click').click(function() { form.slideUp('fast'); });
	$('[type=submit]', form).unbind('click').click(function() {
		$(this).attr('disabled', true);
		$.get(form.attr('action'), form.serialize(), form.slideToggle('fast'));
		$(this).attr('disabled', false);
		return false;
	});
	form.slideToggle('fast');
}

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

	$('.reply').click(replyButton);
	$('a.sig_option').click(signatureOption);
});
