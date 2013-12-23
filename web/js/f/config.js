/**
 * @fileOverview All configurations of system
 * @author pzh
 */

/**
 * @namespace f.config Including pageInfo, userInfo, systemConfig, etc..
 */
f.namespace('f.config');

f.config.systemConfig = {
    version: '201312130001',
    webRoot: '/web',
    baseUri: '/web',
    defaultPage: 'index.html',
    ajaxUri: '/web/ajax'
};

/**
 * @namespace f.zIndex 全局z-index管理
 */
f.namespace('f.zIndex');

f.zIndex.scroll = 9999; // 随屏滚动的z-index范围为8889-9999;
f.zIndex.ui = 10000; // ui的z-index的z-index范围
f.zIndex.tooltip = 11000;
f.zIndex.mask = 12000;

/**
 * Global mediator object used to subscribe/publish custom events
 *
 * @public
 */
f.mediator = Mediator();
