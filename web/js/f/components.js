/**
 * @fileOverview Initializing components
 * @author pzh
 */

f.namespace('f.components');

(function ($, f) {
    /**
     * 获取事件名
     *
     * @private
     * @param {string} name 事件名称
     * @param {string} prefix 事件名称前缀
     *
     * @return {string} 拼装后的事件名称
     */
    var _getChannel = function (name, prefix) {
        return prefix ? prefix + name : "onchange" + name;
    };

    f.extend(f.components, {
        Ajax: function (thisValue) {
            f.extend(thisValue, {
                /**
                 * 封装ajax操作，添加回调事件支持
                 *
                 * @public
                 * @param  {Object} method   方法对象
                 * @config {Object} postData 发送请求时的数据
                 * @config {*}      extData  根据业务需求，发送请求时自行添加的额外数据
                 */
                ajax: function (method, postData, extData) {
                    postData = postData || {};
                    f.extend(postData, {
                        method: method.url
                    });

                    //触发正在请求中事件……
                    this.trigger(_getChannel(method.name, 'on'), {
                        postData: postData,
                        extData: extData
                    });

                    $.jsonPost(this.systemConfig.ajaxUri, postData, function (data, status, msg) {
                        // whether the data adpter exist?
                        if (f.isString(method.adapter) && f.isFunction(thisValue.DataAdapter[method.adapter])) {
                            data.status = status;
                            thisValue.trigger(_getChannel(f.capitalize(method.adapter), 'before'), data);
                            data = thisValue.DataAdapter[method.adapter](data, thisValue);
                            thisValue.trigger(_getChannel(f.capitalize(method.adapter), 'after'), data);
                        }

                        thisValue.trigger(_getChannel(method.name + 'Success', 'on'), data, {
                            status: status,
                            postData: postData,
                            extData: extData,
                            msg: msg
                        });
                    }, function (msg, data) {
                        thisValue.trigger(_getChannel(method.name + 'Failed', "on"), msg, {
                            data: data,
                            postData: postData,
                            extData: extData
                        });
                    });
                }
            });
        },
        Button: function (thisValue) {
            $('.button').button();
        },
        Breadcrumb: function (thisValue) {
            f.extend(thisValue, {
                _initBreadcrumb: function (params) {
                    this[params.id] = $('#breadcrumb').breadcrumb({
                        items: thisValue.breadcrumbList
                    }).breadcrumb('instance');
                }
            });
        },
        ToggleTarget: function (thisValue) {
            $('.toggleable').toggleTarget({
                onchange: function (event, data) {
                    thisValue.trigger(_getChannel('ToggleTarget'), data);
                    $(window).trigger('resize');
                }
            });
        },
        ScrollTarget: function (thisValue) {
            $('.scroll').scrollTarget();
        },
        DataAdapter: function (thisValue) {
            thisValue['DataAdapter'] = new f.dataAdapter();
            f.extend(thisValue, {

            });
        }
    });

    f.extend(f.components, {

    });
}(jQuery, f));
