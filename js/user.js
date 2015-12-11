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
				if ($div) {
					$div.fadeIn();
					layoutUserCard($div, $e);
				} else {
					App.load('user', {
						name: $e.text(),
						s: 1
					}, function(data) {
						$div = $(App.render('user-card', data));
						if (hover) {
							$div.appendTo($e.parent());
							layoutUserCard($div, $e);
							$div.fadeIn();
						}
					});
				}
			},
			function() {
				hover = false;
				if ($div)
					$div.fadeOut();
			}
		);
	});
})();
