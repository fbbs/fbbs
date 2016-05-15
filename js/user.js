(function() {
	function layoutUserCard($div, $anchor) {
		var $window = $(window),
			scrollTop = $window.scrollTop(),
			divHeight = $div.outerHeight(),
			anchorHeight = $anchor.height(),
			anchorOffset = $anchor.offset(),

			topSpace = anchorOffset.top - scrollTop,
			botSpace = scrollTop + $window.height() - (anchorOffset.top + anchorHeight),

			coords = { left: anchorOffset.left };

		if (topSpace > botSpace) {
			coords.top = anchorOffset.top - divHeight;
		} else {
			coords.top = anchorOffset.top + anchorHeight;
		}
		$div.offset(coords);
	}

	App.hook('a.usr', function(e) {
		var hover, $e = $(e), $div;
		$e.hover(
			function() {
				hover = true;
				App.load('user', {
					name: $e.text(),
					s: 1
				}, function(data) {
					$div = $(App.render('user-card', data));
					if (hover) {
						$div.appendTo($('body'));
						layoutUserCard($div, $e);
						$div.fadeIn();
					}
				});
			},
			function() {
				hover = false;
				if ($div) {
					$div.fadeOut(400, function() {
						$div.remove();
					});
				}
			}
		);
		$e.click(function() {
			hover = false;
			if ($div)
				$div.remove();
		});
	});

	App.P({
		tmpl: 'user',
		m: {
			init: function(data) {
				data.login = new Date(+data.login).toISOString();
				if (data.logout)
					data.logout = new Date(+data.logout).toISOString();
				if (data.plan)
					data.plan = Post.parse(data.plan.split('\n'));
				this.data = data;
			}
		}
	});
})();
