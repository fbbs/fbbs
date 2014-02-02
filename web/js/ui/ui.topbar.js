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

            urls: {},

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
            this.element.html(f.tpl.format(this.options.tpl, this.options.urls));
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
            this._onHideCanvas();
            if ($(event.target).hasClass('left-off-canvas-toggle')) {
                this.element.addClass('move-right');
            }
            else if ($(event.target).hasClass('right-off-canvas-toggle')) {
                this.element.addClass('move-left');
            }
        },
        _onHideCanvas: function (event) {
            this.element.removeClass('move-right').removeClass('move-left');
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
        },
        setUrl: function (options) {
            this.element.find(options.selector).attr('href', options.url);
        }
    });
}(jQuery, f));
