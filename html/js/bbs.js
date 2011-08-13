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

function hideP()
{
	$('.prompt').hide();
}

$.fn.toggleLoading = function() {
	if (this.attr('disabled')) {
		$('#loading').hide();
		return this.attr('disabled', false);
	} else {
		hideP();
		$('#loading').show();
		return this.attr('disabled', true);
	}
}

function error(text) {
	hideP();
	$('#error').text(text).fadeIn('fast');
}

function replyButton() {
	var div = $(this).parent().parent();
	var f = $('div.quick_reply', div);
	var action = $('.reply').attr('href').replace(/^pst/, 'snd') + '&utf8=1';
	if (!f.length) {
		$(this).parent().after($('#quick_reply').clone(false).removeAttr('id'));
		f = $('div.quick_reply', div);
		$('form.quick_reply', div).attr('action', action);

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
		$('.cancel', f).click(function() { hideP(); f.hide(); $('.plink', div).show(); });
		$('.confirm', f).click(replyFormSubmit);

		$('iframe').load(function() {
			$('[type="file"]').toggleLoading();
			var url = $(this).contents().find('#url');
			if (url.length) {
				var text = $('textarea', f);
				text.val(text.val() + "\n" + url.text() + "\n");
			} else {
				alert("Error!");
			}
		});
		$('[type="file"]', f).change(function() {
			$(this).parent().submit();
			$('[type="file"]').toggleLoading();
		});
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
	button.toggleLoading();
	var form = $('form.quick_reply', $(this).parent().parent());
	$.ajax({
		type: 'POST', url: form.attr('action'), data: form.serialize(),
		success: function(data) {
			var url = $(data).filter('#url');
			if (url.length) {
				$('<div class="preply"></div>').text($('textarea', form).val().replace(/【 [\s\S]+/, " ...")).prepend('<a class="success" href="' + url.attr('href') + '">回复成功</a>').insertAfter(form.parent());
				form.parent().slideUp().prev().slideDown();
			} else {
				error($(data).filter('div').text());
			}
		},
		error: function() {
			error('发送失败，请稍候重试');
		},
		complete: function() {
			button.toggleLoading();
		}
	});
	return false;
}

function signatureOption()
{
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

function crossPostButton()
{
	var div = $(this).parent().parent();
	var f = $('form.quick_cp', div);
	var toggle = function() { f.toggle(); $('.plink', div).toggle(); };
	if (!f.length) {
		f = $('<form class="quick_cp"><input type="button" class="cancel" value="取消"/>转载到版面 <input type="text" name="t"/><input type="submit" value="转载"/></form>').attr('action', $(this).attr('href'));
		f.insertAfter($(this).parent());
		$('.cancel').click(toggle);
		$('[type="submit"]', f).click(function() {
			if (!$('[name="t"]', f).val())
				return false;
			$(this).toggleLoading();
			$.ajax({
				type: 'POST', url: f.attr('action'), data: f.serialize(),
				success: function(data) {
					var node = $(data).find('bbsccc');
					if (node.length) {
						var fid = node.attr('f');
						var b = node.attr('b');
						$('<div class="preply"></div>').text($('[name="t"]', f).val()).prepend('<a class="success" href=con?new=1&bid="' + b + '&f=' + fid + '">转载成功</a>').insertAfter(f);
						toggle();
					} else {
						alert($(data).filter('div').text());
					}
				},
				complete: function() { $('[type="submit"]', f).toggleLoading(); }
			});
			return false;
		});
	}
	toggle();
	$('[name="t"]', f).focus();
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
		$('form[name="maillist"] input:checkbox').attr('checked', true);
		return false;
	});
	$('.check_rev').click(function () {
		$('form[name="maillist"] input:checkbox').attr('checked', function() { return !this.checked; });
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
	});
	$('#navl').click(function() {
		$('#login-dialog-form').dialog('open');
		return false;
	});
});
