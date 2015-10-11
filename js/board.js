(function() {
	var T = Template;
	T.set('board-sector', {
		/*
		 * { sectors: [ { name, descr, boards: [ { id, name, descr } ... ] } ... ] }
		 */
		pre: function(data) {
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
			return result;
		},
		post: function() {
			Ui.showNavBoard(true);
		}
	});

	T.set('board-toc', {
		pre: function(data) {
			var f = function(e) {
				e.stamp = Post.stamp(e.id).format();
				e.board_id = data.board.id;
			};
			if (data.board) {
				data.board.bms = data.board.bms.split(' ');
			}
			data.sticky.forEach(f);
			data.posts.forEach(f);
			return data;
		},
		render: function(elem, data) {
			var t1 = $('#t-board-toc').text(), t2 = $('#t-board-toc-list').text(), $ul;
			elem.html(Mustache.render(t1, data));
			$ul = elem.find('ul');
			if (data.sticky)
				$ul.append(Mustache.render(t2, { posts: data.sticky }));
			$ul.append(Mustache.render(t2, data));
		},
	});
})();
