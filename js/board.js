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
			}
		},

		v: {
			append: function(posts) {
				$($.map(posts, function(p) {
					return App.partial('board-toc-list', p);
				}).join(''))
				.appendTo(this.$('#board-toc-list'))
				.hook();
			}
		},

		c: {
			nav: 'board',

			post: function() {
				var $load = this.view.$('.load-more'),
					clickable = true,
					model = this.model, view = this.view,
					count = 20;

				if (model.data.posts.length < count) {
					$load.hide();
					return;
				}

				$load.text('↓ 载入更多');
				$load.click(function() {
					if (clickable) {
						$load.text('载入中…');
						App.load('board-toc', {
							id: model.data.board.id,
							max_id: model.min_id()
						}, function(data) {
							var posts = data.posts;
							if (posts.length > 0) {
								model.append(posts);
								view.append(posts);
							}
							if (posts.length < count) {
								$load.text('已全部载入');
							} else {
								$load.text('↓ 载入更多');
								clickable = true;
							}
						}, function(jqXHR, textStatus, errorThrown) {
							$load.text('载入失败，点击重试');
							clickable = true;
						});
						clickable = false;
					}
				});
			}
		}
	});
})();
