// 本插件用于测试拦截效果
// 
// 这个插件的逻辑是，不管请求是否正常，一律拦截
// 若要开启这个插件，请先删除下面的 return :-)
return

'use strict'
var plugin  = new RASP('block-all-test')

const default_action = {
    action:     'block',
    message:    '- 插件全部拦截测试 -',
    confidence: 90
}

plugin.register('sql', function (params, context) {
    return default_action
})

plugin.register('ssrf', function (params, context) {
    return default_action
})

plugin.register('directory', function (params, context) {
    return default_action
})

plugin.register('readFile', function (params, context) {
    return default_action
})

plugin.register('webdav', function (params, context) {
    return default_action
})

plugin.register('include', function (params, context) {
    return default_action
})

plugin.register('writeFile', function (params, context) {
    return default_action
})

plugin.register('fileUpload', function (params, context) {
    return default_action
})

plugin.register('command', function (params, context) {
    return default_action
})

// 注意: PHP 不支持XXE检测
plugin.register('xxe', function (params, context) {
    return default_action
})

// 默认情况下，当OGNL表达式长度超过30才会进入检测点，此长度可配置
plugin.register('ognl', function (params, context) {
    return default_action
})

// [[ 近期调整~ ]]
plugin.register('deserialization', function (params, context) {
    return default_action
})

plugin.log('全部拦截插件测试: 初始化成功')
