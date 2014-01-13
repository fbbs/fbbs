/**
 * @fileOverview base functionality and jQuery and underscore extension
 */

/**
 * 排他层显示，即同一时间只显示一个层
 *
 * @author tangjinhua
 */
(function ($) {
    'use strict';

    var currentTrigger = null;
    var currentLayer = null;

    $(document).on('click', function (event) {
        var target = event.target;
        var layerTrigger = null;
        var layer = null;

        do {
            var $target = $(target);

            if (target != document) {
                if ($target.hasClass('layer')) {
                    layer = target;
                }

                if ($target.attr('layer')) {
                    layerTrigger = target;
                }
            }
        }
        while (target = target.parentNode);

        var toggleSelected = function (element, open) {
            var $element = $(element);

            if (open) {
                $element.addClass('selected layer-trigger-selected');
            } else {
                $element.removeClass('selected layer-trigger-selected');
            }

            $element.find('.arrow').each(function () {
                if (open) {
                    $(this).addClass('selected');
                } else {
                    $(this).removeClass('selected');
                }
            });
        }

        if (layerTrigger == null) {
            //如果没有点击trigger且点击范围在currentLayer外(如点击document空白处时)，需要关闭当前layer
            if (currentLayer != null && layer != currentLayer) {
                $(currentLayer).hide();
                toggleSelected(currentTrigger, false);
                currentLayer = null;
                currentTrigger = null;
            }
        } else {
            //点击包含layer属性的元素或它的子元素
            var $layerTrigger = $(layerTrigger);
            var $layer = $($layerTrigger.attr('layer'));
            var layer = $layer.get(0);

            //如果找不到layer属性所指向的元素，则返回
            if (layer == null) {
                return false;
            }

            //如果是打开新的layer，则需要先关闭当前layer
            if (currentLayer != null && layer != currentLayer) {
                $(currentLayer).hide();
                toggleSelected(currentTrigger, false);
            }

            currentLayer = null;
            currentTrigger = null;

            //如果当前layer隐藏或者是shared-layer(用于多个trigger控制同一layer的展示)，则显示它； 否则隐藏它
            if ($layer.css('display') === 'none' || $layer.hasClass('shared-layer')) {
                $layer.show();
                toggleSelected(layerTrigger, true);
                currentLayer = layer;
                currentTrigger = layerTrigger;
            } else {
                $layer.hide();
                toggleSelected(layerTrigger, false);
            }
        }
    });
}(jQuery));

// get rid of reporting errors where some browsers don't support `console`
(function () {
    var method;
    var noop = function () {};
    var methods = ['assert', 'clear', 'count', 'debug', 'dir', 'dirxml', 'error', 'exception', 'group', 'groupCollapsed', 'groupEnd', 'info', 'log', 'markTimeline', 'profile', 'profileEnd', 'table', 'time', 'timeEnd', 'timeStamp', 'trace', 'warn'];
    var length = methods.length;
    var console = (window.console = window.console || {});

    while (length--) {
        method = methods[length];

        // Only stub undefined methods.
        if (!console[method]) {
            console[method] = noop;
        }
    }
}());

