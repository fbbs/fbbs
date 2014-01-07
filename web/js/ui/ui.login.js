(function ($, f) {
    'use strict';

    $.widget('ui.login', {
        options: {
            loginUrl: '',
            registUrl: '',
            forgetUrl: '',
            trigger: '',
            tpl: 'ui.login.tpl'
        },
        /**
         * widget 构造器
         *
         * @private
         */
        _create: function () {
            this.element.append(f.tpl.format(this.options.tpl, this.options));
            this._renderDialog();
            this._bindEvents();
        },
        _bindEvents: function () {
            $(this.options.trigger).on('click', $.proxy(this, 'open'));
        },
        _renderDialog: function (options) {
            var options = $.extend({}, this.options, options);
            var me = this;

            this.dialog = this.element;

            this.dialog.dialog({
                autoOpen: false,
                closeText: '关闭',
                dialogClass: 'ui-login-dialog',
                title: '登陆日月光华',
                modal: true,
                overlayClass: 'ui-login-overlay',
                resizable: false,
                width: '50%',
                minHeight: 50
            });
        },
        open: function () {
            this.dialog.dialog('open');
        }
    });
}(jQuery, f));
