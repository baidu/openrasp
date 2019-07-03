const plugin_version = '2018-1000-1000'
const plugin_name    = 'identify-file-writer'

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

plugin.register('readFile', function (params, context) {
    return clean
})

// 检查谁写入了 abc.js，并打印报警
plugin.register('writeFile', function (params, context) {
    if (params.realpath.indexOf('abc.js') != -1) {
        return {
            action:  'log',
            message: 'Opened ' + params.realpath + ' for writing'
        }
    }

    return clean
})

plugin.log('004-identify-file-writer: plugin loaded')


