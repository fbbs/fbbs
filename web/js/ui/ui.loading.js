(function ($, f) {
    'use strict';

    $.widget('ui.loading', {
        options: {
            loadingText: '加载中，请稍候...',

            // tpl
            tpl: ' \
                    <div class="ui-loading" id="loading" style="display:none"> \
                        <div class="loading-content"> \
                            <div class="loading-inner"> \
                                <div class="loading-left-shadow"></div> \
                                <span class="loading-img"></span> \
                                <div class="loading-text">加载中，请稍候...</div> \
                            </div> \
                        </div> \
                    </div> \
            '
        },
        /**
         * widget 构造器
         *
         * @private
         */
        _create: function () {
            this.element.append(f.trim(this.options.tpl));
            this._$loading = $('#loading');
            this._$loadingText = this._$loading.find('.loading-text');
            this._mask = $(document).mask({
                zIndex: f.zIndex.mask
            });
        },
        /**
         * 显示loading
         *
         * @public
         */
        show: function () {
            this.setText();
            this._mask.mask('show');
            this._$loading.show();

            if ($.browser.ie && $.browser.version < 7) {
                this._$loading.css('top', '120');
            }
        },

        /**
         * 隐藏loading
         *
         * @public
         */
        hide: function () {
            this._mask.mask('remove');
            this._$loading.hide();
        },
        /**
         * 设置loading文字
         *
         * @public
         * @param {string} text 提示文字
         */
        setText: function (text) {
            this._$loadingText.text(text || this.options.loadingText);
        }
    });
}(jQuery, f));
