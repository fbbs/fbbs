/**
 * Constructor: f.Class
 * Base class used to construct all other classes. Includes support for
 *     multiple inheritance.
 *
 * To create a new class, use the following syntax:
 * (code)
 *     var MyClass = f.Class(prototype);
 * (end)
 *
 * To create a new class with multiple inheritance, use the
 *     following syntax:
 * (code)
 *     var MyClass = f.Class(Class1, Class2, prototype);
 * (end)
 *
 * Note that instanceof reflection will only reveal Class1 as superclass.
 *
 */
f.Class = function () {
    var len = arguments.length;
    //superclass
    var P = arguments[0];
    //prototype
    var F = arguments[len - 1];

    //constructor
    var C = typeof F.initialize == "function" ? F.initialize : function () {
            P.prototype.initialize.apply(this, arguments);
        };

    if (len > 1) { //inheritance
        var newArgs = [C, P].concat(
        Array.prototype.slice.call(arguments).slice(1, len - 1), F);
        f.inherit.apply(null, newArgs);
    } else {
        C.prototype = F;
    }

    //extend
    C.extend = function (components) {
        if (!'[object Array]' == Object.prototype.toString.call(components)) {
            components = Array.prototype.concat.call(components);
        }
        var i = components.length;
        while (i--) {
            components[i].call(C.prototype);
        }
        return C;
    };

    return C;
};

/**
 * Function: f.inherit
 *
 * Parameters:
 * C - {Object} the class that inherits
 * P - {Object} the superclass to inherit from
 *
 * In addition to the mandatory C and P parameters, an arbitrary number of
 * objects can be passed, which will extend C.
 */
f.inherit = function (C, P) {
    var F = function () {};
    F.prototype = P.prototype;
    C.prototype = new F;
    var i, l, o;
    for (i = 2, l = arguments.length; i < l; i++) {
        o = arguments[i];
        if (typeof o === "function") {
            o = o.prototype;
        }
        f.util.extend(C.prototype, o);
    }
};

/**
 * @namespace f.util 工具类库的命名空间
 */
f.namespace('f.util');

/**
 * APIFunction: extend
 * Copy all properties of a source object to a destination object.  Modifies
 *     the passed in destination object.  Any properties on the source object
 *     that are set to undefined will not be (re)set on the destination object.
 *
 * Parameters:
 * destination - {Object} The object that will be modified
 * source - {Object} The object with properties to be set on the destination
 *
 * Returns:
 * @return {Object} The destination object.
 */
f.util.extend = function (destination, source) {
    destination = destination || {};
    if (source) {
        for (var property in source) {
            var value = source[property];
            if (value !== undefined) {
                destination[property] = value;
            }
        }

        /**
         * IE doesn't include the toString property when iterating over an object's
         * properties with the for(property in object) syntax.  Explicitly check if
         * the source has its own toString property.
         */

        /*
         * FF/Windows < 2.0.0.13 reports "Illegal operation on WrappedNative
         * prototype object" when calling hawOwnProperty if the source object
         * is an instance of window.Event.
         */

        var sourceIsEvt = typeof window.Event == "function" && source instanceof window.Event;

        if (!sourceIsEvt && source.hasOwnProperty && source.hasOwnProperty("toString")) {
            destination.toString = source.toString;
        }
    }
    return destination;
};

/**
 * 这是一个空函数，用于需要排除函数作用域链干扰的情况.
 *
 * @name f.util.blank
 * @function
 * @grammar f.util.blank()
 *
 * @return {Function} 一个空函数
 */
f.util.blank = function () {};
