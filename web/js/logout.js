(function ($, f) {
    'use strict';

    var page = {
        components: [],
        config: {
            logoutErrorTip: '退出失败，请刷新重试。(#{0})',
            logoutSuccessTip: '您已成功退出。',
            logoutDefaultErrorTip: '网络链接失败'
        },
        methods: {
            logout: {
                name: 'logout',
                url: 'user/logout.json',
                type: 'POST'
            }
        },
        oninit: function () {
            this.ajax(this.methods.logout);
        },
        onlogoutSuccess: function () {
            var me = this;
            var options = {
                path: me.systemConfig.cookiePath
            };
            $.removeCookie('utmpuserid', options);
            $.removeCookie('utmpkey', options);
            f.config.userInfo = {};
            // Rerender the topbar
            $('.menu-nav').topbar('render');
            this.Login.login('rebind');
            $('.logout-loading').hide();
            $('.logout-tip').text(this.config.logoutSuccessTip);
            window.location.href = this.systemConfig.defaultPage;
        },
        onlogoutFailed: function (msg) {
            $('.logout-loading').hide();
            $('.logout-tip').text(f.format(this.config.logoutErrorTip, msg || this.config.logoutDefaultErrorTip));
        }
    };

    f.bbsPage = new f.page(page);
}(jQuery, f));
