(function() {
	var jQueryExtend = $.extend;

	var controllers = {},
		currentController,
		hooks = [],
		tokenFunc;

	var createLink = function(href) {
		var a = document.createElement('a');
		a.href = href;

		var p = a.pathname;
		if (!p.startsWith('/'))
			p = '/' + p;
		a.path = p;
		return a;
	}

	var App = window.App = {
		NAME: '日月光华',

		DOMAINS: [ 'bbs.fudan.edu.cn', 'bbs.fudan.sh.cn', '61.129.42.9', '202.120.225.9' ],

		render: function(tmpl, data) {
			return Mustache.render($('#t-' + tmpl).text(), data, this.partials);
		},

		partial: function(tmpl, data) {
			return Mustache.render($('#p-' + tmpl).text(), data, this.partials);
		},

		partials: {},

		init: function() {
			var a = createLink(location.href + '/..');
			this.pathname = a.path;
			this.href = a.href;

			Store.init(createLink(a.href + '../').path);

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

		api: function(api, params) {
			var url = this.pathname + '../bbs/' + api + '.json',
				t = typeof params;
			if (t == 'object')
				url += '?' + $.param(params);
			else if (t == 'string')
				url += params;
			return url;
		},

		token: function(func) {
			if (func)
				tokenFunc = func;
			else
				return tokenFunc();
		},

		ajax: function(settings) {
			settings.data = settings.data || [];
			$.merge(settings.data, [ { name: 'token', value: tokenFunc() } ]);
			return $.ajax(settings);
		},

		load: function(api, param, done, fail) {
			return $.ajax({
				url: this.api(api, param),
				dataType: 'json'
			}).done(done).fail(fail);
		},

		loadPage: function() {
			var url = History.getState().url,
				a = createLink(url),
				api = a.path.replace(this.pathname, ''),
				ctrlClass = App.getC(api);

			if (currentController && currentController.leave) {
				currentController.leave();
			}

			if (!ctrlClass)
				return;
			var controller = currentController = new ctrlClass();

			if (controller.enter && controller.enter(a.search) === false)
				return;

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
		},

		normalize: function(href) {
			var a = createLink(href);
			if ($.inArray(a.hostname, this.DOMAINS) >= 0) {
				return a.path + a.search + a.hash;
			}
		}
	};

	$.fn.extend({
		hook: function() {
			return this.each(function() {
				var $this = $(this);
				hooks.forEach(function(i) {
					$this.find(i[0]).each(function(j, e) {
						i[1](e);
					});
				});
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
				ready: function(data, skip) {
					var model = new Model();
					this.model = model;
					model.init(data, skip);
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
