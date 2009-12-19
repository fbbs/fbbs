// navigation
function switchPanel(link)
{
	var item = link.nextSibling;
	if (item.style.display == 'block') {
		item.style.display = 'none';
	} else {
		item.style.display = 'block';
	}
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

function saveHTML() {
	win = window.open();
	win.document.write('<html>' + document.getElementsByTagName('head')[0].innerHTML + '<body>');
	var divs = document.getElementById('main').getElementsByTagName('div');
	for (var i = 0; i < divs.length; ++i) {
		if (divs[i].className == 'pmain') {
			win.document.write("<div class='pmain'>");
			win.document.write(divs[i].innerHTML);
			win.document.write('</div>');
		}
	}
	win.document.write('</body></html>');
}