function clickup() {
	if (document.upform.up.value) {
		document.upform.submit();
	} else {
		alert('你还没有选中上传文件吧,:)');
	}
}
