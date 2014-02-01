(function ($, f) {
    'use strict';

    var page = {
        components: [],
        beforeInitComponents: function () {
            var me = this;
            var login = {
                type: 'Login',
                selector: '.login-container',
                options: {
                    content: true
                }
            };
            this.components.push(login);
        },
        oninit: function () {
            if (this.userInfo.user && this.userInfo.token) {
                window.location.href = this.pageInfo.params.r || this.systemConfig.defaultPage;
                return;
            }
            $('#id').focus();
        }
    };

    f.bbsPage = new f.page(page);
}(jQuery, f));
