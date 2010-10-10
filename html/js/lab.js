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
	$('a').click(changeHash);
	load();
	hashChange(load);
});

function changeHash() {
	var link = $(this).attr('href');
	if (!link.match(/^(?:http|\.\.)/)) {
		location.hash = encodeURI(link);
		return false;
	} else {
		return true;
	}
}

function load() {
	var link = decodeURI(location.hash).substr(1);
	if (link.length == 0 || link == $('#st_link').html())
		return;
	$('#status').removeClass('st_err').addClass('st_load').html('<img src="images/indicator.gif"/>Loading').fadeIn('fast');
	$('#st_link').html(link);
	$.get('bbs/' + link, function(data) {
		if (typeof data == 'object') {
			$('#main').empty().append(xslt(data));
			$('#main a').click(changeHash);
			$('#status').hide();
		} else {
			$('#status').addClass('st_err').html($(data).filter('div').text());
		}
		$('#status').removeClass('st_load');
	});
}

function xslt(xml) {
	if (typeof XSLTProcessor != 'undefined') {
		return transformer.transformToFragment(xml, document);
	} else if ('transformNode' in xml) {
		return xml.transformNode(transformer);
	} else {
		throw "xslt not supported";
	}
}

function hashChange(handler) {
	if ('onhashchange' in window) {
		window.onhashchange = function() {
			load();
		};
	}
}
