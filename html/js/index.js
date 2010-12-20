$(document).ready(function() {
	$('#sw').hover(
		function() { $('#software').show(); },
		function() { $('#software').hide(); }
	);
	$('#guest').click(function() {
		window.location='bbs/sec';
		return true;
	})
	$('#id').focus();
});
