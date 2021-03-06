(function() {
	window.Ui = {
		showNavBoard: function(show) {
			var $navBoard = $('#nav-brd'),
				$main = $('#main'),
				cls = 'nav-left-shown';
			if (show) {
				$navBoard.addClass(cls);
				$main.addClass(cls);
			} else {
				$navBoard.removeClass(cls);
				$main.removeClass(cls);
			}
		},

		hideHidden: function() {
			$('.hidden').removeClass('hidden').hide();
		},

		loadMore: function(options) {
			var ctrl = options.ctrl,
				$load = ctrl.view.$('.load-more'),
				clickable = true, autoload = options.autoload,
				retries = 0, autoclick = true,
				model = ctrl.model, view = ctrl.view,

				loadTop = $load.offset().top,
				callback = function() {
					if (autoclick) {
						var $t = $(this);
						var t = $t.scrollTop() + $t.height();
						if (t > loadTop) {
							$load.click();
						}
					}
				},
				end = function($el, done) {
					autoload = clickable = autoclick = !done;
					if (done) {
						if (options.retry) {
							$el.text(options.retry(retries++));
							clickable = true;
						} else {
							$el.hide();
						}
					} else {
						$el.text('↓ 载入更多');
					}
				};

			end($load, model.count() < options.count);

			if (autoload) {
				$(window).on('scroll', callback);
			}

			$load.click(function() {
				if (clickable) {
					clickable = false;
					if (!autoload) {
						$(window).on('scroll', callback);
						autoload = true;
					}
					$load.text('载入中…');
					App.load(options.api, options.param(model),
						function(data) {
							var list = options.done(data);
							if (list.length > 0) {
								list = model.append(list);
								view.append(list);
								loadTop = $load.offset().top;
							}
							end($load, list.length < options.count);
						}, function(jqXHR, textStatus, errorThrown) {
							$load.text('载入失败，点击重试');
							clickable = true;
							if (options.fail) {
								options.fail(jqXHR, textStatus, errorThrown);
						}
					});
				}
			});
		}
	};
})();
