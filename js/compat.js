'use strict';

if (!Object.keys) {
	Object.keys = function(o) {
		var k = [], p;
		for (p in o) {
			if (Object.prototype.hasOwnProperty.call(o,p)) {
				k.push(p);
			}
		}
		return k;
	}
}

if (!Array.prototype.forEach) {
	Array.prototype.forEach = function(f) {
		$.each(this, function(i, e) {
			f(e);
		});
	};
}

if (!String.prototype.startsWith) {
	String.prototype.startsWith = function(s) {
		return this.substring(0, s.length) === s;
	}
}

if (!String.prototype.endsWith) {
	String.prototype.endsWith = function(s) {
		var tl = this.length, sl = s.length;
		return tl >= sl && this.substring(tl - sl) === s;
	}
}
