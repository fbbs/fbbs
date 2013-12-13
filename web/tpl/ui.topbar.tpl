<tpl id="ui.topbar.tpl">
var menuList = f.config.menuList;
var userInfo = f.config.userInfo;
<nav class="top-bar" data-topbar>
    <ul class="title-area">
        <li class="name">
            if (menuList.title) {
                <h1><a href="#{menuList.title.url}">#{menuList.title.label}</a></h1>
            }
        </li>
    </ul>
    <section class="top-bar-section">
        <ul class="right">
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
                    <a href="javascript:void(0);" class="button login">登陆</a>
                </li>
            }
        </ul>
        <ul class="left">
            f.each(menuList.menus, function (menu) {
                <li class="nav" data-menu-nav-id="#{menu.id}"><a href="#{menu.url}">#{menu.label}</a></li>
            });
        </ul>
    </section>
</nav>
</tpl>
