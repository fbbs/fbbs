(function() {
	var parseTitle = function(t) {
		return t.escapeHtml()
			.replace(/\x1b\[1;3\dm\[(.+?)\]\x1b\[m/, "<span class='post-tag'>$1</span>")
			.replace(/^\[(转载|合集)\]/, "<span class='post-tag'>$1</span>");
	};

	var parseHeader = function(lines) {
		var R1 = /发信人: ([^ ]+) \((.*)\), 信区: (.+)$/,
			R2 = /(\d\d\d\d)年(\d\d)月(\d\d)日(\d\d):(\d\d):(\d\d)/,
			R3 = /\((.+)\)/,
			r = {}, m;

		m = lines[0].match(R1);
		if (m) {
			r.user_name = m[1];
			r.user_nick = m[2];
			r.board_name = m[3];
		}
		m = lines[2].match(R2);
		if (m)
			r.date = new Date(m[1], m[2] - 1, m[3], m[4], m[5], m[6]);
		else {
			m = lines[2].match(R3);
			if (m)
				r.date = new Date(m[1]);
		}
		r.title = parseTitle(lines[1].substring(6));
		return r;
	}

	window.Post = {
		stamp: function(id) {
			return new Date((new Long(id)).rshift(21).toInteger());
		},
		makeId: function(stamp) {
			return (new Long(stamp)).lshift(21).toString();
		},

		_State: function() {
			this.lines = [];
			this.quote = false;
			this.footer = false;
			this.setQuote = function(quote) {
				if (this.quote != quote) {
					this.lines.push(quote ? '<blockquote>' : '</blockquote>');
					this.quote = quote;
				}
			};
			this.setFooter = function(footer) {
				if (this.footer != footer) {
					if (this.quote)
						this.setQuote(false);
					this.lines.push(footer ? '<footer>' : '</footer>');
					this.footer = footer;
				}
			};
			this.parseLine = function(l) {
				var hl = false, fg = 37, bg = 40, s = 0, closed = true,
					m = l.match(/(\x1b\[[\d;]*\w|https?:\/\/[\w\d\-\._~:\/\?#\[\]@!\$&'\(\)\*\+,;=%]+)/g),
					lines = [];

				if (m) {
					m.forEach(function(item) {
						var b = l.indexOf(item, s), sm;
						if (b > s)
							lines.push(l.substring(s, b).escapeHtml());
						if (item.startsWith('\x1b')) {
							sm = item.match(/(\d+)/g);
							if (sm) {
								sm.forEach(function(d) {
									if (d == 0 || d == 1) hl = d;
									if (d >= 30 && d <= 37) fg = d;
									if (d >= 40 && d <= 48) bg = d;
								});
							}
							if (!closed)
								lines.push('</span>');
							lines.push('<span class="a' + (+hl) + fg + ' a' + bg + '">');
							closed = false;
						} else if (item.startsWith('http')) {
							if (item.match('/jpg|jpeg|png|gif/i')) {
								lines.push('<img src="' + encodeURI(item) + '"/>');
							} else {
								lines.push('<a href="' + encodeURI(item) + '">' + item.escapeHtml() + '</a>');
							}
						}
						s = b + item.length;
					});
				}
				if (s < l.length)
					lines.push(l.substring(s).escapeHtml());
				if (!closed)
					lines.push('</span>');
				return lines;
			};
		},

		parse: function(s) {
			var lines = s.split('\n'), s = new this._State(), r = {};
			if (lines.length > 4) {
				r = parseHeader(lines);
				lines = lines.slice(4);
			}

			$.each(lines, function(i, l) {
				if (!s.footer) {
					if (l == '--') {
						s.setFooter(true);
						return;
					}
					s.setQuote(l.startsWith('【 在 ') || l.startsWith(': '));
				}
				s.lines.push('<p>');
				$.merge(s.lines, s.parseLine(l));
				s.lines.push('</p>');
			});
			s.setQuote(false);
			s.setFooter(false);
			r.parsed = s.lines.join('');
			return r;
		},

		parseTitle: parseTitle
	};

	Post.Content = App.P({
		tmpl: 'post-content',
		m: {
			init: function(data) {
				data.posts.forEach(function(p) {
					$.extend(p, Post.parse(p.content));
				});
				this.data = data;
			}
		},
		c: {
			nav: 'board',
			post: function() {
			}
		}
	});
})();
