$(function() {
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
				data: $form.serialize(),
				success: function(data) {
					var expire = data['expire_time'];

					Store.clear();
					Store.set('session-user-name', data['user_name']);
					Store.set('session-key', data['session_key']);
					Store.set('session-token', data['token']);
					if (expire)
						Store.set('session-expire-time', expire);
					Session.updateLastActivity();

					Cookie.set('utmpkey', data['session_key'], expire ? new Date(expire * 1000) : 0, Cookie.abs('bbs/'));
					Cookie.set('utmpuser', data['user_name'], expire ? new Date(expire * 1000) : 0, Cookie.abs('bbs/'));
					location.assign('alpha/home');
				}
			});
		}
		return false;
	});

	if (Store.get('session-logout')) {
		Store.remove('session-logout');
		Cookie.remove('utmpkey', Cookie.abs('bbs/'));
		Cookie.remove('utmpuser', Cookie.abs('bbs/'));
	} else {
		$.getJSON('bbs/login.json', function(data) {
			if (data.code == 10002) {
				location.assign('alpha/home');
			}
		});
	}
});
