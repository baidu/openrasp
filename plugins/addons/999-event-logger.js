//
// OpenRASP 事件记录插件 - DEMO
// 

'use strict'
var plugin  = new RASP('event-logger')

const clean = {
    action:     'ignore',
    message:    '无风险',
    confidence: 0
}

plugin.register('directory', function (params, context) {
    plugin.log('列出目录: ' + params.realpath)
    return clean
})

plugin.register('webdav', function (params, context) {
    plugin.log('使用WEBDAV操作文件: ', context.method, params.source, params.dest)
    return clean
})

plugin.register('fileUpload', function (params, context) {
    plugin.log('文件上传: ' + params.filename)
    return clean
})

plugin.register('command', function (params, context) {
    plugin.log('命令执行: ' + params.command)
    return clean
})

// 为了提高性能，只有当OGNL表达式长度超过30时，才会调用插件
// 这个30可以配置，aka "ognl.expression.minlength"
// https://rasp.baidu.com/doc/setup/others.html
plugin.register('ognl', function (params, context) {
    plugin.log('执行OGNL表达式: ' + params.expression)
    return clean
})

// 下面的这些方法，可能产生大量日志
plugin.register('xxe', function (params, context) {
    plugin.log('读取XML外部实体: ' + params.entity)
    return clean
})

plugin.register('include', function (params, context) {
    plugin.log('文件包含: ' + params.url)
    return clean
})

plugin.register('readFile', function (params, context) {
    plugin.log('读取文件: ' + params.realpath)
    return clean
})

plugin.register('writeFile', function (params, context) {
    plugin.log('文件写入: ' + params.realpath)
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
    plugin.log('SQL查询: ' + normalize_query(params.query))
    return clean
})

plugin.log('999-event-logger: 初始化成功')


