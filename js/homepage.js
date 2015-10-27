$(function() {
	Store.init(location.pathname);

	$('#id').focus();
	$('#sw').click(function() {
		$('#more').toggle();
		return false;
	});

	$('#login').click(function() {
		var $form = $('form');
		if ($('#id').val().length >= 2 && $('#pw').val().length >= 4) {
			$.ajax({
				type: 'POST',
				url: $form.attr('action'),
				data: $form.serialize()
			}).done(function(data) {
				Session.onLoginSuccess(data);
				location.assign('alpha/home');
			});
		}
		return false;
	});

	if (Store.get('session-logout')) {
		Store.clear();
	} else {
		$.getJSON('bbs/login.json', function(data) {
			if (data.code == 10002) {
				location.assign('alpha/home');
			}
		});
	}
});
