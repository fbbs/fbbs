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
