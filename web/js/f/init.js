/**
 * @fileOverview Initializing system, and this is the entry of system
 * @author pzh
 */

(function ($, f) {
    'use strict';

    $.extend(f, (function () {
        /**
         * Initializing widget of system and regiser global event callback handler
         */

        function initBBS() {
            // Apply fastclick
            FastClick.attach(document.body);

            // Get user's cookies
            var user = {
                user: $.cookie('utmpuserid'),
                token: $.cookie('utmpkey')
            };
            if (user.user && user.token) {
                $.extend(f.config.userInfo, user);
            }

            // Render topbar
            $('.menu-nav').topbar({
                onchange: function (event, data) {
                    f.mediator.trigger('app:topbar:onchange', data);
                }
            });

            // Render global tip
            $('#global-tip').tip();

            // Initialize notification
            $(document.body).notifications({
                notificationUrl: [f.config.systemConfig.ajaxUri, '/user/notifications.json'].join(""),
                getInterval: 5000,
                ongetNotification: function () {
                    return !!(f.config.userInfo.user && f.config.userInfo.token);
                },
                ongetNotificationSuccess: function (event, data) {
                    var notified = false;
                    f.each(data, function (value, key) {
                        if (value) {
                            if (~~$('.notifications').find(['.', key].join("")).find('span').text() < value) {
                                notified = true;
                            }
                            $('.notifications').find(['.', key].join(""))
                                .addClass('notified')
                                .find('span')
                                .text(value);
                        }
                        else {
                            $('.notifications').find(['.', key].join(""))
                                .removeClass('notified')
                                .find('span')
                                .text(value);
                        }
                    });
                    if (notified) {
                        if (window.webkitNotifications && window.webkitNotifications.checkPermission() == 0) {
                            var notification = webkitNotifications.createNotification(
                                [f.config.systemConfig.webRoot, '/images/apple-touch-icon-the-new-ipad.png'].join(""),
                                f.config.pageInfo.title || f.config.systemConfig.defaultTitle,
                                '您有新的通知，请点击查看。'
                            );
                            notification.onclick = function() {
                                this.cancel();
                            };
                            notification.replaceId = 'bbsNotification';
                            notification.show();
                        }
                    }
                }
            });

            /*
            $('.side-nav').sideNav({
                zIndex: f.zIndex.ui--
            });
            */

            $(document.body).loading();

            //history back click event handler
            $('.body').on('click', '.history-back', function () {
                window.history.back();
            });

            // hide layer click event handler
            $('.body').on('click', '.cancel-layer', function () {
                $(document).trigger('click');

                return false;
            });

            // Enable notifications
            if (window.webkitNotifications && window.webkitNotifications.checkPermission() == 1) {
                window.webkitNotifications.requestPermission();
            }

        }

        /**
         * Load template from server, then parse and cache the template on the client-side
         */

        function loadTemplate() {
            $.ajax({
                type: 'GET',
                url: f.config.systemConfig.webRoot + '/tpl/tpl.html?__v=' + f.config.systemConfig.version,
                async: false
            }).done(function (data, textStatus, jqXHR) {
                parseTemplate(String(data));
            }).fail(function () {});
        }

        /**
         * Parse and cache the client-side template
         *
         * @param  {string} tplText Template text value
         */

        function parseTemplate(tplText) {
            var tplSettings = [];
            var pattern = /<(script|tpl)[^>]*id=['"]([\w-\.]*)['"][^>]*>([\s\S]*?)<\/\1>/g;
            var result;

            while ((result = pattern.exec(tplText)) != null) {
                tplSettings.push({
                    id: result[2],
                    tpl: result[3]
                });
            }

            var i = tplSettings.length;

            while (i--) {
                var tpl = tplSettings[i];

                f.tpl.register(tpl.id, tpl.tpl);
            }
        }

        /**
         * The entry of system, initializing template, system and register hash change event etc.
         */

        function init() {
            loadTemplate();
            initBBS();
            $('#body').opoa({
                baseUri: f.config.systemConfig.baseUri,
                defaultPage: f.config.systemConfig.defaultPage,
                version: f.config.systemConfig.version,
                beforeClose: function () {
                    $(document.body).loading('show');
                },
                onclose: function () {
                    if (f.bbsPage) {
                        f.bbsPage.close();
                    }
                },
                onfail: function () {
                    document.title = $.isMobile.any ? f.config.systemConfig.defaultShortTitle : f.config.systemConfig.defaultTitle;
                    $(document.body).loading('hide');
                    $('#main').html('<p class="loading-error">载入失败，请刷新重试。</p>');
                },
                afterClose: function (event, data) {
                    var url = data.url;
                    var pageInfo = f.config.pageInfo;
                    var sideNav = $(':data(ui-sidenav)');
                    var menuNav = $(':data(ui-topbar)');
                    var globalTip = $('#global-tip');
                    var title = pageInfo.title || f.config.systemConfig.defaultTitle;
                    var shortTitle = pageInfo.shortTitle || f.config.systemConfig.defaultShortTitle;

                    document.title = $.isMobile.any ? shortTitle : title;

                    $(document.body).loading('hide');

                    globalTip.tip('close');

                    /*
                    if (pageInfo.isShowSideNav) {
                        sideNav.sideNav({
                            isShow: 1
                        });

                        if (pageInfo.isRenderSideNav) {
                            sideNav.sideNav('render');
                        }
                    } else {
                        sideNav.sideNav({
                            isShow: 0
                        });
                    }
                    */

                    if (pageInfo.menuNavId) {
                        menuNav.topbar('setActivate', pageInfo.menuNavId);
                    }

                    // ie6,7提示
                    if ($.browser.msie && ~~$.browser.version < 8) {
                        globalTip.tip('append', {
                            id: 'browser-alert',
                            boxClass: 'warning',
                            content: f.format('提示：本系统不支持IE6、IE7浏览器！请升级浏览器。<a target="_target" href="!{firefox}">Firefox浏览器下载</a>&#12288;|&#12288;<a target="_target" href="!{chrome}">Chrome浏览器下载</a>', {
                                chrome: 'http://www.google.cn/intl/zh-CN/chrome/browser/',
                                firefox: 'http://firefox.com.cn/download/'
                            })
                        });
                    }
                }
            });
        }

        return {
            init: init,
            bbsPage: null
        };

    })());
}(jQuery, f));
