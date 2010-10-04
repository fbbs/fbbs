var ok = false;

$(document).ready(function() {
	$("span").hide();
	$('#id').focus();
});

$("input").focus(focus).blur(blur).keyup(check);
$("input[name=agree]").click(check);
$("select").change(check);
$("input[type=submit]").click(function () {
	ok = true;
	$("input").each(check);
	$("select").each(check);
	return ok;
});

function focus() { $(this).parent().next().show(); }
function blur() { $(this).parent().next().hide(); }
function check() {
	if (checker[this.name] && checker[this.name](this) == false) {
		$(this).addClass('warn');
		ok = false;
	} else {
		$(this).removeClass('warn');
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
	tel: function(i) { return i.value.length >= 8 && !i.value.match(/[^0-9]/);},
	agree: function(i) { return i.checked; }
}
