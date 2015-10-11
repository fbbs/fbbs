(function() {
	var _template = {};
	window.Template = {
		get: function(name, param) {
			var tmpl = _template[name];
			if (tmpl) {
				tmpl.param = param;
			}
			return tmpl;
		},
		set: function(name, handler) {
			_template[name] = handler;
		}
	};
})();
