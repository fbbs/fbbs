(function() {
	var KEEP_ALIVE = 15 * 60 * 1000;

	window.Session = {
		checkLoginStatus: function() {
			var last = Store.get('session-last-activity'),
				current = (new Date()).getTime();
			App.E.fire('s:login', !!(last && current - last < KEEP_ALIVE));
		},

		init: function() {
			App.E.on('s:login', function(evt, loggedIn) {
				$('#nav-personal>li').each(function() {
					var t = $(this);
					t.hasClass('require-login') == loggedIn ? t.show() : t.hide();
				});
			});
			this.checkLoginStatus();

			$('.session-logout').click(function() {
				$.post('../bbs/session-logout.json',
					{ token: Store.get('session-token') },
					function(data) {
						Store.clear();
						Store.set('session-logout', true);
						location.assign('..');
					}
				);
				return false;
			});
		},

		updateLastActivity: function() {
			Store.set('session-last-activity', (new Date()).getTime());
		}
	};
})();
