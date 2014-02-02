<tpl id="ui.topbar.tpl">
var menuList = f.config.menuList;
var userInfo = f.config.userInfo;
var pageInfo = f.config.pageInfo;
var systemConfig = f.config.systemConfig;
<nav class="ui-top-bar">
    <div class="wrapper">
        <section class="left-small hide-for-desktop">
            <a class="left-off-canvas-toggle canvas-toggle menu-icon"><span></span><i></i></a>
        </section>
        <ul class="title-area">
            <li class="name">
                if (menuList.title) {
                    <h1>
                        <a class="menu-toggle hide-for-mobile" href="#{menuList.title.url}">#{menuList.title.label}</a>
                        <a class="canvas-toggle hide-for-desktop" href="javascript:void(0);">#{menuList.title.label}</a>
                    </h1>
                }
            </li>
        </ul>
        <section class="top-bar-section clearfix hide-for-mobile">
            <ul class="left">
                f.each(menuList.menus, function (menu) {
                    <li class="nav" data-menu-nav-id="#{menu.id}"><a class="menu-toggle" href="#{menu.url}">#{menu.label}</a></li>
                });
            </ul>
            <ul class="right notifications">
                if ('object' === typeof userInfo && !f.isEmpty(userInfo)) {
                    <li class="divider"></li>
                    <li class="has-dropdown">
                        <a href="#{this.userInfoUrl}" title="#{userInfo.user}" class="dropdown-a">#{f.prune(userInfo.user, 10)}<span class="dropdown-arrow">&nbsp;</span></a>
                        <ul class="dropdown">
                            f.each(menuList.userMenus, function (subMenu) {
                                <li class="dropdown-list" data-dropdown-menu-id="#{subMenu.id}">
                                    <a href="#{subMenu.url}" class="dropdown-menu">#{subMenu.label}</a>
                                </li>
                            });
                            if (menuList.userMenus.length) {
                                <li class="divider"></li>
                            }
                            <li class="dropdown-list">
                                <a href="#{this.logoutUrl}" class="dropdown-menu logout">退出</a>
                            </li>
                        </ul>
                    </li>
                    <li class="divider"></li>
                    <li>
                        <a href="#{this.mailUrl}" title="查看信件" class="notification mail"><span class="notification-icon">&nbsp;</span><span class="notification-label">0</span></a>
                    </li>
                }
                else {
                    <li class="has-form">
                        <a href="javascript:void(0);" class="button login menu-toggle menu-toggle-new">登陆</a>
                    </li>
                }
            </ul>
        </section>
    </div>
    <aside class="left-off-canvas-menu hide-for-desktop">
        <ul class="off-canvas-list">
            if ('object' === typeof userInfo && !f.isEmpty(userInfo)) {
                <li>
                    <a href="#{this.userInfoUrl}" title="#{userInfo.user}" class="menu-toggle">#{f.prune(userInfo.user, 10)}</a>
                    <div class="off-canvas-notification notifications">
                        <a href="#{this.mailUrl}" title="查看信件" class="notification mail menu-toggle"><span class="notification-icon">&nbsp;</span><span class="notification-label">0</span></a>
                    </div>
                </li>
            }
            else {
                <li><a class="menu-toggle menu-toggle-login" href="#{this.loginUrl}">登陆</a></li>
            }
            <li><label>导航</label></li>
            if (menuList.title) {
                <li><a class="menu-toggle" href="#{menuList.title.url}">#{menuList.title.label}</a></li>
            }
            f.each(menuList.menus, function (menu) {
                <li><a class="menu-toggle" href="#{menu.url}">#{menu.label}</a></li>
            });
            if ('object' === typeof userInfo && !f.isEmpty(userInfo)) {
                <li><label>用户</label></li>
                f.each(menuList.userMenus, function (subMenu) {
                    <li><a href="#{subMenu.url}" class="menu-toggle">#{subMenu.label}</a></li>
                });
                <li><a href="#{this.logoutUrl}" class="menu-toggle logout">退出</a></li>
            }
            <li><label>其它</label></li>
            f.each(menuList.footerMenus, function (menu) {
                <li><a class="menu-toggle" href="#{menu.url}" target="#{menu.target ? '_blank' : '_self'}">#{menu.label}</a></li>
            });
        </ul>
        <div class="off-canvas-footer">!#{systemConfig.copyright}&emsp;<a href="#{systemConfig.icp.url}" target="_blank">#{systemConfig.icp.label}</a></div>
    </aside>
    <a class="exit-canvas-menu" href="javascript:void(0);"></a>
</nav>
</tpl>
