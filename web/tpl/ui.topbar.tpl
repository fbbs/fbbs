<tpl id="ui.topbar.tpl">
var menuList = f.config.menuList;
var userInfo = f.config.userInfo;
var pageInfo = f.config.pageInfo;
<nav class="ui-top-bar">
    <section class="left-small hide-for-desktop">
        <a class="left-off-canvas-toggle canvas-toggle menu-icon"><span></span></a>
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
    <section class="top-bar-section hide-for-mobile">
        <ul class="left">
            f.each(menuList.menus, function (menu) {
                <li class="nav" data-menu-nav-id="#{menu.id}"><a class="menu-toggle" href="#{menu.url}">#{menu.label}</a></li>
            });
        </ul>
        <ul class="right">
            <li class="divider"></li>
            if ('object' === typeof userInfo && !f.isEmpty(userInfo)) {
                <li class="has-dropdown">
                    <a href="#">Right Button with Dropdown</a>
                    <ul class="dropdown">
                        <li><a href="#">First link in dropdown</a></li>
                    </ul>
                </li>
            }
            else {
                <li class="has-form">
                    <a href="javascript:void(0);" class="button login menu-toggle menu-toggle-new">登陆</a>
                </li>
            }
        </ul>
    </section>
    <aside class="left-off-canvas-menu hide-for-desktop">
        <ul class="off-canvas-list">
            <li><label>导航</label></li>
            if (menuList.title) {
                <li><a class="menu-toggle" href="#{menuList.title.url}">#{menuList.title.label}</a></li>
            }
            f.each(menuList.menus, function (menu) {
                <li><a class="menu-toggle" href="#{menu.url}">#{menu.label}</a></li>
            });
            <li><label>用户</label></li>
            if ('object' === typeof userInfo && !f.isEmpty(userInfo)) {

            }
            else {
                <li><a class="menu-toggle menu-toggle-new login" href="javascript:void(0);">登陆</a></li>
            }
        </ul>
    </aside>
    <a class="exit-canvas-menu" href="javascript:void(0);"></a>
</nav>
</tpl>
