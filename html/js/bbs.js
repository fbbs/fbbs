/*
 * jQuery Hotkeys Plugin
 * Copyright 2010, John Resig
 * Dual licenced under the MIT or GPLv2 licenses.
 */
(function(jQuery){
	jQuery.hotkeys = {
		version: '0.8',
		specialKeys: {
			8: 'backspace', 9: 'tab', 13: 'return', 16: 'shift', 17: 'ctrl', 18: 'alt', 19: 'pause',
			20: 'capslock', 27: 'esc', 32: 'space', 33: 'pageup', 34: 'pagedown', 35: 'end', 36: 'home',
			37: 'left', 38: 'up', 39: 'right', 40: 'down', 45: 'insert', 46: 'del',
			96: '0', 97: '1', 98: '2', 99: '3', 100: '4', 101: '5', 102: '6', 103: '7',
			104: '8', 105: '9', 106: '*', 107: '+', 109: '-', 110: '.', 111 : '/',
			112: 'f1', 113: 'f2', 114: 'f3', 115: 'f4', 116: 'f5', 117: 'f6', 118: 'f7', 119: 'f8',
			120: 'f9', 121: 'f10', 122: 'f11', 123: 'f12', 144: 'numlock', 145: 'scroll', 191: '/', 224: 'meta'
		},
		shiftNums: {
			'`': '~', '1': '!', '2': '@', '3': '#', '4': '$', '5': '%', '6': '^', '7': '&',
			'8': '*', '9': '(', '0': ')', '-': '_', '=': '+', ';': ': ', "'": '\'', ',': '<',
			'.': '>',  '/': '?',  '\\': '|'
		}
	};

	function keyHandler(handleObj) {
		// Only care when a possible input has been specified
		if (typeof handleObj.data !== 'string') {
			return;
		}
		
		var origHandler = handleObj.handler,
			keys = handleObj.data.toLowerCase().split(' ');
	
		handleObj.handler = function(event) {
			// Don't fire in text-accepting inputs that we didn't directly bind to
			if (this !== event.target && (/textarea|select/i.test(event.target.nodeName) ||
				 event.target.type === 'text')) {
				return;
			}
			
			// Keypress represents characters, not special keys
			var special = event.type !== 'keypress' && jQuery.hotkeys.specialKeys[event.which],
				character = String.fromCharCode(event.which).toLowerCase(),
				key, modif = '', possible = {};

			// check combinations (alt|ctrl|shift+anything)
			if (event.altKey && special !== 'alt') {
				modif += 'alt+';
			}

			if (event.ctrlKey && special !== 'ctrl') {
				modif += 'ctrl+';
			}
			
			// TODO: Need to make sure this works consistently across platforms
			if (event.metaKey && !event.ctrlKey && special !== 'meta') {
				modif += 'meta+';
			}

			if (event.shiftKey && special !== 'shift') {
				modif += 'shift+';
			}

			if (special) {
				possible[modif + special] = true;
			} else {
				possible[modif + character] = true;
				possible[modif + jQuery.hotkeys.shiftNums[character]] = true;

				// '$' can be triggered as 'Shift+4' or 'Shift+$' or just '$'
				if (modif === 'shift+') {
					possible[jQuery.hotkeys.shiftNums[character]] = true;
				}
			}

			for (var i = 0, l = keys.length; i < l; i++) {
				if (possible[keys[i]]) {
					return origHandler.apply(this, arguments);
				}
			}
		};
	}

	jQuery.each(['keydown', 'keyup', 'keypress'], function() {
		jQuery.event.special[this] = { add: keyHandler };
	});

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

	$.fn.toggleLoading = function() {
		if (this.prop('disabled')) {
			$('#loading').hide();
			return this.prop('disabled', false);
		} else {
			$('.prompt').hide();
			$('#loading').show();
			return this.prop('disabled', true);
		}
	}
})(jQuery);

