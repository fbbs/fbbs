;(function($, undefined) {
	$.fn.extend({
		ajaxify: function() {
			this.find('a').each(function() {
				var $this = $(this),
					href = $this.attr('href');

				if (href && href.indexOf('/') === -1 && !$this.hasClass('no-ajaxify')) {
					$this.click(function(evt) {
						var title = $this.attr('title') || $this.text();
						title += ' - 日月光华'
						History.pushState(null, title, href);
						evt.preventDefault();
						return false;
					});
				}
			});
		}
	});

	function splitUrl(url) {
		var start, result = { param: '' },
			qmark = url.indexOf('?');
		if (qmark >= 0) {
			result.param = url.substring(qmark);
			url = url.substring(0, qmark);
		}
		start = url.lastIndexOf('/');
		result.path = url.substring(0, start + 1);
		result.base = url.substring(start + 1);
		return result;
	}

	$(function() {
		var $window = $(window),
			$body = $(document.body);

		$body.ajaxify();

		Ui.hideHidden();

		function loadUrl(url) {
			$.ajax({
				url: '../bbs/' + url.base + '.json' + url.param,
				dataType: 'json',
				success: function(data, textStatus, jqXHR) {
					var tmpl = Template.get(url.base, url.param), $main = $('#main');
					if (tmpl) {
						if (tmpl.pre) {
							data = tmpl.pre(data);
						}
					}
					if (tmpl && tmpl.render) {
						tmpl.render($main, data);
					} else {
						var temp = '#t-' + url.base;
						$main.html(Mustache.render($(temp).text(), data));
					}
					if (tmpl && tmpl.post) {
						tmpl.post(data);
					} else {
					}
					$main.ajaxify();
					$('footer').show();
					Session.updateLastActivity();
				},
				error: function(jqXHR, textStatus, errorThrown) {
					return false;
				}
			});
		}

		function showPersonalNavigation(loggedIn) {
			$('#nav-personal li').each(function() {
				var $this = $(this);
				if ($this.hasClass('require-login') !== loggedIn) {
					$this.hide();
				}
			});
		}

		function checkLoginStatus() {
			var last = Store.get('session-last-activity'),
				current = (new Date()).getTime();
			showPersonalNavigation(!!(last && current - last < Session.keepAlive));
		}

		$window.bind('statechange', function() {
			loadUrl(splitUrl(History.getState().url));
		});

		(function() {
			var $navHeaders = $('#nav-notification-header, #nav-board-header');
			$navHeaders.click(function() {
				$(this).parent().find('ul').slideToggle('fast');
			});
		 })();

		loadUrl(splitUrl(location.href));
		checkLoginStatus();

		Session.register();
	});
})(jQuery);
