(function() {
	var jQueryExtend = $.extend;

	var controllers = {},
		currentController,
		hooks = [],
		tokenFunc;

	var createLink = function(href) {
		var a = document.createElement('a');
		a.href = href;
		return a;
	}

	var App = window.App = {
		NAME: '日月光华',

		render: function(tmpl, data) {
			return Mustache.render($('#t-' + tmpl).text(), data, this.partials);
		},

		partial: function(tmpl, data) {
			return Mustache.render($('#p-' + tmpl).text(), data, this.partials);
		},

		partials: {},

		init: function() {
			var a = createLink(location.pathname + '/..');
			this.pathname = a.pathname;
			this.href = a.href;

			Store.init(createLink(a.pathname + '../').pathname);

			var p = this.partials;
			$('[id^=p-]').each(function() {
				var $t = $(this);
				p[$t.attr('id').substring(2)] = $t.text();
			});
		},

		reg: function(name, controller) {
			controllers[name] = controller;
		},

		getC: function(name) {
			return controllers[name];
		},

		api: function(api) {
			return this.pathname + '../bbs/' + api + '.json';
		},

		token: function(func) {
			if (func)
				tokenFunc = func;
			else
				return tokenFunc();
		},

		postForm: function(settings) {
			settings.data = $.merge(settings.data, [ { name: 'token', value: tokenFunc() } ]);
			return $.ajax(settings);
		},

		load: function(api, param, done, fail) {
			if (typeof(param) === 'object')
				param = '?' + $.param(param);
			return $.ajax({
				url: this.api(api) + param,
				dataType: 'json'
			}).done(done).fail(fail);
		},

		loadPage: function() {
			var url = History.getState().url,
				a = createLink(url),
				api = a.pathname.replace(this.pathname, ''),
				ctrlClass = App.getC(api);

			if (currentController && currentController.leave) {
				currentController.leave();
			}

			if (!ctrlClass)
				return;
			var controller = currentController = new ctrlClass();

			this.load(api, a.search, function(data) {
				window.scrollTo(0, 0);
				if (url === History.getState().url) {
					controller.ready(data);
					controller.defaultPost();
					if (controller.post)
						controller.post();
				}
			},
			function(jqXHR, textStatus, errorThrown) {
				if (url === History.getState().url) {
					controller.error(jqXHR, textStatus, errorThrown);
				}
			});
		},

		hook: function(selector, func) {
			hooks.push([selector, func]);
		}
	};

	$.fn.extend({
		hook: function() {
			var $this = this;
			hooks.forEach(function(i) {
				$this.find(i[0]).each(function(j, e) {
					i[1](e);
				});
			});
		},

		selectRange: function(b, e) {
			return this.each(function() {
				if (this.setSelectionRange) {
					this.setSelectionRange(b, e);
				} else if (this.createTextRange) {
					var range = this.createTextRange();
					range.collapse(true);
					range.moveEnd('character', b);
					range.moveStart('character', e);
					range.select();
				}
			});
		}
	});

	$.extend({
		deparam: function(s) {
			if (s.startsWith('?'))
				s = s.substring(1);
			var a = s.split('&'),
				r = {};
			a.forEach(function(e) {
				var p = e.split('=');
				r[p[0]] = p[1];
			});
			return r;
		}
	});

	var Model = App.M = function(options) {
		jQueryExtend(this, options);
	};

	jQueryExtend(Model.prototype, {
		init: function(data) {
			this.data = data;
		},
		set: function(data) {
			this.data = data;
		},
		get: function(s) {
			var d;
			if (s) {
				d = this.data;
				s = s.split('.');
				s.forEach(function(i) {
					d = d[i];
				});
			} else {
				return this.data;
			}
		}
	});

	var View = App.V = function(options) {
		jQueryExtend(this, options);
		this._init();
		this.init();
	};

	jQueryExtend(View.prototype, {
		tag: 'div',

		init: function() {},

		$: function(selector) {
			return this.$el.find(selector);
		},

		render: function() {
			if (this.tmpl && this.model) {
				this.$el.html(App.render(this.tmpl, this.model.get()));
			}
			return this;
		},

		_init: function() {
			var t = this;
			if (t.el) {
				t.$el = t.el instanceof $ ? t.el : $(t.el);
			} else {
				t.$el = $(document.createElement(t.tag));
				if (t.id)
					t.$el.attr('id', t.id);
				if (t.cls)
					t.$el.attr('class', t.cls);
			}
		}
	});

	var Controller = App.C = function(options) {
		jQueryExtend(this, options);
	};

	jQueryExtend(Controller.prototype, {
		ready: function(data) {},
		defaultPost: function() {
			this.view.$el.hook();
			Ui.showNavBoard(this.nav == 'board');
		},
		error: function() {}
	});

	var listeners = {};
	App.E = {
		on: function(evt, callback) {
			if (!listeners[evt]) {
				listeners[evt] = [];
			}
			var list = listeners[evt];
			list.push(callback);
		},
		off: function(evt, callback) {
			if (callback) {
				var list = listeners[evt];
				if (list) {
					list = $.grep(list, function(c, i) {
						return c !== callback;
					});
				}
			} else {
				delete listeners[evt];
			}
		},
		fire: function(evt) {
			var list = listeners[evt];
			if (list) {
				var args = arguments;
				list.forEach(function(f) {
					f.apply(this, args);
				});
			}
		}
	};

	var extend = function(props) {
		var parent = this;
		var child = function() {
			parent.apply(this, arguments);
		};

		var p = child.prototype = Object.create(parent.prototype);
		jQueryExtend(p, props);
		p.constructor = child;

		child._super = parent.prototype;
		return child;
	};

	Model.ext = View.ext = Controller.ext = extend;

	App.P = function(options) {
		var tmpl = options.tmpl,
			Model = App.M.ext(options.m),
			View = App.V.ext(jQueryExtend({
				el: '#main'
			}, options.v));
		var r = {
			M: Model,
			V: View,
			C: App.C.ext(jQueryExtend({
				ready: function(data) {
					var model = new Model();
					this.model = model;
					model.init(data);
					this.view = new View({
						model: model,
						tmpl: tmpl
					});
					this.view.render();
					$('footer').show();
					Session.updateLastActivity();
				}
			}, options.c))
		};
		App.reg(tmpl, r.C);
		return r;
	};
})();
