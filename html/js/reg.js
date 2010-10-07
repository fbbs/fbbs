var ok = false;

$(document).ready(function() {
	$("span").hide();
	$('#exist').show();
	$("input").focus(focus).blur(blur).keyup(check);
	$("input[type=button]").unbind('focus').unbind('blur').attr("disabled", true);
	$('#id').focus();
	$("input[name=agree]").click(check);
	$("input[type=button]").click(function() {
		$(this).attr("disabled", true);
		var id = $("#id").get(0).value;
		$.get("bbs/exist", { user: id },
			function(data) {
				var text = $(data).find("bbsexist").text();
				if (text == "1") {
					$('#e1').show().html('ÕÊºÅ' + id + 'ÒÑ±»ÈË×¢²á');
					$('#e0').hide();
				} else {
					$('#e0').show().html('ÕÊºÅ' + id + 'ÉÐÎ´±»×¢²á');
					$('#e1').hide();
				}
			});
	});
	$("select").change(check);
	$("input[type=submit]").click(function () {
		ok = true;
		$("input").each(check);
		$("select").each(check);
		if (!ok) {
			$('#prompt').show();
		}
		return ok;
	});
});

function focus() { $(this).parent().next().show(); }
function blur() { $(this).parent().next().hide(); }
function check() {
	if (checker[this.name] && checker[this.name](this) == false) {
		$(this).addClass('warn');
		ok = false;
		if (this.name == 'id')
			$("input[type=button]").attr("disabled", true);
	} else {
		$(this).removeClass('warn');
		$('#prompt').hide();
		if (this.name == 'id')
			$("input[type=button]").attr("disabled", false);
	}
}

String.prototype.getBytes = function() {
	var c = this.match(/[^\x00-\xff]/ig);
	return this.length + (c == null ? 0 : c.length);
}

var checker = {
	id: function(i) { return i.value.length >= 2 && i.value.length <= 12 && !i.value.match(/[^a-zA-Z]/) },
	pw: function(i) { return i.value.getBytes() >= 4; },
	pw2: function(i) { return i.value.getBytes() >= 4 && i.value == $('#pw').val(); },
	mail: function(i) { return i.value; },
	nick: function(i) { b = i.value.getBytes(); return b >= 2 && b <= 39; },
	byear: function(i) { var y = parseInt(i.value); if (y < 100) y += 1900; return y > 1900 && y < 2000; },
	bmon: function(i) { var s = $("#bmon option:selected"); return s && s.val() != "0"; },
	bday: function(i) { var s = $("#bday option:selected"); return s && s.val() != "0"; },
	name: function(i) { return i.value.getBytes() >= 4 && i.value.match(/[^\x00-\xff]/ig) != null; },
	tel: function(i) { return i.value.length >= 8 && !i.value.match(/[^0-9\-\+\(\)\*x]/);},
	agree: function(i) { return i.checked; }
}
