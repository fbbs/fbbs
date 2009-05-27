function closewin()
{
	opener.document.postform.text.value += "\n" + document.getElementById('url').innerHTML + "\n";
	return window.close();
}