// jQuery extension
(function ($) {
    'use strict';

    var _isMobile = {
            Android: function () {
                return /Android/i.test(navigator.userAgent);
            },
            BlackBerry: function () {
                return /BlackBerry/i.test(navigator.userAgent);
            },
            iOS: function () {
                return /iPhone|iPad|iPod/i.test(navigator.userAgent);
            },
            Windows: function () {
                return /IEMobile/i.test(navigator.userAgent);
            }
        };

    $.extend({
        /**
         * 发送一个ajax请求
         *
         * @param {string} type 发送请求的方式
         * @param {string} url 发送请求的url
         * @param {Object} jsonData 需要发送的数据
         * @param {Function} successFn 请求成功时触发，function(data, status)
         * @param {Function} failedFn 请求失败时触发，function(msg)
         */
        jsonAjax: function (type, url, jsonData, successFn, failedFn) {
            $.ajax({
                type: type,
                url: url,
                data: jsonData,
                dataType: 'json',
                success: function (data, textStatus, jqXHR) {
                    var ret = data;

                    //状态为0，表示成功
                    if (ret.status === 0) {
                        //调用成功函数，传递数据，同时传递状态码
                        successFn(ret.data, ret.status, ret.msg);
                    }
                    //状态为1，表示重定向至某个地址
                    else if (ret.status === 1) {
                        window.location = ret.data;
                    }
                    //状态为其他，表示失败
                    else {
                        if (failedFn) {
                            failedFn(ret.msg, ret.data);
                        }
                    }

                }
            }).fail(function (jqXHR, textStatus, errorThrown) {
                if (failedFn) {
                    failedFn(errorThrown.message);
                }
            });
        },
        isMobile: {
            Android: _isMobile.Android(),
            iOS: _isMobile.iOS(),
            BlackBerry: _isMobile.BlackBerry(),
            Windows: _isMobile.Windows(),
            any: _isMobile.Android() || _isMobile.iOS() || _isMobile.BlackBerry() || _isMobile.Windows()
        }

    });

    //extend ui.widget
    $.extend($.Widget.prototype, {
        /**
         * Generate the unique id for widget
         *
         * @public
         * @param {string} key optional suffix key
         *
         * @return {string} id
         */
        getId: function (key) {
            var widget = this;
            var idPrefix;

            idPrefix = "jQuery-" + widget.widgetFullName + '--' + (widget.id ? widget.id : widget.uuid);
            return key != null ? idPrefix + "-" + key : idPrefix;
        },
        /**
         * Return the instance of widget, it equals `this`
         *
         * @public
         *
         * @return {Object} instance of widget
         */
        instance: function () {
            return this;
        }
    });
}(jQuery));

