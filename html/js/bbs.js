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