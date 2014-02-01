(function ($, f) {
    'use strict';

    $.widget('ui.login', {
        version: '1.0.0',
        options: {
            registUrl: 'javascript:void(0);',
            forgetUrl: 'javascript:void(0);',
            loginText: '登&emsp;&emsp;陆',
            dialogTitle: '登陆',
            loadingText: '登陆中...',
            trigger: '',
            overlayClass: 'ui-login-overlay',
            tpl: 'ui.login.tpl',
            cookiePath: '/'
        },
        /**
         * widget 构造器
         *
         * @private
         */
        _create: function () {
            this.element.append(f.tpl.format(this.options.tpl, this.options));
            this._renderDialog();
            this._$id = this.element.find('.ui-login-input-id');
            this._$pw = this.element.find('.ui-login-input-pw');
            this._$rm = this.element.find('.ui-login-input-rm');
            this._$tip = this.element.find('.ui-login-tip');
            this._$inputs = this.element.find('.ui-login-input');
            this._$submit = this.element.find('.ui-login-submit');
            this._bindEvents();
        },
        _bindEvents: function () {
            this.rebind();
            this._on({
                'click .ui-login-submit': '_onclickSubmit',
                'keydown .ui-login-input': '_onkeydownInput',
                'click .ui-login-label-tip': '_onclickTip'
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
        _onclickSubmit: function (event) {
            event.preventDefault();
            if (this.validation()) {
                this._trigger('onsubmit', null, {
                    id: this._$id.val(),
                    pw: this._$pw.val(),
                    rm: this._$rm.prop('checked')
                });
            }
        },
        _onkeydownInput: function (event) {
            if (event.which === 13) {
                event.preventDefault();
                var $target = $(event.target);
                if ($target.attr('name') === 'pw') {
                    $target.blur();
                    this._onclickSubmit(event);
                }
                else {
                    var index = this._$inputs.index($target) + 1;
                    index === this._$inputs.length && (index = 0);
                    var $next = this._$inputs.eq(index);
                    if ($next.length) {
                        $next.focus();
                    }
                }
            }
        },
        _onclickTip: function () {
            this.close();
        },
        _saveCookie: function (cookies, options) {
            f.each(cookies, function (value, key) {
                $.cookie(key, value, options);
            });
        },
        save: function (data) {
            var me = this;
            var options = {
                path: me.options.cookiePath,
                expires: data.rm ? 90 : 0
            };
            if (data && data.expires) {
                options.expires = new Date(parseInt(data.expires) * 1000);
            }
            if (!options.expires) {
                delete options.expires;
            }
            this._saveCookie({
                'utmpuserid': data.user,
                'utmpkey': data.token
            }, options);
        },
        open: function () {
            this.dialog.dialog('open');
        },
        close: function () {
            this.dialog.dialog('close');
        },
        error: function (err) {
            this._$tip.text(err);
        },
        validation: function () {
            this.error('');
            var err = '';
            if (this._$id.val() === '') {
                err = '请输入账号';
                this._$id.focus();
            }
            else if (!/[a-z]{2,12}/i.test(this._$id.val())) {
                err = '请输入合法账号';
                this._$id.focus();
            }
            else if (this._$pw.val() === '') {
                err = '请输入密码';
                this._$pw.focus();
            }
            else {
                return true;
            }
            this.error(err);
            return false;
        },
        loading: function () {
            this._$submit
                .prop('disabled', true)
                .addClass('disabled')
                .html(this.options.loadingText);
            this._$inputs
                .prop('disabled', true)
                .addClass('disabled');
            this.element.addClass('loading');
        },
        reset: function () {
            this._$submit
                .prop('disabled', false)
                .removeClass('disabled')
                .html(this.options.loginText);
            this._$inputs
                .prop('disabled', false)
                .removeClass('disabled');
            this.element.removeClass('loading');
        },
        rebind: function () {
            $(this.options.trigger).off('click');
            $(this.options.trigger).on('click', $.proxy(this, 'open'));
        }
    });
}(jQuery, f));
