(function() {
	window.Session = {
		keepAlive: 15 * 60 * 1000,

		register: function() {
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
