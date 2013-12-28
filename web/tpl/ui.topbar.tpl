<tpl id="ui.topbar.tpl">
var menuList = f.config.menuList;
var userInfo = f.config.userInfo;
var pageInfo = f.config.pageInfo;
<nav class="top-bar" data-topbar>
    <ul class="title-area">
        <li class="name">
            if (menuList.title) {
                <h1><a class="menu-nav" href="#{menuList.title.url}">#{menuList.title.label}</a></h1>
            }
        </li>
        <li class="toggle-topbar menu-icon"><a href="javascript:void(0);"><span></span></a></li>
    </ul>
    <section class="top-bar-section">
        <ul class="left">
            f.each(menuList.menus, function (menu) {
                if (menu.id === pageInfo.pageName) {
                    <li class="nav active" data-menu-nav-id="#{menu.id}"><a class="menu-nav" href="#{menu.url}">#{menu.label}</a></li>
                }
                else {
                    <li class="nav" data-menu-nav-id="#{menu.id}"><a class="menu-nav" href="#{menu.url}">#{menu.label}</a></li>
                }
            });
        </ul>
        <ul class="right">
            <li class="divider"></li>
            if (userInfo && userInfo.username) {
                <li class="has-dropdown">
                    <a href="#">Right Button with Dropdown</a>
                    <ul class="dropdown">
                        <li><a href="#">First link in dropdown</a></li>
                    </ul>
                </li>
            }
            else {
                <li class="has-form">
                    <a href="javascript:void(0);" class="button login menu-nav menu-nav-new">登陆</a>
                </li>
            }
        </ul>
    </section>
</nav>
</tpl>
