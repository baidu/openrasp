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

function checkContext(context) {
    function assert(flag) {
        if (!flag) {
            throw new PluginError(flag);
        }
    }
    assert(context.method);
    assert(context.protocol === 'http/1.1');
    assert(context.parameter.test.indexOf('a') !== -1 && context.parameter.test.indexOf('b') !== -1);
    assert(context.server);
    assert(context.url);
    assert(context.path);
    assert(context.header['test-test'] === 'Test-Test');
    assert(context.querystring);
    assert(context.remoteAddr);
}

plugin.register('request', function (params, context) {
    plugin.log('request', params);
    if (/request/.test(context.path)) {
        throw new Attack();
    }
});

plugin.register('directory', function (params, context) {
    checkContext(context);
    plugin.log('directory', params);
    if (params.path === '/etc') {
        throw new Attack();
    }
});

plugin.register('readFile', function (params, context) {
    checkContext(context);
    plugin.log('readFile', params);
    if (params.path === '/etc/passwd') {
        throw new Attack();
    }
});

plugin.register('writeFile', function (params, context) {
    checkContext(context);
    plugin.log('writeFile', params);
    if (params.name === 'writeFileTest') {
        throw new Attack();
    }
});

plugin.register('fileUpload', function (params, context) {
    checkContext(context);
    plugin.log('fileUpload', params);
    if (context.body) {
        throw new Attack();
    }
});

plugin.register('sql', function (params, context) {
    checkContext(context);
    plugin.log('sql', params);
    if (params.query === 'SELECT * FROM user') {
        throw new Attack();
    }
});

plugin.register('command', function (params, context) {
    checkContext(context);
    plugin.log('command', params);
    if (params.command[0] === 'pwd') {
        throw new Attack();
    }
});

plugin.register('xxe', function (params, context) {
    checkContext(context);
    plugin.log('xxe', params);
    if (params.entity.endsWith('/etc/passwd')) {
        throw new Attack();
    }
});

plugin.register('ognl', function (params, context) {
    checkContext(context);
    plugin.log('ognl', params);
    if (params.expression === 'java.lang.Runtime') {
        throw new Attack();
    }
});

plugin.register('deserialization', function (params, context) {
    checkContext(context);
    plugin.log('deserialization', params);
    if (params.clazz === 'sun.reflect.annotation.AnnotationInvocationHandler') {
        throw new Attack();
    }
});

plugin.log('初始化成功');
