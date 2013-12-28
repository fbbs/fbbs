(function ($, f) {
    'use strict';

    $.widget("ui.mask", {
        version: '1.0.0',
        options: {
            zIndex: 999999
        },
        render: function () {
            this.maskElement = $('<div>')
                .attr('id', f.uniqueId('mask'))
                .attr('class', 'mask-layer')
                .appendTo('body');

            this._setStyles();

            if ($.browser.ie && $.browser.version < 7) {
                this.element.addClass('body-masked');
            }
        },
        _setStyles: function () {
            var position = this.element.offset();

            this.maskElement.css({
                left: 0,
                top: 0,
                position: 'absolute',
                width: this.element.width(),
                height: this.element.height(),
                zIndex: this.options.zIndex
            });
        },
        show: function () {
            this.render();
        },
        remove: function () {
            this.maskElement.remove();
        }
    });
}(jQuery, f));
