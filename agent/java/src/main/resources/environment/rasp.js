/**
 * @file
 */
'use strict';
/* globals Attack */

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var RASP = function () {
    function RASP(name) {
        _classCallCheck(this, RASP);

        if (typeof name !== 'string' || name.length == 0) {
            throw new TypeError('Plugin name must be a string');
        }
        this.name = name;
        RASP.plugins[name] = this;
    }

    _createClass(RASP, [{
        key: 'register',
        value: function register(checkPoint, checkProcess) {
            if (typeof checkPoint !== 'string' || checkPoint.length == 0) {
                throw new TypeError('Check point name must be a string');
            }
            if (!RASP.checkPoints[checkPoint]) {
                this.log('Unknown check point name \'' + checkPoint + '\'');
                return;
            }
            if (typeof checkProcess !== 'function') {
                throw new TypeError('Check process must be a function');
            }
            RASP.checkPoints[checkPoint].push({
                func: checkProcess,
                plugin: this
            });
        }
    }, {
        key: 'log',
        value: function log() {
            var len = arguments.length;
            var args = Array(len);
            for (var key = 0; key < len; key++) {
                args[key] = arguments[key];
            }
            console.log.apply(console, ['[' + this.name + ']'].concat(args));
        }
    }, {
        key: 'request',
        value: function request() {}
    }, {
        key: 'getCache',
        value: function getCache() {}
    }, {
        key: 'setCache',
        value: function setCache() {}
    }], [{
        key: 'check',
        value: function check(checkPoint, checkParams, checkContext) {
            if (typeof checkPoint !== 'string' || checkPoint.length == 0) {
                throw new TypeError('Check point name must be a string');
            }
            if (!RASP.checkPoints[checkPoint]) {
                throw new Error('Unknown check point name \'' + checkPoint + '\'');
            }
            var results = RASP.checkPoints[checkPoint].map(function (checkProcess) {
                var result = {};
                try {
                    result = checkProcess.func(checkParams, checkContext);
                } catch (e) {
                    if (e instanceof Attack) {
                        result.action = 'block';
                        result.message = e.message + '\n' + e.stack;
                    } else {
                        console.log(e.toString(), '\n', e.stack);
                        result.action = 'ignore';
                    }
                }
                result = (typeof result === 'undefined' ? 'undefined' : _typeof(result)) === 'object' ? result : {};
                result.action = result.action || 'ignore';
                result.message = result.message || '';
                result.name = result.name || checkProcess.plugin.name;
                result.confidence = result.confidence || 0;
                return result;
            });
            return results;
        }
    }, {
        key: 'clean',
        value: function clean() {
            Object.keys(RASP.plugins).forEach(function (key) {
                delete RASP.plugins[key];
            });
            Object.keys(global).filter(function (key) {
                return key.startsWith('CheckPoint');
            }).forEach(function (key) {
                RASP.checkPoints[global[key].name] = [];
            });
        }
    }]);

    return RASP;
}();
RASP.plugins = {};
RASP.checkPoints = {};
RASP.clean();

Object.defineProperty(global, 'RASP', {
    value: RASP,
    enumerable: true
});