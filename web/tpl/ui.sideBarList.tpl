<tpl id="ui.sideBarList.tpl">
    <ul class="ui-side-bar-list #{this.className}">
        <li class="ui-side-bar-list-label">
            <label>#{this.label}</label>
            if (this.rightLabel) {
                <span class="ui-side-bar-list-reset">清空</span>
            }
        </li>
        if (this.list && this.list.length) {
            f.each(this.list, function (item) {
                <li class="ui-side-bar-list-item">
                    <a href="#{item.url}" class="menu-toggle ui-side-bar-list-target">
                        <div class="ui-side-bar-list-zh">#{item.fullName}</div>
                        if (item.name) {
                            <div class="ui-side-bar-list-en">#{item.name}</div>
                        }
                    </a>
                </li>
            });
        }
        else if (this.emptyTip) {
            <li class="ui-side-bar-list-empty-tip">
                <a href="javascript:void(0);" class="ui-side-bar-list-target">!#{this.emptyTip}</a>
            </li>
        }
    </ul>
</tpl>
