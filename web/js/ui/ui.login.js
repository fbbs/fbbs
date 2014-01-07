(function ($, f) {
    'use strict';

    $.widget('ui.login', {
        options: {
            loginUrl: '',
            registUrl: 'javascript:void(0);',
            forgetUrl: 'javascript:void(0);',
            loginText: '登&emsp;&emsp;陆',
            dialogTitle: '登陆',
            trigger: '',
            overlayClass: 'ui-login-overlay',
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
            $(document).on('touchstart', '.' + this.options.overlayClass, $.proxy(this, 'close'));
            this._on({
                //'touchmove .ui-login-overlay': '_ontouchmoveMask'
            });
        },
        _renderDialog: function (options) {
            this.dialog = this.element;

            this.dialog.dialog({
                autoOpen: false,
                closeText: '关闭',
                dialogClass: 'ui-login-dialog',
                title: this.options.dialogTitle,
                modal: true,
                overlayClass: this.options.overlayClass,
                resizable: false,
                width: 270,
                maxHeight: 270
            });
        },
        open: function () {
            this.dialog.dialog('open');
        },
        close: function () {
            this.dialog.dialog('close');
        }
    });
}(jQuery, f));
