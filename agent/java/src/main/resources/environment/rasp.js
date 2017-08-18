/**
 * @file
 */
'use strict';
/* globals Attack */
const RASP = class {
    constructor(name) {
        if (typeof(name) !== 'string' || name.length == 0) {
            throw new TypeError('Plugin name must be a string');
        }
        this.name = name;
        RASP.plugins[name] = this;
    }

    static check(checkPoint, checkParams, checkContext) {
        if (typeof(checkPoint) !== 'string' || checkPoint.length == 0) {
            throw new TypeError('Check point name must be a string');
        }
        if (!RASP.checkPoints[checkPoint]) {
            throw new Error('Unknown check point name \'' + checkPoint + '\'');
        }
        let results = RASP.checkPoints[checkPoint].map(checkProcess => {
            let result = {};
            try {
                result = checkProcess.func(checkParams, checkContext);
            } catch (e) {
                result.message = e.message;
                result.action = e instanceof Attack ? 'block' : 'log';
            }
            result = typeof(result) === 'object' ? result : {};
            result.action = result.action || 'log';
            result.message = result.message || '';
            result.name = result.name || checkProcess.plugin.name;
            return result;
        });
        return results;
    }

    static clean() {
        Object.keys(RASP.plugins)
            .forEach(key => {
                delete RASP.plugins[key];
            });
        Object.keys(RASP.checkPoints)
            .forEach(key => {
                delete RASP.checkPoints[key];
            });
    }

    register(checkPoint, checkProcess) {
        if (typeof(checkPoint) !== 'string' || checkPoint.length == 0) {
            throw new TypeError('Check point name must be a string');
        }
        if (!RASP.checkPoints[checkPoint]) {
            this.log('Unknown check point name \'' + checkPoint + '\'');
            return;
        }
        if (typeof(checkProcess) !== 'function') {
            throw new TypeError('Check process must be a function');
        }
        RASP.checkPoints[checkPoint].push({
            func: checkProcess,
            plugin: this
        });
    }

    log() {
        let len = arguments.length;
        let args = Array(len);
        for (let key = 0; key < len; key++) {
            args[key] = arguments[key];
        }
        console.log.apply(console, ['[' + this.name + ']'].concat(args));
    }

    request() {}

    getCache() {}

    setCache() {}
};
RASP.plugins = {};
RASP.checkPoints = {};
Object.keys(global)
    .filter(key => key.startsWith('CheckPoint'))
    .forEach(key => {
        RASP.checkPoints[global[key].name] = [];
    });
Object.freeze(RASP);

Object.defineProperty(global, 'RASP', {
    value: RASP,
    enumerable: true
});
