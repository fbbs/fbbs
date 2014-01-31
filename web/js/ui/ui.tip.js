/**
 * @fileOverview global tip
 * @author pzh
 */

(function ($, f) {
    'use strict';

    $.widget("ui.tip", {
        version: '1.0.0',
        options: {
            tpl: 'ui.tip.tpl',
            onclose: null
        },
        append: function (alertOptions) {
            var options = $.extend({}, {
                id: ['alert', (new Date()).valueOf()].join(""),
                boxClass: '',
                content: ''
            }, alertOptions);
            this.element
                .append(f.tpl.format(this.options.tpl, options));
            this.element
                .find(['#', options.id, ' .ui-alert-close'].join(""))
                .on('click', $.proxy(this, 'close'));
        },
        close: function (id) {
            if (!id) {
                this.element.empty();
                return;
            }
            if ('string' !== typeof id) {
                var ele = $(id.target).parent();
                var eid = ele.attr('id');
            }
            else {
                var eid = id;
                var ele = this.element.find(['#', id].join(""));
            }
            this._trigger('onclose', null, {
                id: eid
            });
            if (ele.length) {
                ele.remove();
            }
        }
    });
}(jQuery, f));