// underscore extension
(function (_) {
    'use strict';

    /**
     * 格式化数字, by Tangram.js
     * @private
     * @param {Number} number 需要格式化的数字
     * @param {Object} options 格式化数字使用的参数
     * @return {String}
     */
    var _format = function (number, options) {
        var numberArray = String(number).split(options.decimal),
            preNum = numberArray[0].split('').reverse(),
            aftNum = numberArray[1] || '',
            len = 0,
            remainder = 0,
            result = '';

        len = parseInt(preNum.length / options.groupLength);
        remainder = preNum.length % options.groupLength;
        len = remainder == 0 ? len - 1 : len;

        for (var i = 1; i <= len; i++) {
            preNum.splice(options.groupLength * i + (i - 1), 0, options.group);
        }
        preNum = preNum.reverse();
        result = options.symbol + preNum.join('') + (aftNum.length > 0 ? options.decimal + aftNum : '');

        return result;
    };

    /**
     * 格式化数字
     * @private
     * @param {Number} number 需要格式化的数字
     * @return {String}
     */
    var formatNumber = function (num) {
        if (num == null) {
            return '';
        }
        if (num == '--') {
            return num;
        }
        if (isNaN(parseFloat(num))) {
            return num;
        }
        return _format(num, {
            group: ',',
            decimal: '.',
            groupLength: 3,
            symbol: ''
        })
    };

    /**
     * 格式化比率
     * @private
     * @param {Number} number 需要格式化的数字
     * @return {String}
     */
    var formatRatio = function (num) {
        if (num == null) {
            return '';
        }
        if (num == '--') {
            return num;
        }
        if (isNaN(parseFloat(num))) {
            return num;
        }
        return formatNumber(num) + '%'
    };

    /**
     * 格式化时间
     * @private
     * @param {Number} number 需要个数化的时间
     * @param {Number} number 格式化类型
     * @return {String}
     */
    var formatTime = function (second, type) {
        var dd, hh, mm, ss, result;
        if (second == '--') {
            return second;
        }

        if (type == 2) {
            //时间的另一种表示法： 22'11"
            mm = second / 60 | 0;
            ss = Math.round(second) - mm * 60;
            var ret = "";
            if (mm) {
                ret += mm + "&#039;";
            }
            ret += ss + "&quot;";
            return ret;
        }

        //先处理天
        dd = second / (24 * 3600) | 0;
        second = Math.round(second) - dd * 24 * 3600;
        //接着是小时
        hh = second / 3600 | 0;
        second = Math.round(second) - hh * 3600;
        //之后是分
        mm = second / 60 | 0;
        //最后是秒
        ss = Math.round(second) - mm * 60;

        if (Math.round(dd) < 10) {
            dd = dd > 0 ? '0' + dd : '';
        }
        if (Math.round(hh) < 10) {
            hh = '0' + hh;
        }
        if (Math.round(mm) < 10) {
            mm = '0' + mm;
        }
        if (Math.round(ss) < 10) {
            ss = '0' + ss;
        }
        if (dd) {
            result = dd + ' ' + hh + ':' + mm + ':' + ss;
        } else {
            result = hh + ':' + mm + ':' + ss;
        }
        return result;
    };

    var htmlEncodeDict = {
        '"': "quot",
        "'": "#39",
        "<": "lt",
        ">": "gt",
        "&": "amp",
        " ": "nbsp"
    };

    // Mix in non-conflict functions to Underscore namespace
    _.mixin(_.str.exports());

    _.mixin({
        /**
         * Check whether it's a valid url
         *
         * @public
         * @param {string} url the value of url
         *
         * @return {boolean} check result
         */
        isUrl: function (url) {
            return (/^((https|http|ftp|rtsp|mms)?:\/\/)?(([\w-]+\.)+[a-z]{2,6}|((25[0-5]|2[0-4]\d|1\d{2}|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d{2}|[1-9]\d|\d))(:[0-9]+)?(\/|\/([\w#!:.?+=&%@!\-\/]))?/i).test(url);
        },
        /**
         * Check whether it's a valid email
         *
         * @public
         * @param {string} value the value of email
         *
         * @return {boolean} check result
         */
        isEmail: function (value) {
            return (/^\w+([-\.]\w+)*@\w+([-\.]\w+)*\.\w+([-\.]\w+)*$/i).test(value);
        },
        /**
         * Encode the html DOM attrs
         *
         * @public
         * @param {String} html string value
         */
        encodeAttr: function (html) {
            return String(html).replace(/["']/g, function (all) {
                return "&" + htmlEncodeDict[all] + ";";
            });
        },
        /**
         * 对目标字符串进行格式化，同时添加对encode的支持，!{0}保持原有；#{0}html encode； #{0, 10} truncat length
         *
         * @public
         * @name f.format
         * @function
         * @grammar f.format(source, opts)
         * @param {string} source 目标字符串
         * @param {Object|string...} opts 提供相应数据的对象或多个字符串
         * @remark
         *
            opts参数为"Object"时，替换目标字符串中的#{property name}部分。<br>
            opts为“string...”时，替换目标字符串中的#{0}、#{1}...部分。
         * @shortcut f.format
         * @meta standard
         *
         * @returns {string} 格式化后的字符串
         */
        format: function (source, opts) {
            source = String(source);
            var data = Array.prototype.slice.call(arguments, 1),
                toString = Object.prototype.toString;
            if (data.length) {
                data = data.length == 1 ? /* ie 下 Object.prototype.toString.call(null) == '[object Object]' */
                (opts !== null && (/\[object Array\]|\[object Object\]/.test(toString.call(opts))) ? opts : data) : data;
                return source.replace(/>\s*</g, '><').replace(/(#|!|@)\{(.+?)(?:\s*[,:]\s*(\d+?))*?\}/g, function (match, type, key, length) {
                    var replacer = data[key];
                    // chrome 下 typeof /a/ == 'function'
                    if ('[object Function]' == toString.call(replacer)) {
                        replacer = replacer(key);
                    }
                    if (length) {
                        replacer = f.truncate(replacer, length);
                    }
                    //html encode
                    if (type == "#") {
                        replacer = f.escape(replacer);
                    } else if (type === '@') {
                        replacer = f.encodeAttr(replacer);
                    }

                    return ('undefined' == typeof replacer ? '' : replacer);
                });
            }
            return source;
        },
        /**
         * Format the date depend on the pattern
         *
         * @public
         * @param {(Date | string | number)} source value that represent the date
         * @param {string} pattern date pattern string
         *
         * @return {string} result of date format
         */
        formatDate: function (source, pattern) {
            if ('string' != typeof pattern) {
                return source.toString();
            }

            if (!f.isDate(source)) {
                source = new Date(source);
            }

            function replacer(patternPart, result) {
                pattern = pattern.replace(patternPart, result);
            }

            var pad = function (source, length) {
                var pre = "",
                    negative = (source < 0),
                    string = String(Math.abs(source));

                if (string.length < length) {
                    pre = (new Array(length - string.length + 1)).join('0');
                }

                return (negative ? "-" : "") + pre + string;
            },
            year = source.getFullYear(),
                month = source.getMonth() + 1,
                date2 = source.getDate(),
                hours = source.getHours(),
                minutes = source.getMinutes(),
                seconds = source.getSeconds();

            replacer(/yyyy/g, pad(year, 4));
            replacer(/yy/g, pad(parseInt(year.toString().slice(2), 10), 2));
            replacer(/MM/g, pad(month, 2));
            replacer(/M/g, month);
            replacer(/dd/g, pad(date2, 2));
            replacer(/d/g, date2);

            replacer(/HH/g, pad(hours, 2));
            replacer(/H/g, hours);
            replacer(/hh/g, pad(hours % 12, 2));
            replacer(/h/g, hours % 12);
            replacer(/mm/g, pad(minutes, 2));
            replacer(/m/g, minutes);
            replacer(/ss/g, pad(seconds, 2));
            replacer(/s/g, seconds);

            return pattern;
        },
        /**
         * Generate namespace
         *
         * @public
         * @param {string} ns_string namespace string that is joined with dot
         *
         * @return {Object} the top-end object of the namespace
         */
        namespace: function (ns_string) {
            if (!ns_string || !ns_string.length) {
                return null;
            }

            var _package = window;

            for (var a = ns_string.split('.'), l = a.length, i = (a[0] == 'window') ? 1 : 0; i < l;
            _package = _package[a[i]] = _package[a[i]] || {}, i++);

            return _package;
        },
        /**
         * Truncate string
         *
         * @public
         * @param {string} str the source string text
         * @param {number} length determine how many characters to truncate to
         * @param {string} truncateStr this is a text string that replaces the truncated text, tts length is not included in the truncation length setting
         * @param {[type]} middle this determines whether the truncation happens at the end of the string with false, or in the middle of the string with true
         *
         * @return {string} string text that is truncated to
         */
        truncate: function (str, length, truncateStr, middle) {
            if (str == null) return '';
            str = String(str);

            if (typeof middle !== 'undefined') {
                middle = truncateStr;
                truncateStr = '...';
            } else {
                truncateStr = truncateStr || '...';
            }

            length = ~~length;
            if (!middle) {
                return str.length > length ? str.slice(0, length) + truncateStr : str;
            } else {
                return str.length > length ? str.slice(0, length / 2) + truncateStr + str.slice(-length / 2) : str;
            }
        },
        /**
         * Converts a underscored or camelized string into an dasherized one
         *
         * @public
         * @param {string} str source text
         * @param {boolean} isRemoveFirstDash whether remove the first char if that is a dash
         *
         * @return {string} dasherized string text
         */
        dasherize: function (str, isRemoveFirstDash) {
            var result = f.trim(str).replace(/([A-Z])/g, '-$1').replace(/[-_\s]+/g, '-').toLowerCase();

            if (isRemoveFirstDash) {
                return result.replace(/^-/, '');
            } else {
                return result;
            }
        },
        /**
         * Convert a value to JSON, delegate to JSON2.js plugin
         *
         * @public
         *
         * @return {string} a JSON string
         */
        stringify: function () {
            return JSON.stringify.apply(null, arguments);
        },
        /**
         * Parse a string as JSON, delegate to JSON2.js plugin
         *
         * @public
         *
         * @return {any} parsed value
         */
        parse: function () {
            return JSON.parse.apply(null, arguments);
        },
        /**
         * Generate global guid
         *
         * @public
         *
         * @return {string} guid string
         */
        guidGenerator: function () {
            var S4 = function () {
                return (((1 + Math.random()) * 0x10000) | 0).toString(16).substring(1);
            };

            return (S4() + S4() + "-" + S4() + "-" + S4() + "-" + S4() + "-" + S4() + S4() + S4());
        },
        /**
         * Blank function
         *
         * @public
         */
        noop: function () {},
        formatNumber: formatNumber,
        formatRatio: formatRatio,
        formatTime: formatTime
    });
}(_));
