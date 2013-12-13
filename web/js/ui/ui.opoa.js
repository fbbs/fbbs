/**
 * @fileOverview One page one application widget
 * @author pzh
 */

(function ($) {
    'use strict';

    /**
     * Local storage of html page corresponding with url
     *
     * @private
     * @type {Object}
     */
    var pageCache = {};

    $.widget("ui.opoa", {
        version: '1.0.0',
        options: {
            defaultPage: null,
            baseUri: '',
            isCache: false,
            contentSelector: '#main',
            styleSelector: 'link[rel="stylesheet"]:not([data-persist="true"])',
            scriptSelector: 'script:not([data-persist="true"])'
        },
        /**
         * Widget constructor
         *
         * @private
         */
        _create: function () {
            this.window
                .on('statechange', $.proxy(this, '_onstatechange'))
                .trigger('statechange');
            $(document).on('click', 'a', $.proxy(this, '_onAnchorClick'));
        },
        _onAnchorClick: function (event) {
            event.preventDefault();
            var $target = $(event.target);
            var url = $target.attr('href');
            var title = window.document.title;
            History.pushState({
                url: url,
                data: $target.attr('data-state')
            }, title, url);
        },
        _onstatechange: function (event) {
            var state = History.getState();
            var url = state.data.url;
            var title = window.document.title;

            if (!url || url.indexOf('?') !== 0) {
                url = this.options.defaultPage;
                History.pushState({
                    url: url
                }, title, url);

                return;
            }

            url = this.options.baseUri + url.replace(/^\?/, '/') + '.html';

            var isUpdatePage = this._trigger("beforeClose", null, {
                url: url
            });

            // it it returns false, and then stop update page
            if (isUpdatePage) {
                // if the cache exists and we can get it, so update page using cache
                if (this.options.isCache && pageCache[url]) {
                    this._updatePage(url);
                } else {
                    this._loadPage(url);
                }
            }
        },
        /**
         * Callback handler after loading page successfully
         *
         * @private
         * @param  {string} url  page url string
         * @param  {string} html page html value
         */
        _onloadPageSuccess: function (url, html) {
            this._jqXHR = null;
            pageCache[url] = $.trim(html);

            this._updatePage(url);
        },
        /**
         * Callback handler after loading page fail: hide loading tip and load default page
         *
         * @private
         */
        _onloadPageFail: function () {
            this._trigger('onfail', null);
        },
        /**
         * Loading page
         *
         * @private
         * @param  {string} url page url
         */
        _loadPage: function (url) {
            // if the current request is loading, and then we need to abort that request
            if (this._jqXHR) {
                this._jqXHR.abort();
            }

            this._jqXHR = $.get(url)
                .done($.proxy(this, '_onloadPageSuccess', url))
                .fail($.proxy(this, '_onloadPageFail'));
        },
        /**
         * Update page
         *
         * @private
         * @param  {string} url page url
         */
        _updatePage: function (url) {
            var $doc = $('<div>');
            var main = this.options.contentSelector;

            this._trigger('onclose', null, {
                url: url
            });

            //$doc.html(pageCache[url]);
            // because of `tmp.innerHTML = wrap[1] + elem.replace( rxhtmlTag, "<$1></$2>" ) + wrap[2]`;
            $doc.get(0).innerHTML = pageCache[url];

            $(this.options.styleSelector).remove();
            $doc.find(this.options.styleSelector).appendTo('head');

            //$(main).html($doc.find(main).html());
            // because of `tmp.innerHTML = wrap[1] + elem.replace( rxhtmlTag, "<$1></$2>" ) + wrap[2]` of jQuery;
            $(main).empty(); // we need to remove all child nodes , as well as everything inside it.

            $(main).get(0).innerHTML = $doc.find(main).html();

            $(this.options.scriptSelector).remove();
            $doc.find(this.options.scriptSelector).appendTo('body');

            this._trigger("afterClose", null, {
                url: url
            });
        }
    });
}(jQuery));
