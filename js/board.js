(function() {
	var BOARD_FAVORITE_KEY = 'favorite';

	var Board = window.Board = {};

	var favorite = {
		has: function(id) {
			var boards = Store.get(BOARD_FAVORITE_KEY);
			return ($.grep(boards, function(e) {
				return e.id == id;
			})).length > 0;
		},

		toggleButton: function(el, added) {
			el.toggleClass('fav-add', !added)
				.toggleClass('fav-rm', added);
		},

		setupButton: function(el, id, name) {
			var t = this;
			el.click(function() {
				var $this = $(this),
					rm = $this.hasClass('fav-rm');
				$this.prop('disabled', true);
				App.ajax({
					type: rm ? 'DELETE' : 'POST',
					url: App.api('favorite') + '?id=' + id
				}).done(function() {
					App.E.fire('b:favorite', rm ? 'rm' : 'add', rm ? id : { id: id, name: name });
				}).always(function() {
					$this.prop('disabled', false);
				});
				return false;
			});
			var callback = function() {
				t.toggleButton(el, t.has(id));
			};
			callback();
			App.E.on('b:favorite', callback);
			return callback;
		}
	};

	Board.Sector = App.P({
		tmpl: 'sector',
		m: {
			init: function(data) {
				/*
				 * { sectors: [ { name, descr, boards: [ { id, name, descr } ... ] } ... ] }
				 */
				var result = { sectors: [] }, boards;
				data.sectors.sort(function(a, b) { return a.name.localeCompare(b.name); });
				data.sectors.forEach(function(sector) {
					boards = [];
					data.boards.forEach(function(b) {
						if (b.sector_id == sector.id) {
							boards.push({ id: b.id, name: b.name, descr: b.descr });
						}
					});
					boards.sort(function(a, b) { return a.name.localeCompare(b.name); });
					result.sectors.push({
						name: sector.name,
						descr: sector.descr + ' - [' + sector.short_descr + ']',
						boards: boards
					});
				});
				this.data = result;
			}
		},

		c: {
			nav: 'board',

			post: function() {
				if (this.model.data.sectors.length <= 1) {
					var c = 'column-md', view = this.view;
					view.$('#sct').removeClass(c);
					view.$('ul').addClass(c);
					view.$('h1~a').hide();
				}
			}
		}
	});

	Board.Toc = App.P({
		tmpl: 'board',
		m: {
			convert: function(board_id) {
				return function(e) {
					e.date = Post.date(e.id).toISOString();
					e.board_id = board_id;
					e.title = Post.titleHtml(e.title);
				};
			},

			init: function(data, skip) {
				if (!skip) {
					if (data.board) {
						data.board.bms = data.board.bms.split(' ');
					}
					data.posts.forEach(this.convert(data.board.id));
				}
				this.data = data;
			},

			prepend: function(post) {
				var data = this.data, sticky = 0;
				this.convert(data.board.id)(post);
				$.each(data.posts, function(i, p) {
					if (!p.sticky) {
						sticky = i;
						return false;
					}
				});
				data.posts.splice(sticky, 0, post);
				return sticky;
			},

			append: function(posts) {
				posts.forEach(this.convert(this.data.board.id));
				$.merge(this.data.posts, posts);
				return posts;
			},

			min_id: function() {
				var p = this.data.posts,
					l = p.length;
				return l ? p[l - 1].id : 0;
			},

			count: function() {
				return this.data.posts.length;
			},

			clear: function() {
				this.data.posts.forEach(function(p) {
					delete p.unread;
				});
			}
		},

		v: {
			prepend: function(post, pos) {
				var $el = $(App.partial('board', post)),
					$list = this.$('#brd-li').find('li');
				$($list.get(pos)).before($el);
				$el.hide().slideDown().hook();
			},

			append: function(posts) {
				$($.map(posts, function(p) {
					return App.partial('board', p);
				}).join(''))
				.appendTo(this.$('#brd-li'))
				.hook();
			},

			clear: function() {
				var c = 'pst-unread';
				this.$('li.' + c).each(function() {
					$(this).removeClass(c);
				});
			}
		},

		c: {
			nav: 'board',

			post: function() {
				var view = this.view,
					model = this.model,
					board = model.data.board;
				this.callback = favorite.setupButton(
					this.view.$('.fav-btn'),
					board.id, board.name);

				view.$('.pst-new-btn').click(function() {
					var $f = view.$('.pst-new'),
						$main = view.$('#brd-main'),
						$links = view.$('#brd-links'),
						cancel = function() {
							$main.show();
							$links.show();
							$f.remove();
							return false;
						};

					if ($f.length == 0) {
						$f = Post.setupForm({
							anonymous: !!board.anonymous,
							boardId: board.id,
							replyId: 0,
							done: function(data) {
								cancel();
								var post = {
									id: data.id,
									title: $f.find('[name=title]').val(),
									user_name: Store.get('session-user-name')
								};
								view.prepend(post, model.prepend(post));
							},
							cancel: cancel
						});
						$f.insertBefore($main).show();
						$f.find('[name=title]').focus();
						$main.hide();
						$links.hide();
					}
					return false;
				});

				view.$('#brd-clear').click(function() {
					App.ajax({
						url: App.api('clear', { id: board.id }),
						type: 'POST'
					}).done(function() {
						model.clear();
						view.clear();
					});
					return false;
				});

				Ui.loadMore({
					ctrl: this,
					count: 20,
					api: 'board',
					param: function(model) {
						return {
							id: model.data.board.id,
							max_id: model.min_id()
						}
					},
					done: function(data) {
						return data.posts;
					}
				});
			},

			leave: function() {
				var $window = $(window);
				$window.off('scroll');
				App.E.off('b:favorite', this.callback);

				App.S.set('board-toc', {
					data: this.model.data,
					scrollTop: $window.scrollTop()
				});
			},

			enter: function(search) {
				var cache = App.S.get('board-toc'),
					params = $.deparam(search);
				if (cache && params && cache.data.board.id == params.id) {
					var rc = Post.ReadCache.remove();
					if (rc) {
						cache.data.posts.forEach(function(p) {
							if (p.id in rc) {
								delete p.unread;
							}
						});
					}
					this.ready(cache.data, true);
					this.defaultPost();
					this.post();
					window.scrollTo(0, cache.scrollTop);
					return false;
				}
			}
		}
	});

	Board.Hot = App.P({
		tmpl: 'trend',

		m: {
			init: function(data) {
				data.posts.forEach(function(obj) {
					obj.title = Post.titleHtml(obj.title);
				});
				this.data = data;
			}
		},

		c: {
			nav: 'board'
		}
	});

	App.E.on('b:favorite', function(evt, action, data) {
		var s = BOARD_FAVORITE_KEY,
			boards = Store.get(s) || [];

		if (action == 'set') {
			boards = data;
		} else if (action == 'add') {
			$.merge(boards, [data]);
		} else if (action == 'rm') {
			boards = $.grep(boards, function(b, i) { return b.id != data });
		}

		if (boards && boards.length > 0)
			boards.sort(function(a, b) { return a.name.localeCompare(b.name); });
		Store.set(s, boards);

		$('#fav').html(App.render(s, {
			boards: boards
		})).hook();
	});
})();
