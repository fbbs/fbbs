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
                'click .menu-nav': $.proxy(this, '_onNavClick'),
                'click .toggle-topbar': $.proxy(this, '_toggle'),
                'touchstart .main': $.proxy(this, '_close')
            });
        },
        _toggle: function (event) {
            this.element
                .find('.top-bar')
                .toggleClass('expanded');
        },
        _close: function (event) {
            this.element
                .find('.top-bar')
                .removeClass('expanded');
        },
        /**
         * nav click event handler
         *
         * @private
         * @event
         * @param {Object} event Event object
         */
        _onNavClick: function (event) {
            var $target = $(event.currentTarget);
            if ($target.attr('target') !== '_blank' && !$target.hasClass('menu-nav-new')) {
                this.element
                    .find('.nav')
                    .removeClass('active');

                $target.closest('.nav')
                    .addClass('active');
            }

            this._close();

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
