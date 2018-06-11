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
                if (e instanceof Attack) {
                    result.action = 'block';
                    result.message = e.stack;
                } else {
                    console.log(e.stack);
                    result.action = 'ignore';
                }
            }
            result = typeof(result) === 'object' ? result : {};
            result.action = result.action || 'ignore';
            result.message = result.message || '';
            result.name = result.name || checkProcess.plugin.name;
            result.confidence = result.confidence || 0;
            return result;
        });
        return results;
    }

    static clean() {
        Object.keys(RASP.plugins).forEach(key => {
            delete RASP.plugins[key];
        })
        Object.keys(global)
            .filter(key => key.startsWith('CheckPoint'))
            .forEach(key => {
                RASP.checkPoints[global[key].name] = [];
            });
    }

    static sql_tokenize(query) {
        if (sql_tokenize) {
            return sql_tokenize(query)
        } else {
            return []
        }
    }

    static get_jsengine() {
        return 'v8'
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
RASP.clean();

Object.defineProperty(global, 'RASP', {
    value: RASP,
    enumerable: true
});
