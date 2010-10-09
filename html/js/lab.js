var transformer = null;

$(document).ready(function() {
	$.get('../xsl/lab.xsl', function(xsl) {
		if (typeof XSLTProcessor != 'undefined') {
			transformer = new XSLTProcessor();
			transformer.importStylesheet(xsl);
		} else if ('transformNode' in xsl) {
			transformer = xsl;
		} else {
			throw "xslt not supported";
		}
	});
	$('a').click(load);
});

function load() {
	var link = $(this).attr('href');
	if (!link.match(/^(?:http|\.\.)/)) {
		location.hash = encodeURI(link);
		link = 'bbs/' + link;
	}
	$('#status').removeClass('st_err').addClass('st_load').html('<img src="images/indicator.gif"/>Loading').fadeIn('fast');
	$.get(link, function(data) {
		if (typeof data == 'object') {
			$('#main').empty().append(xslt(data));
			$('#main a').click(load);
			$('#status').hide();
		} else {
			$('#status').addClass('st_err').html($(data).filter('div').text());
		}
		$('#status').removeClass('st_load');
	});
	return false;
}

function xslt(xml)
{
	if (typeof XSLTProcessor != 'undefined') {
		return transformer.transformToFragment(xml, document);
	} else if ('transformNode' in xml) {
		return xml.transformNode(transformer);
	} else {
		throw "xslt not supported";
	}
}
