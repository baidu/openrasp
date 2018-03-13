/**
 * @file test plugin
 */
/* globals RASP, Attack, PluginError */
'use strict';
var plugin = new RASP('test');

var clean = {
    action: 'ignore',
    message: '无风险'
};

function checkContext(context, paramInit, hookName) {
    function assert(flag) {
        if (!flag) {
            throw new PluginError(flag);
        }
    }

    if (hookName === 'request') {
        if (/param-encoding/.test(context.path)) {
            assert(context.parameter.test)
        } else {
            assert(!context.parameter.test)
        }
    } else {
        assert(context.method);
        assert(context.protocol === 'http/1.1');
        assert(context.server);
        assert(context.url);
        assert(context.path);
        if (paramInit === true) {
            assert(context.parameter.test[0] === 'a' && context.parameter.test[1] === 'b');
        } else {
            assert(JSON.stringify(context.parameter) === "{}");
        }
        assert(context.header['test-test'] === 'Test-Test');
        assert(context.querystring);
        assert(context.remoteAddr);
    }
}

plugin.register('request', function (params, context) {
    checkContext(context, true, 'request');
    plugin.log('request', params);
    if (/request/.test(context.path) || /param-encoding/.test(context.path)) {
        return {
            action: 'block'
        }
    }
});

plugin.register('directory', function (params, context) {
    plugin.log('directory', params);
    if (params.path === '/etc') {
        return {
            action: 'block'
        }
    }
});

plugin.register('readFile', function (params, context) {
    checkContext(context);
    plugin.log('readFile', params);
    if (params.path === '/etc/passwd') {
        return {
            action: 'block'
        }
    }
});

plugin.register('writeFile', function (params, context) {
    checkContext(context);
    plugin.log('writeFile', params);
    if (params.name === 'writeFileTest') {
        return {
            action: 'block'
        }
    }
});

plugin.register('fileUpload', function (params, context) {
    checkContext(context);
    plugin.log('fileUpload', params);
    if (context.body) {
        return {
            action: 'block'
        }
    }
});

plugin.register('sql', function (params, context) {
    checkContext(context);
    plugin.log('sql', params);
    if (params.query === 'SELECT * FROM user') {
        return {
            action: 'block'
        }
    }
});

plugin.register('command', function (params, context) {
    checkContext(context);
    plugin.log('command', params);
    if (params.command[0] === 'pwd') {
        return {
            action: 'block'
        }
    }
});

plugin.register('xxe', function (params, context) {
    checkContext(context, true);
    plugin.log('xxe', params);
    if (params.entity.endsWith('/etc/passwd')) {
        return {
            action: 'block'
        }
    }
});

plugin.register('ognl', function (params, context) {
    checkContext(context);
    plugin.log('ognl', params);
    if (params.expression === "@org.apache.commons.io.IOUtils@toString(@java.lang.Runtime@getRuntime().exec('whoami').getInputStream())") {
        return {
            action: 'block'
        }
    }
});

plugin.register('deserialization', function (params, context) {
    checkContext(context);
    plugin.log('deserialization', params);
    if (params.clazz === 'sun.reflect.annotation.AnnotationInvocationHandler') {
        return {
            action: 'block'
        }
    }
});

plugin.register('include', function (params, context) {
    checkContext(context);
    plugin.log('include', params);
    if (/passwd/.test(params.url)) {
        return {
            action: 'block'
        }
    }
});

plugin.register('ssrf', function (params, context) {
    checkContext(context);
    plugin.log('ssrf', params);
    if (params.hostname === '0x7f.0x0.0x0.0x1' && params.url === 'http://0x7f.0x0.0x0.0x1:8080/app') {
        return {
            action: 'block'
        }
    }
});

plugin.log('初始化成功');
