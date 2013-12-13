/**
 * @fileOverview in charge of top menu navigation
 * @author pzh
 */

(function ($, f) {
    'use strict';

    $.widget("ui.topbar", {
        version: '1.0.0',
        options: {

            // tpls
            tpl: 'ui.topbar.tpl',

            // callbacks
            onchange: null
        },
        /**
         * widget constructor
         *
         * @private
         */
        _create: function () {
            this.render();
            this._bindEvents();
        },
        /**
         * render menu nav
         *
         * @private
         */
        render: function () {
            this.element.html(f.tpl.format(this.options.tpl));
        },
        /**
         * bind events
         *
         * @private
         */
        _bindEvents: function () {
            this._on({
                'click .nav': $.proxy(this, '_onNavClick'),
                'click .name': $.proxy(this, '_onNavClick')
            });
        },
        /**
         * nav click event handler
         *
         * @private
         * @event
         * @param {Object} event Event object
         */
        _onNavClick: function (event) {
            //如果是打开新页面的链接则不切换样式
            if ($(event.currentTarget).attr('target') !== '_blank') {
                this.element
                    .find('.nav')
                    .removeClass('active');

                $(event.currentTarget)
                    .closest('.nav')
                    .addClass('active');
            }

            this._trigger('onchange', null, {
                currentTarget: event.currentTarget
            });
        },
        setActivate: function (menNavId) {
            this.element
                .find('.nav')
                .removeClass('active');

            this.element
                .find(f.format('[data-menu-nav-id=#{0}]', menNavId))
                .addClass('active');
        }
    });
}(jQuery, f));
