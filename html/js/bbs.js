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