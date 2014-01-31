/**
 * @fileOverview notifications
 * @author pzh
 */

(function ($, f) {
    'use strict';

    $.widget("ui.notifications", {
        version: '1.0.0',
        options: {
            ongetNotification: null,
            ongetNotificationSuccess: null,
            ongetNotificationFailed: null,
            getInterval: 60000,
            notificationUrl: 'notifications.json'
        },
        _create: function () {
            this.reset();
        },
        _getNotification: function () {
            var me = this;
            var login = this._trigger('ongetNotification', null);
            if (!login) {
                return;
            }
            $.ajax({
                type: 'GET',
                url: this.options.notificationUrl,
                cache: false
            }).done(function (data, textStatus, jqXHR) {
                me._trigger('ongetNotificationSuccess', null, data.data);
            }).fail(function (jqXHR, textStatus, error) {
                me._trigger('ongetNotificationFailed', null, textStatus);
            });
        },
        reset: function () {
            this._interval && clearInterval(this._interval);
            this._getNotification();
            this._interval = setInterval($.proxy(this, '_getNotification'), this.options.getInterval);
        }
    });

}(jQuery, f));
