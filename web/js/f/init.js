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
                },
                urls: {
                    userInfoUrl: f.config.urlConfig.userInfo,
                    logoutUrl: f.config.urlConfig.logout,
                    loginUrl: f.config.urlConfig.login,
                    mailUrl: f.config.urlConfig.mail
                }
            });

            // Render footer
            $('#footer').html(f.tpl.format('ui.footer.tpl', {
                copyright: f.config.systemConfig.copyright,
                menu: f.config.menuList.footerMenus,
                icp: f.config.systemConfig.icp
            }));

            // Render global tip
            $('#global-tip').tip();

            // Initialize notification
            $(document.body).notifications({
                notificationUrl: [f.config.systemConfig.ajaxUri, '/user/notifications.json'].join(""),
                getInterval: f.config.systemConfig.notificationInterval,
                // Check if logged in
                ongetNotification: function () {
                    return !!(f.config.userInfo.user && f.config.userInfo.token);
                },
                // Get notifications
                ongetNotificationSuccess: function (event, data) {
                    var notified = [],
                        newMsg = false,
                        list = {
                            mail: '您有#{0}封未读信件。'
                        };
                    f.each(data, function (value, key) {
                        var noti = $('.notifications').find(['.', key].join("")),
                            icon = noti.find('.notification-icon'),
                            label = noti.find('.notification-label'),
                            text = label.text();
                        // If there are new notifications
                        if (value) {
                            newMsg = true;
                            // Set url when get new messages
                            noti.addClass('notified')
                                .attr('href', f.config.urlConfig[['new', f.capitalize(key)].join("")]);
                            // Show number of messages
                            label.text(value);
                            // If more new messages
                            if (~~text < value) {
                                // Save notification text
                                notified.push(f.format(list[key], value));
                                notified.push('\n');
                            }
                            // Show animation
                            icon.addClass('tada');
                            setTimeout(function () {
                                icon.removeClass('tada');
                            }, 1000);
                        }
                        else {
                            noti.removeClass('notified')
                                .attr('href', f.config.urlConfig[key]);
                            label.text(value);
                        }
                    });

                    // Set small notification dot
                    var smallIcon = $('.left-off-canvas-toggle i');
                    if (newMsg) {
                        if (smallIcon.hasClass('notified')) {
                            smallIcon.addClass('pulse');
                            setTimeout(function () {
                                smallIcon.removeClass('pulse');
                            }, 1000);
                        }
                        smallIcon.addClass('notified');
                    }
                    else {
                        smallIcon.removeClass('notified');
                    }

                    // Check hidden prop of window
                    var getHiddenProp = function () {
                        var prefixes = ['webkit','moz','ms','o'];

                        // if 'hidden' is natively supported just return it
                        if ('hidden' in document) {
                            return 'hidden';
                        }

                        // otherwise loop over all the known prefixes until we find one
                        for (var i = 0; i < prefixes.length; i++) {
                            if ((prefixes[i] + 'Hidden') in document) {
                                return prefixes[i] + 'Hidden';
                            }
                        }

                        // otherwise it's not supported
                        return null;
                    };

                    // Return if the window is hidden
                    var isHidden = function () {
                        var prop = getHiddenProp();
                        if (!prop) {
                            return false;
                        }

                        return document[prop];
                    };

                    // If the window is hidden and there are new notifications, display them
                    if (isHidden() && notified.length) {
                        if (window.webkitNotifications && window.webkitNotifications.checkPermission() == 0) {
                            var notification = webkitNotifications.createNotification(
                                [f.config.systemConfig.webRoot, '/images/apple-touch-icon-iphone.png'].join(""),
                                f.config.systemConfig.defaultTitle,
                                notified.join("")
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

            // Show loading
            $(document.body).loading();

            // history back click event handler
            $('.body').on('click', '.history-back', function () {
                window.history.back();
            });

            // hide layer click event handler
            $('.body').on('click', '.cancel-layer', function () {
                $(document).trigger('click');

                return false;
            });

            // Enable notifications
            $('.notifications').on('click', function () {
                if (window.webkitNotifications && window.webkitNotifications.checkPermission() == 1) {
                    window.webkitNotifications.requestPermission();
                }
            });

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

            // Load saved page
            var defaultPage = f.config.systemConfig.defaultPage;
            if (window.localStorage) {
                var idleTime = parseInt(window.localStorage.getItem('idleTime'));
                var time = (new Date()).getTime();
                if (time - idleTime <= f.config.systemConfig.idleTime) {
                    defaultPage = window.localStorage.getItem('idlePage') || defaultPage;
                }
            }
            $('#body').opoa({
                baseUri: f.config.systemConfig.baseUri,
                defaultPage: defaultPage,
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
                    var hash = ['#', $.param.fragment()].join("");
                    var pageInfo = f.config.pageInfo;
                    var sideNav = $(':data(ui-sidenav)');
                    var menuNav = $(':data(ui-topbar)');
                    var globalTip = $('#global-tip');
                    var title = pageInfo.title || f.config.systemConfig.defaultTitle;
                    var shortTitle = pageInfo.shortTitle || f.config.systemConfig.defaultShortTitle;

                    // Set title
                    document.title = $.isMobile.any ? shortTitle : title;

                    // Hide loading
                    $(document.body).loading('hide');

                    // Close all tips
                    globalTip.tip('close');

                    // Reset notifications
                    $(document.body).notifications('reset');

                    // Change login url
                    if (f.config.pageInfo.pageName !== 'login') {
                        menuNav.topbar('setUrl', {
                            selector: '.menu-toggle-login',
                            url: [f.config.urlConfig.login, '?r=', encodeURIComponent(hash)].join("")
                        });
                    }

                    // Set active menu nav
                    if (pageInfo.menuNavId) {
                        menuNav.topbar('setActivate', pageInfo.menuNavId);
                    }

                    // ie6,7提示
                    if (($.browser.msie && parseInt($.browser.version) < 8) || ($.browser.webkit && parseInt($.browser.version) < 5) || ($.browser.opera && parseInt($.browser.version) < 11) || ($.browser.mozilla && parseInt($.browser.version) < 4)) {
                        globalTip.tip('append', {
                            id: 'browser-alert',
                            boxClass: 'warning',
                            content: f.format('提示：本系统不支持IE6、IE7浏览器！请升级浏览器。<a target="_target" href="!{firefox}">Firefox浏览器下载</a>&#12288;|&#12288;<a target="_target" href="!{chrome}">Chrome浏览器下载</a>', {
                                chrome: 'http://www.google.cn/intl/zh-CN/chrome/browser/',
                                firefox: 'http://firefox.com.cn/download/'
                            })
                        });
                    }
                    if ($.isMobile.iOS && $.browser.safari && parseInt($.browser.version) < 534) {
                        globalTip.tip('append', {
                            id: 'ios-alert',
                            boxClass: 'warning',
                            content: '您的iOS系统版本过低，无法使用本系统。'
                        });
                    }
                    if ($.isMobile.Android && $.browser.webkit && parseInt($.browser.version) < 534) {
                        globalTip.tip('append', {
                            id: 'ios-alert',
                            boxClass: 'warning',
                            content: '您的浏览器版本过低，无法使用本系统。'
                        });
                    }

                    // Save idle status
                    if (window.localStorage) {
                        window.localStorage.setItem('idlePage', hash);
                        window.localStorage.setItem('idleTime', (new Date()).getTime());
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
