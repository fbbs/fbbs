(function() {
	var QUOTE_LINE_MAX = 5,
		QUOTE_WIDTH_MAX = 140,
		TRUNCATE_WIDTH = 76,
		QUOTE_TEMPLATE = '【 在 {{{user_name}}} ({{{user_nick}}}) 的大作中提到: 】',
		OMIT_STRING = ': .................（以下省略）';

	function parseHeader(lines) {
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
		r.title = lines[1].substring(6);
		return r;
	}

	function generateLink(href, img) {
		var link = App.normalize(href);
		if (img && /(jpg|jpeg|png|gif)$/i.test(href) && link) {
			link = encodeURI(link);
			return '<img src="' + link + '" alt="' + link + '">';
		}
		link = link || href;
		return '<a href="' + encodeURI(link) + '">' + Util.escape(href) + '</a>';
	}

	var State = function() {
		$.extend(this, {
			lines: [],
			quote: false,
			footer: false
		});
	};

	$.extend(State.prototype, {
		setQuote: function(quote) {
			if (this.quote != quote) {
				this.lines.push(quote ? '<blockquote>' : '</blockquote>');
				this.quote = quote;
			}
		},

		setFooter: function(footer) {
			if (this.footer != footer) {
				if (this.quote)
					this.setQuote(false);
				this.lines.push(footer ? '<footer>' : '</footer>');
				this.footer = footer;
			}
		},

		parseLine: function(l) {
			var hl = false, fg = 37, bg = 40, s = 0, closed = true,
				m = l.match(/(\x1b\[[\d;]*\w|https?:\/\/[\w\d\-\._~:\/\?#\[\]@!\$&'\(\)\*\+,;=%]+)/g),
				lines = [], img = !this.quote && !this.footer;

			if (m) {
				m.forEach(function(item) {
					var b = l.indexOf(item, s), sm;
					if (b > s)
						lines.push(Util.escape(l.substring(s, b)));
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
						lines.push(generateLink(item, img))
					}
					s = b + item.length;
				});
			}
			if (s < l.length)
				lines.push(Util.escape(l.substring(s)));
			if (!closed)
				lines.push('</span>');
			return lines;
		}
	});

	var Post = window.Post = {
		stamp: function(id) {
			return (new Util.Long(id)).rshift(21).toInteger();
		},

		date: function(id) {
			return new Date(this.stamp(id));
		},

		makeId: function(stamp) {
			return (new Util.Long(stamp)).lshift(21).toString();
		},

		titleHtml: function(title) {
			if (title) {
				return Util.escape(title)
						.replace(/\x1b\[1(?:;3\d)?m\[(.+?)\]\x1b\[m/, "<span class='pst-tag'>$1</span>")
						.replace(/^\[(转载|合集)\]/, "<span class='pst-tag'>$1</span>");
			}
		},

		Article: function(post) {
			var lines = post.content.split('\n');
			if (lines.length > 4) {
				$.extend(this, parseHeader(lines));
				lines = lines.slice(4);
			}
			this.lines = lines;

			$.extend(this, post);
			delete this.content;
		},

		setupForm: function(options) {
			var $f = $(App.render('post-new', {
				anonymous: options.anonymous
			}));

			if (options.hideTitle)
				$f.find('[name=title]').attr('type', 'hidden');

			$f.find('[type=submit]').click(function() {
				$f.prop('disabled', true);
				App.ajax({
					type: 'POST',
					url: App.api('post', {
						board_id: options.boardId,
						reply_id: options.replyId
					}),
					data: $f.serializeArray(),
				}).done(options.done)
				.fail(function(jqXHR, textStatus, errorThrown) {
					$f.prop('disabled', false);
				});
				return false;
			});
			$f.find('[name=title]').attr('value', options.title);
			$f.find('textarea').val(options.content).focus();
			$f.find('button').click(options.cancel);
			return $f;
		},

		parse: function(lines) {
			var state = new State();
			lines.forEach(function(line) {
				if (!state.footer) {
					if (line == '--') {
						state.setFooter(true);
						return;
					}
					state.setQuote(line.startsWith('【 在 ') || line.startsWith(': '));
				}
				state.lines.push('<p>');
				$.merge(state.lines, state.parseLine(line));
				state.lines.push('</p>');
			});
			state.setQuote(false);
			state.setFooter(false);
			return state.lines.join('');
		}
	};

	$.extend(Post.Article.prototype, {
		contentHtml: function() {
			return Post.parse(this.lines);
		},

		titleHtml: function() {
			return Post.titleHtml(this.title);
		},

		titleReply: function() {
			if (this.title) {
				return this.title.startsWith('Re: ') ? this.title : 'Re: ' + this.title;
			}
		},

		quote: function() {
			var quotes = [
				'',
				Mustache.render(QUOTE_TEMPLATE, this)
			];
			var length = this.lines.length,
				seeker, line, i, l, left = QUOTE_WIDTH_MAX, end = false;
			for (i = 0; i < length && !end; ++i) {
				line = this.lines[i];
				if (line == '--') break;
				if (/^([:>] ){2,}/.test(line) || /^\s+$/.test(line)) continue;

				seeker = new Util.Width.Seeker(line);
				while (true) {
					l = seeker.next(left > TRUNCATE_WIDTH ? TRUNCATE_WIDTH : left);
					if (!l.str) break;

					quotes.push(': ' + l.str);
					left -= l.width;

					if (quotes.length >= QUOTE_LINE_MAX + 2 || left <= 0) {
						quotes.push(OMIT_STRING);
						end = true;
						break;
					}
				}
			}
			return Util.escape(quotes.join('\n'));
		}
	});

	Post.Content = App.P({
		tmpl: 'post',
		m: {
			convert: function(p) {
				return new Post.Article(p);
			},
			init: function(data) {
				data.posts = $.map(data.posts, this.convert);
				this.data = data;
			},
			thread_id: function() {
				var l = this.data.posts.length;
				return l ? this.data.posts[0].thread_id : 0;
			},
			max_id: function() {
				var l = this.data.posts.length;
				return l ? this.data.posts[l - 1].id : 0;
			},
			count: function() {
				return this.data.posts.length;
			},
			append: function(posts) {
				posts = $.map(posts, this.convert);
				$.merge(this.data.posts, posts);
				return posts;
			},
			anonymous: function() {
				return !!this.data.board.anonymous;
			},
			find: function(id) {
				return $.grep(this.data.posts, function(p, i) {
					return p.id == id;
				})[0];
			}
		},
		v: {
			append: function(posts) {
				$($.map(posts, function(p) {
					return App.partial('post', p);
				}).join(''))
					.appendTo(this.$('#pst-li'))
					.hook();
			}
		},
		c: {
			nav: 'board',
			post: function() {
				Ui.loadMore({
					ctrl: this,
					count: 20,
					autoload: true,
					retry: function(retries) {
						var s = '看看有没有新回复? ';
						for (var i = 0; i < retries; ++i)
							s += '没有..';
						return s;
					},
					api: 'post',
					param: function(model) {
						return {
							board_id: model.data.board.id,
							thread_id: model.thread_id(),
							since_id: model.max_id()
						};
					},
					done: function(data) {
						return data.posts;
					}
				});

				var model = this.model;
				this.view.$('.pst-new-btn').click(function(evt) {
					var $t = $(this), $p = $t.parent(),
						$f = $p.parent().find('.pst-new'), $s,
						replyId = $t.data('replyId'),
						post = model.find(replyId),
						cancel = function() {
							$f.remove();
							$t.prop('disable', false);
							return false;
						};

					if ($f.length == 0) {
						$f = Post.setupForm({
							anonymous: model.anonymous(),
							boardId: $t.data('boardId'),
							replyId: replyId,
							title: post.titleReply(),
							content: post.quote(),
							hideTitle: true,
							done: function(data) {
								cancel();
								$s = $('<div class="pst-done">发表成功</div>').appendTo($p);
								setTimeout(function() { $s.slideUp(); }, 1000);
							},
							cancel: cancel
						});
						$f.appendTo($p).show();
						$t.prop('disable', true);
					}
					return false;
				});
			},
			leave: function() {
				$(window).off('scroll');
				this.model.data.posts.forEach(function(p) {
					if (p.id == p.reply_id) {
						Post.ReadCache.set(p.id);
					}
				});
			}
		}
	});

	Post.ReadCache = {
		key: 'read-cache',

		set: function(postId) {
			var val = App.S.get(this.key) || {};
			val[postId] = 1;
			App.S.set(this.key, val);
		},

		remove: function() {
			return App.S.remove(this.key);
		}
	};
})();
