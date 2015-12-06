'use strict';

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
