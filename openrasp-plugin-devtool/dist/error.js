/**
 * @file
 */
'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var Attack = function (_Error) {
    _inherits(Attack, _Error);

    function Attack(msg) {
        _classCallCheck(this, Attack);

        var _this = _possibleConstructorReturn(this, (Attack.__proto__ || Object.getPrototypeOf(Attack)).call(this, msg));

        Object.defineProperty(_this, 'name', {
            value: 'Attack'
        });
        Error.captureStackTrace(_this, Attack);
        return _this;
    }

    return Attack;
}(Error);

var PluginError = function (_Error2) {
    _inherits(PluginError, _Error2);

    function PluginError(msg) {
        _classCallCheck(this, PluginError);

        var _this2 = _possibleConstructorReturn(this, (PluginError.__proto__ || Object.getPrototypeOf(PluginError)).call(this, msg));

        Object.defineProperty(_this2, 'name', {
            value: 'PluginError'
        });
        Error.captureStackTrace(_this2, Attack);
        return _this2;
    }

    return PluginError;
}(Error);

Object.defineProperty(global, 'Attack', {
    value: Attack,
    enumerable: true
});
Object.defineProperty(global, 'PluginError', {
    value: PluginError,
    enumerable: true
});