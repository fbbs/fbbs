(function() {
	$.fn.extend({
		ajaxify: function() {
			this.find('a').each(function() {
				var $this = $(this);
				if (this.href.startsWith(App.href) && !$this.hasClass('na')) {
					$this.click(function(evt) {
						var title = $this.attr('title') || $this.text();
						title += ' - ' + App.NAME;
						History.pushState(null, title, this.href);
						evt.preventDefault();
						return false;
					});
				}
			});
		}
	});

	$(function() {
		App.init();

		Ui.hideHidden();
		$('body').ajaxify();

		App.E.on('statechange', function(evt) {
			App.load();
		});

		$(window).bind('statechange', function() {
			App.E.fire('statechange');
		});

		Session.init();

		App.E.fire('statechange');

		(function() {
			var $navHeaders = $('#nav-notification-header, #nav-board-header');
			$navHeaders.click(function() {
				$(this).parent().find('ul').slideToggle('fast');
			});
		 })();
	});
})();
