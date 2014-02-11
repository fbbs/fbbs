/**
 * @fileOverview collection list
 * @author pzh
 */

(function ($, f) {
    'use strict';

    $.widget("ui.collections", {
        version: '1.0.0',
        options: {
            tpl: 'ui.sideBarList.tpl',
            collectionEditUrl: '',
            label: '我的收藏',
            editLabel: '编辑',
            emptyTip: '没有收藏版面',
            loadingTip: '载入中...',
            className: 'ui-collections-list',
            onrefresh: null
        },
        render: function (list, option) {
            var options = $.extend({}, this.options, option);
            this.element.html(f.tpl.format(this.options.tpl, {
                className: options.className,
                label: options.label,
                rightLabel: options.editLabel,
                rightUrl: options.collectionEditUrl,
                emptyTip: options.emptyTip,
                loadingTip: option.loadingTip,
                list: list
            }));
        },
        empty: function () {
            this.render();
        },
        error: function (msg) {
            this.render(null, {
                emptyTip: msg
            });
        },
        loading: function () {
            this.render(null, {
                loadingTip: this.options.loadingTip
            });
        },
        refresh: function () {
            this._trigger('onrefresh', null);
        }
    });
}(jQuery, f));
