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
			post: function() {
				Ui.showNavBoard(true);
			}
		}
	});

	Board.Toc = App.P({
		tmpl: 'board-toc',
		m: {
			init: function(data) {
				var f = function(e) {
					e.stamp = Post.stamp(e.id).format();
					e.board_id = data.board.id;
				};
				if (data.board) {
					data.board.bms = data.board.bms.split(' ');
				}
				data.posts.forEach(f);
				this.data = data;
			}
		},
	});
})();
