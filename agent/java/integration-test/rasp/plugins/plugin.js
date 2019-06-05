/**
 * @file test plugin
 */
/* globals RASP, Attack, PluginError */
'use strict';
var plugin = new RASP('test');

function checkContext(context) {
    function assert(flag) {
        if (!flag) {
            throw new Error();
        }
    }

    assert(context.method);
    assert(context.protocol === 'http/1.1');
    assert(context.server);
    assert(context.url);
    assert(context.path);
    assert(context.parameter.test[0] === 'a' && context.parameter.test[1] === 'b');
    assert(context.header['test-test'] === 'Test-Test');
    assert(context.querystring);
    assert(context.remoteAddr);
}

plugin.register('request', function (params, context) {
    if (/request/.test(context.path)) {
        plugin.log('request', params);
        return {
            action: 'block'
        }
    }
});

plugin.register('request', function (params, context) {
    if (/param-encoding/.test(context.path)) {
        plugin.log('param-encoding', params, context);
        checkContext(context);
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
    if (params.path === 'writeFileTest') {
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
    plugin.log('sql', params);
    if (context.protocol === 'dubbo') {
        if (params.query === 'SELECT * FROM user') {
            return {
                action: 'block'
            }
        }
    } else {
        if (params.query === 'SELECT * FROM user') {
            return {
                action: 'block'
            }
        }
    }
});

plugin.register('command', function (params, context) {
    checkContext(context);
    plugin.log('command', params, context);
    if (params.command === 'pwd') {
        return {
            action: 'block'
        }
    }
});

plugin.register('xxe', function (params, context) {
    checkContext(context);
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
    if (params.expression === "@org.apache.commons.io.IOUtils@toString(@java.lang.Runtime@getRuntime().exec(\"whoami\").getInputStream())") {
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
    plugin.log('include', params);
    if (/passwd/.test(params.url)) {
        return {
            action: 'block'
        }
    }
});

plugin.register('ssrf', function (params, context) {
    plugin.log('ssrf', params);
    if (params.hostname === '0x7f.0x0.0x0.0x1' && params.url === 'http://0x7f.0x0.0x0.0x1:8080/app') {
        return {
            action: 'block'
        }
    }
});

plugin.log('初始化成功');
