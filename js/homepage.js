$(function() {
	Store.init(location.pathname.replace(/[^\/]+\.htm/, ''));

	$('#id').focus();
	$('#sw').click(function() {
		$('#more').toggle();
		return false;
	});

	$('#login').click(function() {
		var $form = $('form');
		if ($('#id').val().length >= 2 && $('#pw').val().length >= 4) {
			$('#error').hide();
			$.ajax({
				type: 'POST',
				url: 'bbs/session-login.json',
				data: $form.serialize()
			}).done(function(data) {
				Session.onLoginSuccess(data);
				location.assign('forum/hot');
			}).fail(function(jqXHR) {
				if (jqXHR.status == 403) {
					$('#error').removeClass('hidden').show();
				}
			});
		}
		return false;
	});

	if (Store.get('session-logout')) {
		Store.clear();
	} else {
		$.getJSON('bbs/session-login.json', function(data) {
			if (data.code == 10002) {
				location.assign('forum/hot');
			}
		});
	}
});
