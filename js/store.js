(function() {
	var store = window.Store;

	window.Store = {
		init: function(prefix) {
			this._p = prefix + ':';
		},

		get: function(key) {
			return store.get(this._p + key);
		},

		set: function(key, val) {
			return store.set(this._p + key, val);
		},

		clear: function() {
			var keys = [], p = this._p;
			store.forEach(function(k, v) {
				if (k.startsWith(p)) {
					keys.push(k);
				}
			});
			keys.forEach(function(k) {
				store.remove(k);
			});
		}
	};
})();
