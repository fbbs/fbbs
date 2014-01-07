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
                'click .menu-toggle': $.proxy(this, '_onNavClick'),
                'click .canvas-toggle': $.proxy(this, '_onShowCanvas'),
                'mousedown .exit-canvas-menu': $.proxy(this, '_onHideCanvas'),
                'touchstart .exit-canvas-menu': $.proxy(this, '_onHideCanvas')
            });
        },
        _onShowCanvas: function (event) {
            this.element.addClass('move-right');
        },
        _onHideCanvas: function (event) {
            this.element.removeClass('move-right');
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
            if ($target.attr('target') !== '_blank' && !$target.hasClass('menu-toggle-new')) {
                this.element
                    .find('.nav')
                    .removeClass('active');

                $target.closest('.nav')
                    .addClass('active');
            }

            this._onHideCanvas();

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
