/**
 * @file
 */
'use strict';

const Attack = class extends Error {
    constructor(msg) {
        super(msg);
        Object.defineProperty(this, 'name', {
            value: 'Attack'
        });
        Error.captureStackTrace(this, Attack);
    }
};

const PluginError = class extends Error {
    constructor(msg) {
        super(msg);
        Object.defineProperty(this, 'name', {
            value: 'PluginError'
        });
        Error.captureStackTrace(this, Attack);
    }
};

Object.defineProperty(global, 'Attack', {
    value: Attack,
    enumerable: true
});
Object.defineProperty(global, 'PluginError', {
    value: PluginError,
    enumerable: true
});
