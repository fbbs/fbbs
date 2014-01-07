/**
 * @fileOverview Page: in charge of page business logic representing report and setting page
 * @author pzh
 */

(function (f) {
    'use strict';

    f.page = f.Class( /**@lends f.page.prototype */ {
        //callbacks
        beforeInit: f.noop,
        oninit: f.noop,
        beforeInitComponents: f.noop,
        afterInitComponents: f.noop,
        onclose: f.noop,
        /**
         * Page: in charge of page business logic
         *
         * @constructs
         * @garmmer new f.page(components, options) or
         *          new f.page({components: [xx,..], ....}) or
         *          new f.page('Ajax', 'Filter', options) or
         *          new f.page('*', options)
         * @param  {Array} components array of components
         * @param  {Object} options arguments object
         */
        initialize: function (components, options) {
            var args = [].slice.call(arguments);

            options = args.pop();
            f.extend(this, options);

            // components can be passed as an array or as individual parameter
            if (!this.components) {
                this.components = (args[0] && typeof args[0] === 'string') ? args : args[0];
            }

            this.init();
        },
        /**
         * Init page
         *
         * @public
         */
        init: function () {
            f.extend(this, f.config);

            //默认添加ajax、dataAdaper、login组件
            f.components['Ajax'](this);
            f.components['DataAdapter'](this);
            this._mediator = Mediator();

            this.trigger("beforeInit");

            this.trigger("beforeInitComponents");
            this._initComponents();
            this.trigger("afterInitComponents");

            this.trigger("oninit");
        },
        /**
         * Attach an event handler function for one custom channel
         *
         * @public
         * @grammar on(channel, callback)
         * @param   {string}   channel   custom event channel name
         * @param   {Function} handler A function to execute when the event is triggered.
         *
         * @return {Object} this
         */
        on: function (channel, callback) {
            this._mediator.subscribe.apply(this._mediator, [].slice.call(arguments));

            return this;
        },
        /**
         * Remove an event handler
         *
         * @public
         * @grammer off(channel, identifier)
         * @param   {string}   channel   custom event channel name
         * @param   {Function} handler A handler function previously attached for the event
         *
         * @return {Object} this
         */
        off: function (channel, identifier) {
            this._mediator.off.apply(this._mediator, [].slice.call(arguments));

            return this;
        },
        /**
         * Execute all handlers and behaviors attached to the given event channel type.
         *
         * @public
         * @grammar trigger(channel, data)
         * @param  {string} channel A string containing a JavaScript event type
         * @param  {Object} data Additional parameters to pass along to the event handler
         *
         * @return {Object} this
         */
        trigger: function (channel, data) {
            if (f.isFunction(this[channel])) {
                this[channel].apply(this, [].slice.call(arguments, 1));
            }

            this._mediator.publish.apply(this._mediator, [].slice.call(arguments));

            return this;
        },
        /**
         * Callback when initializing components
         *
         * @public
         * @param  {[Array]} components array of components
         */
        oninitComponents: function (components) {
            if (f.isArray(components)) {
                components.push('Button', 'ToggleTarget', 'ScrollTarget');
            }
        },
        /**
         * Callback when page close
         *
         * @public
         */
        close: function () {
            // we need to unbind any custom events that our view raises
            this._mediator.removeAll();
            // and also we need to unbind any ui events that our view is bound to, such as this.ui.bind('onchange', f.noop)
            this.trigger('onclose');
        },
        /**
         * Dispose
         *
         * @public
         */
        dispose: function () {
            for (var property in this) {
                if (!f.isFunction(this[property])) {
                    delete this[property];
                }
            }

            this.disposed = true;
        },
        /**
         * Initialize components
         *
         * @private
         */
        _initComponents: function () {
            var components = this.components;

            // no components or '*' means use all components
            if (!components || f.indexOf(components, '*') !== -1) {
                components = [];
                f.each(f.components, function (fn, component) {
                    components.push(component);
                });
            }

            this.trigger("oninitComponents", components);

            for (var i = 0, l = components.length; i < l; i++) {
                var component = components[i];
                var id = null;
                var type = null;
                var options = null;

                if (f.isString(component)) {
                    type = component;
                    id = type;
                    options = {};
                } else if (f.isObject(component)) {
                    type = component.type;
                    options = component.options;
                    id = component.id ? component.id : type;
                }

                var initType = "_init" + type;
                var params = {
                    id: id,
                    type: type,
                    options: options
                };

                if (!this[params.id] && f.components[type]) {
                    f.components[type](this);
                }

                if (f.isFunction(this[initType])) {

                    this[initType](params);
                }
            }
        },
        /**
         * Page history back
         *
         * @private
         */
        _back: function () {
            window.history.back();
        }
    });
}(f));
