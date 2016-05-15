/*
 * https://developer.mozilla.org/en-US/docs/Web/API/document.cookie
 * https://developer.mozilla.org/User:fusionchess
 * Revision #1 - September 4, 2014
 * This framework is released under the GNU Public License, version 3 or later.
 */
(function() {
	window.Cookie = {
		abs: function(sRelPath) {
			var nUpLn,
				sDir = "",
				sPath = location.pathname.replace(/[^\/]*$/, sRelPath.replace(/(\/|^)(?:\.?\/+)+/g, "$1"));
			for (var nEnd, nStart = 0; nEnd = sPath.indexOf("/../", nStart), nEnd > -1; nStart = nEnd + nUpLn) {
				nUpLn = /^\/(?:\.\.\/)*/.exec(sPath.slice(nEnd))[0].length;
				sDir = (sDir + sPath.substring(nStart, nEnd)).replace(new RegExp("(?:\\\/+[^\\\/]*){0," + ((nUpLn - 1) / 3) + "}$"), "/");
			}
			return sDir + sPath.substr(nStart);
		},
		get: function(key) {
			if (!key)
				return null;
			return decodeURIComponent(document.cookie.replace(new RegExp('(?:(?:^|.*;)\\s*' + encodeURIComponent(key).replace(/[\-\.\+\*]/g, '\\$&') + '\\s*\\=\\s*([^;]*).*$)|^.*$'), '$1')) || null;
		},
		set: function(key, value, expire, path, domain, secure) {
			if (!key || /^(?:expires|max\-age|path|domain|secure)$/i.test(key))
				return false;
			var expires = '';
			if (expire) {
				switch (expire.constructor) {
					case Number:
						expires = expire == Infinity ? '; expires=Fri, 31 Dec 9999 23:59:59 GMT' : '; max-age=' + expire;
						break;
					case String:
						expires = '; expires=' + expire;
						break;
					case Date:
						expires = '; expires=' + expire.toUTCString();
						break;
				}
			}
			document.cookie = encodeURIComponent(key) + '=' + encodeURIComponent(value) + expires + (domain ? '; domain=' + domain : '') + (path ? '; path=' + path : '') + (secure ? '; secure' : '');
			return true;
		},
		has: function(key) {
			if (!key)
				return false;
			return (new RegExp('(?:^|;\\s*)' + encodeURIComponent(key).replace(/[\-\.\+\*]/g, '\\$&') + '\\s*\\=')).test(document.cookie);
		},
		remove: function(key, path, domain) {
			if (!this.has(key))
				return false;
			document.cookie = encodeURIComponent(key) + '=; expires=Fri, 19-Apr-1996 11:11:11 GMT' + (domain ? '; domain=' + domain : '') + (path ? '; path=' + path : '');
			return true;
		}
	};
})();
