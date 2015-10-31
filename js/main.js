(function() {
	App.hook('a', function(e) {
		var $e = $(e);
		if (e.href.startsWith(App.href) && !$e.hasClass('na')) {
			$e.click(function(evt) {
				var title = $e.attr('title') || $e.text();
				title += ' - ' + App.NAME;
				History.pushState(null, title, this.href);
				evt.preventDefault();
				return false;
			});
		}
	});

	$(function() {
		App.init();

		Ui.hideHidden();
		$('body').hook();

		App.E.on('statechange', function(evt) {
			App.loadPage();
		});

		$(window).bind('statechange', function() {
			App.E.fire('statechange');
		});

		Session.init();

		App.E.fire('statechange');
		App.E.fire('b:favorite');

		(function() {
			var $navHeaders = $('#nav-notification-header, #nav-board-header');
			$navHeaders.click(function() {
				$(this).parent().find('ul').slideToggle('fast');
			});
		 })();
	});
})();
