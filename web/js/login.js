(function ($, f) {
    'use strict';

    var page = {
        components: [],
        config: {
            logoutErrorTip: '退出失败，请刷新重试。(#{0})',
            logoutSuccessTip: '您已成功退出。',
            logoutDefaultErrorTip: '网络链接失败'
        },
        beforeInitComponents: function () {
            var me = this;
            var login = {
                id: 'InnerLogin',
                type: 'Login',
                selector: '.login-container',
                options: {
                    content: true
                }
            };
            this.components.push(login);
        },
        oninit: function () {
            $('#id').focus();
        }
    };

    f.bbsPage = new f.page(page);
}(jQuery, f));
