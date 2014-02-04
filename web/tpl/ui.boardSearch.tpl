<tpl id="ui.boardSearch.tpl">
    <div class="ui-board-search">
        <div class="ui-board-search-input-container">
            <input type="search" class="ui-board-search-input" placeholder="搜索版面" x-webkit-speech x-webkit-grammar="builtin:translate" autocomplete="off" />
        </div>
        <div class="ui-board-search-list-container"></div>
    </div>
</tpl>
<tpl id="ui.boardSearchList.tpl">
    <ul class="ui-board-search-list">
        <li class="ui-board-search-list-label">
            <label>搜索结果</label>
            <span class="ui-board-search-list-reset">清空</span>
        </li>
        if (this.list && this.list.length) {
            f.each(this.list, function (item) {
                <li class="ui-board-search-item">
                    <a href="#{item.url}" class="menu-toggle ui-board-search-target">
                        <div class="ui-board-search-zh">#{item.fullName}</div>
                        <div class="ui-board-search-en">#{item.name}</div>
                    </a>
                </li>
            });
        }
        else {
            <li class="ui-board-search-empty-tip">
                <a href="javascript:void(0);" class="ui-board-search-target">无结果</a>
            </li>
        }
    </ul>
</tpl>
