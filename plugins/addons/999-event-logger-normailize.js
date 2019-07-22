const plugin_version = '2018-1000-1000'
const plugin_name    = 'event-logger-normalize'

//
// OpenRASP plugin: event logger - normalize SQL phrase
// 

'use strict'
var plugin  = new RASP(plugin_name)

const clean = {
    action:     'ignore',
    message:    'Looks fine to me',
    confidence: 0
}

// BEGIN ALGORITHM CONFIG //

var algorithmConfig = {}

// END ALGORITHM CONFIG //

plugin.register('directory', function (params, context) {
    plugin.log('Listing directory content: ' + params.realpath)
    return clean
})

plugin.register('webdav', function (params, context) {
    plugin.log('Webdav operation: ', context.method, params.source, params.dest)
    return clean
})

plugin.register('fileUpload', function (params, context) {
    plugin.log('File upload: ' + params.filename)
    return clean
})

 plugin.register('rename', function (params, context) {
    plugin.log('Rename file - From ' + params.source + ' to ' + params.dest)  
    return clean
})

plugin.register('command', function (params, context) {
    plugin.log('Execute command: ' + params.command)
    return clean
})

// 为了提高性能，只有当OGNL表达式长度超过30时，才会调用插件
// 这个30可以配置，aka "ognl.expression.minlength"
// https://rasp.baidu.com/doc/setup/others.html
plugin.register('ognl', function (params, context) {
    plugin.log('Evaluating OGNL expression: ' + params.expression)
    return clean
})

// 下面的这些方法，可能产生大量日志
plugin.register('xxe', function (params, context) {
    plugin.log('Loading XML entity: ' + params.entity)
    return clean
})

plugin.register('include', function (params, context) {
    plugin.log('Include file: ' + params.url)
    return clean
})

plugin.register('readFile', function (params, context) {
    plugin.log('Read file: ' + params.realpath)
    return clean
})

plugin.register('writeFile', function (params, context) {
    plugin.log('Write file: ' + params.realpath)
    return clean
})

function normalize_query(query) {
    var tokens = RASP.sql_tokenize(query)
    for (var i = 0; i < tokens.length; i ++) {
        var token = tokens[i]

        // 检查是否为字符串
        if ( (token[0] == "'" || token[0] == '"') &&
            (token[token.length - 1] == "'" || token[token.length - 1] == '"'))
        {
            tokens[i] = '"S"'
        }
    }

    return tokens.join(' ')
}

// 记录SQL日志，可能带来如下两种问题
// 1. 查询语句中，可能包含敏感信息
// 2. 日志量可能会很大
plugin.register('sql', function (params, context) {
    plugin.log('SQL query: ' + normalize_query(params.query))
    return clean
})

plugin.log('999-event-logger: plugin loaded')


