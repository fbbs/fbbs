(function() {
	var Board = window.Board = {};

	Board.Sector = App.P({
		tmpl: 'board-sector',
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
			nav: 'board'
		}
	});

	Board.Toc = App.P({
		tmpl: 'board-toc',
		m: {
			convert: function(board_id) {
				return function(e) {
					e.stamp = Post.stamp(e.id).toISOString();
					e.board_id = board_id;
					e.title = Post.parseTitle(e.title);
				};
			},

			init: function(data) {
				if (data.board) {
					data.board.bms = data.board.bms.split(' ');
				}
				data.posts.forEach(this.convert(data.board.id));
				this.data = data;
			},

			append: function(posts) {
				posts.forEach(this.convert(this.data.board.id));
				$.merge(this.data.posts, posts);
			},

			min_id: function() {
				var p = this.data.posts,
					l = p.length;
				return l ? p[l - 1].id : 0;
			},

			count: function() {
				return this.data.posts.length;
			}
		},

		v: {
			append: function(posts) {
				$($.map(posts, function(p) {
					return App.partial('board-toc', p);
				}).join(''))
				.appendTo(this.$('#board-toc-list'))
				.hook();
			}
		},

		c: {
			nav: 'board',

			post: function() {
				Ui.loadMore({
					ctrl: this,
					count: 20,
					api: 'board-toc',
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
				$(window).off('scroll');
			}
		}
	});

	App.E.on('b:favorite', function() {
		var s = 'board-favorite',
			boards = Store.get(s);
		if (boards && boards.length > 0)
			boards.sort(function(a, b) { return a.name.localeCompare(b.name); });

		$('#' + s).html(App.render(s, {
			boards: boards
		}));
	});
})();
