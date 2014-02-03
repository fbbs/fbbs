/**
 * @fileOverview board search
 * @author pzh
 */

(function ($, f) {
    'use strict';

    $.widget("ui.boardSearch", {
        version: '1.0.0',
        options: {
            tpl: 'ui.boardSearch.tpl',
            listTpl: 'ui.boardSearchList.tpl',
            searchDelay: 700,
            onsearch: null
        },
        _debounce: 0,
        _timeout: null,
        _create: function () {
            this.element.html(f.tpl.format(this.options.tpl));
            this._getElements();
            this._bindEvents();
        },
        _getElements: function () {
            this._$input = this.element.find('.ui-board-search-input');
            this._$container = this.element.find('.ui-board-search');
            this._$list = this.element.find('.ui-board-search-list-container');
        },
        _bindEvents: function () {
            this._on({
                'input .ui-board-search-input': $.proxy(this, '_oninput'),
                'propertychange .ui-board-search-input': $.proxy(this, '_oninput'),
                'keydown .ui-board-search-input': $.proxy(this, '_onkeydown'),
                'click .ui-board-search-list-reset': $.proxy(this, '_reset'),
                'click .ui-board-search-target': $.proxy(this, '_onclickTarget')
            });
        },
        _onkeydown: function (event) {
            switch (event.which) {
                case 13:
                    event.preventDefault();
                    event.stopPropagation();
                    var a = this._$list.find('.ui-board-search-item.focus a');
                    if (a.length) {
                        a.click();
                        window.location.href = a.attr('href');
                    }
                    break;
                case 38:
                    this._prev();
                    break;
                case 40:
                    this._next();
                    break;
            }
        },
        _oninput: function (event) {
            if (event.propertyName && event.propertyName.toLowerCase() !== 'value') {
                return;
            }
            var now = (new Date()).getTime();
            if (now - this._debounce < this.options.searchDelay) {
                this._timeout && clearTimeout(this._timeout);
            }
            this._timeout = setTimeout($.proxy(this, '_onsearch', $(event.target).val()), this.options.searchDelay);
            this._debounce = now;
        },
        _onsearch: function (value) {
            var val = f.trim(value);
            this._$input.val(val);
            if (val) {
                this._trigger('onsearch', null, {
                    value: val
                });
            }
            else {
                this.removeList();
            }
        },
        _focus: function (index) {
            this._$list
                .find('.ui-board-search-item')
                .removeClass('focus')
                .eq(index)
                .addClass('focus');
        },
        _next: function () {
            var items = this._$list.find('.ui-board-search-item');
            var focused = items.filter('.focus');
            var index = items.index(focused) + 1;
            if (index < items.length) {
                focused.removeClass('focus');
                items.eq(index).addClass('focus');
            }
        },
        _prev: function () {
            var items = this._$list.find('.ui-board-search-item');
            var focused = items.filter('.focus');
            var index = items.index(focused) - 1;
            if (index >= 0) {
                focused.removeClass('focus');
                items.eq(index).addClass('focus');
            }
        },
        _reset: function () {
            this.removeList();
            this._$input.val('');
        },
        _onclickTarget: function () {
            this._reset();
            $('.menu-toggle').click();
        },
        loading: function (toggle) {
            this._$container.toggleClass('loading', toggle !== false);
        },
        renderList: function (list) {
            this._$list.html(f.tpl.format(this.options.listTpl, {
                list: list
            }));
            this._focus(0);
        },
        removeList: function () {
            this._$list.empty();
        }
    });
}(jQuery, f));
