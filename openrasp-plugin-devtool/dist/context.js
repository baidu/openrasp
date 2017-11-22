/**
 * @file
 */
'use strict';

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var Context = function () {
    function Context(data) {
        _classCallCheck(this, Context);

        Object.assign(this, data);
    }

    _createClass(Context, [{
        key: 'path',
        get: function get() {
            if (this.getPath) {
                return this.getPath();
            }
        }
    }, {
        key: 'url',
        get: function get() {
            if (this.getUrl) {
                return this.getUrl();
            }
        }
    }, {
        key: 'querystring',
        get: function get() {
            if (this.getQuerystring) {
                return this.getQuerystring();
            }
        }
    }, {
        key: 'method',
        get: function get() {
            if (this.getMethod) {
                return this.getMethod();
            }
        }
    }, {
        key: 'protocol',
        get: function get() {
            if (this.getProtocol) {
                return this.getProtocol();
            }
        }
    }, {
        key: 'header',
        get: function get() {
            if (this.getHeader) {
                return this.getHeader();
            }
        }
    }, {
        key: 'parameter',
        get: function get() {
            if (this.getParameter) {
                return this.getParameter();
            }
        }
    }, {
        key: 'body',
        get: function get() {
            if (this.getBody) {
                return this.getBody();
            }
        }
    }, {
        key: 'remoteAddr',
        get: function get() {
            if (this.getRemoteAddr) {
                return this.getRemoteAddr();
            }
        }
    }, {
        key: 'server',
        get: function get() {
            if (this.getServer) {
                return this.getServer();
            }
        }
    }]);

    return Context;
}();

Object.defineProperty(global, 'Context', {
    value: Context,
    enumerable: true
});