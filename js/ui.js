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
		},

		loadMore: function(options) {
			var ctrl = options.ctrl,
				$load = ctrl.view.$('.load-more'),
				clickable = true,
				model = ctrl.model, view = ctrl.view;

			if (model.count() < options.count) {
				$load.hide();
				return;
			}

			$load.text('↓ 载入更多');
			$load.click(function() {
				if (clickable) {
					$load.text('载入中…');
					App.load(options.api, options.param(model),
						function(data) {
							var list = options.done(data);
							if (list.length > 0) {
								model.append(list);
								view.append(list);
							}
							if (list.length < options.count) {
								$load.text('已全部载入');
							} else {
								$load.text('↓ 载入更多');
								clickable = true;
							}
						}, function(jqXHR, textStatus, errorThrown) {
							$load.text('载入失败，点击重试');
							clickable = true;
							if (options.fail) {
								options.fail(jqXHR, textStatus, errorThrown);
						}
					});
					clickable = false;
				}
			});
		}
	};
})();
