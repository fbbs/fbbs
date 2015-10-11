(function() {
	window.Ui = {
		showNavBoard: function(show) {
			var $navBoard = $('#nav-board'),
				$main = $('#main'),
				cls = 'nav-left-shown';
			if (show) {
				$navBoard.show();
				$main.addClass(cls);
			} else {
				$navBoard.hide();
				$main.removeClass(cls);
			}
		},
		hideHidden: function() {
			$('.hidden').removeClass('hidden').hide();
		}
	};
})();