function switchPanel() {
	var item = $(this).next();
	var expand = !item.is(':visible');
	item.slideToggle();
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

function delSelected() {
	document.list.mode.value = 1;
	document.list.submit();
}

function ie6fix() {
	$('#nav').css('height', document.documentElement.clientHeight - 10);
	var width = document.documentElement.clientWidth - 150;
	$('#hd, table.content, div.post').css('width', width);
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

function error(text) {
	$('.prompt').hide();
	$('#error').text(text).fadeIn('fast');
}

function generateReplyTitle(div) {
	var title = $('.ptitle', div).text().replace(/\xa0/g, ' ');
	if (title.substring(0, 4) == 'Re: ') {
		title = 'Re: ' + title.substring(4);
	} else {
		title = 'Re: ' + title;
	}
	return title;
}

function generateQuote(div) {
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
	return q;
}

function replyButton() {
	$('#quick-reply-button').removeAttr('id');
	var link = $(this).attr('id', 'quick-reply-button');
	var div = link.parent().parent();

	var form = $('#quick-reply-form');
	form.attr('action', $(this).attr('href').replace(/^pst/, 'snd') + '&utf8=1');
	$('[name="title"]', form).val(generateReplyTitle(div));
	$('[name="text"]', form).val(generateQuote(div).join('\n'));

	$('#quick-reply-error').hide();
	$('#quick-reply').dialog('open');

	$('textarea', form).focus().selectRange(0, 0);
	return false;
}

function quickUpload() {
	if ($(this).contents().find('title').length) {
		$('#quick-upload input').toggleLoading();
		var url = $(this).contents().find('#url');
		if (url.length) {
			var text = $('#quick-reply textarea');
			text.val(text.val() + "\n" + url.text() + "\n");
		} else {
			alert("Error!");
		}
	}
}

function replyFormSubmit() {
	var form = $('#quick-reply-form');
	$('#quick-reply').dialog('disable');
	$.ajax({
		type: 'POST', url: form.attr('action'), data: form.serialize(),
		success: function(data) {
			var url = $(data).filter('#url');
			if (url.length) {
				$('<div class="preply"></div>').text($('textarea', form).val().replace(/【 [\s\S]+/, " ...")).prepend('<a class="success" href=' + url.attr('href') + '>回复成功</a>').insertAfter($('#quick-reply-button').parent()).hide().slideDown();
				$('#quick-reply').dialog('close');
			} else {
				$('#quick-reply-error').text($(data).filter('div').text()).slideDown();
			}
		},
		error: function() {
			$('#quick-reply-error').text('发送失败，请稍候重试').slideDown();
		},
		complete: function() {
			$('#quick-reply').dialog('enable');
		}
	});
	return false;
}

function signatureOption() {
	var form = $(this).next();
	$('.cancel', form).unbind('click').click(function() { form.slideUp('fast'); });
	$('[type=submit]', form).unbind('click').click(function() {
		$(this).toggleLoading();
		$.get(form.attr('action'), form.serialize(), form.slideToggle('fast'));
		$(this).toggleLoading();
		return false;
	});
	form.slideToggle('fast');
}

function crossPostButton() {
	var link = $(this);
	var div = $('#quick-cp');
	$('form', div).attr('action', $(this).attr('href'));
	div.dialog({
		autoOpen: false, modal: true, resizable: false,
		buttons: {
			'转载': function() { crossPostSubmit(link); },
			'取消': function() { $(this).dialog('close'); }
		}
	}).dialog('open');
	var boards = [];
	$('#navf li.sf').each(function() { boards.push($(this).text()); });
	$('input', div).autocomplete({ source: boards, delay: 100 });
	return false;
}

function crossPostSubmit(link) {
	var f = $('#quick-cp form');
	if (!$('[name="t"]', f).val())
		return false;
	$('#quick-cp').dialog('disable');
	$.ajax({
		type: 'POST', url: f.attr('action'), data: f.serialize(),
		success: function(data) {
			var node = $(data).find('bbsccc');
			if (node.length) {
				var fid = node.attr('f');
				var b = node.attr('t');
				$('<div class="preply"></div>').text($('[name="t"]', f).val()).prepend('<a class="success" href=con?new=1&bid=' + b + '&f=' + fid + '>转载成功</a>').insertAfter(link.parent()).hide().slideDown();
			} else {
				alert($(data).filter('div').text());
			}
		},
		complete: function() { $('#quick-cp').dialog('enable').dialog('close'); }
	});
	return false;
}

function HumanReadableDate() {
	var s = 86400000;
	this.n = new Date();
	var t = new Date(this.n.getFullYear(), this.n.getMonth(), this.n.getDate(), 0, 0, 0, 0);
	this.t1 = new Date(t.getTime() - s);
	this.t2 = new Date(t.getTime() - s * 2);
	this.t3 = new Date(this.n.getFullYear(), 0, 1, 0, 0, 0, 0);
	this.get = function (str, opt) {
		var m = str.match(/(\d+)-(\d+)-(\d+) (\d+):(\d+)/);
		var d = new Date(m[1], m[2] - 1, m[3], m[4], m[5], 0, 0);
		var e = this.n.getTime() - d.getTime();
		if (e < 0)
			return str;
		if (opt != 'short') {
			if (e < 60000)
				return '刚才';
			if (e < s / 24)
				return Math.floor(e / 60000) + ' 分钟前';
			if (e < s)
				return Math.floor(e / 3600000) + ' 小时前';
			if (d > this.t1)
				return '昨天 ' + m[4] + ':' + m[5];
			if (d > this.t2)
				return '前天 ' + m[4] + ':' + m[5];
			if (e < s * 8)
				return Math.floor(e / s) + ' 天前';
		}
		if (d > this.t3)
			return m[2] + '-' + m[3] + ' ' + m[4] + ':' + m[5];
		return str;
	}
}

function buyPropClick() {
	var s = $(this);
	var p = s.parent().parent();
	var m = s.attr('href').match(/(\d+)$/);
	if (m[1] == 2 || m[1] == 3 || m[1] == 4) {
		p.after('<tr><td colspan="4"><form method="POST"><label for="title">请输入自定义身份</label><input name="title" type="text" width="15"></input><input type="submit" value="提交"></input></form></td></tr>');
		$('form', p.next()).attr('action', s.attr('href'));
		s.unbind('click').bind('click', function() {
			p.next().remove();
			s.unbind('click').bind('click', buyPropClick);
			return false;
		});
	}
	return false;
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

	$('#quick-reply').dialog({
		autoOpen: false, modal: false, minWidth: 640, resizable: false,
		buttons: {
			'发表(Ctrl+Enter)': replyFormSubmit,
			'取消(Esc)': function() { $(this).dialog('close'); }
		}
	});
	$('#quick-upload input').change(function() { $('#quick-upload').submit(); $(this).toggleLoading(); });
	$('#quick-reply form textarea').bind('keydown', 'ctrl+return', replyFormSubmit);
	$('.reply').click(replyButton);

	$('a.sig_option').click(signatureOption);
	$('.crosspost').click(crossPostButton);
	$('#forum td.ptitle').mouseenter(function() {
		if ($('a.lastpage', this).length)
			$('a', this).css('display', 'inline-block');
	}).mouseleave(function() {
		$('a.lastpage', this).hide();
	});

	var h = new HumanReadableDate();
	$('div.uptime').each(function () {
		var text = $(this).text();
		var t = h.get(text);
		$(this).attr('title', text);
		$(this).text(t);
	});
	$('div.time').each(function () {
		var text = $(this).text();
		var t = h.get(text, 'short');
		$(this).attr('title', text);
		$(this).text(t);
	});

	$('.check_all').click(function () {
		$('form[name="maillist"] input:checkbox').prop('checked', true);
		return false;
	});
	$('.check_rev').click(function () {
		$('form[name="maillist"] input:checkbox').prop('checked', function() { return !this.checked; });
		return false;
	});

	$('.del_mail').click(function () {
		var f = $('form[name="maillist"]');
		if (!$('input:checked', f).length) {
			error('您还没有选择信件@.@');
			return false;
		}
		var b = $(this).toggleLoading();
		$.ajax({
			type: 'POST', url: f.attr('action'), data: f.serialize(),
			success: function(data) {
				$(data).find('mail').each(function() {
					$('[name="box' + $(this).attr('f') + '"]', f).parent().parent().fadeOut('slow');
				});
			},
			complete: function() { b.toggleLoading(); }
		});
	});

	$('#login-dialog-form').dialog({
		autoOpen: false, modal: true, resizable: false,
		buttons: {
			'登录': function() { $('#login-dialog-form form').submit(); },
			'取消': function() { $(this).dialog('close'); }
		}
	}).bind('keydown', 'return', function() { $('form', this).submit(); });
	$('#navl').click(function() {
		$('#login-dialog-form').dialog('open');
		return false;
	});

	$('iframe').load(quickUpload);

	$('.buy-prop').click(buyPropClick);
});
