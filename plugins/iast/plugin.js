const plugin_version = '2019-0723-1700'
const plugin_name    = 'iast'
const plugin_desc    = 'IAST Fuzz 插件'

/*
 * Copyright 2017-2019 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


'use strict'
var plugin = new RASP(plugin_name)

// 仅在 golang 下面会缓存多个请求信息，其他语言不受影响
var cache  = {}

// BEGIN ALGORITHM CONFIG //

var algorithmConfig = {
    // 防止前端报错
    meta: {},
    iast: {
        fuzz_server:     'http://127.0.0.1:25931/openrasp-result',
        request_timeout: 5000,
        byhost_regex:    ''  
    }
}

// END ALGORITHM CONFIG //

var byhost_regex = new RegExp(algorithmConfig.iast.byhost_regex)

function bufferToHex (buffer) {
    return Array.from (new Uint8Array (buffer)).map (b => b.toString (16).padStart (2, "0")).join ("");
}

function add_hook(hook_type, params, context) {
    var request_id = context.requestId

    if (! cache[request_id]) {
        cache[request_id] = []
    }

    params.hook_type = hook_type
    cache[request_id].push(params)
}

function send_rasp_result(context) {

    var request_id = context.requestId
    var hook_info  = cache[request_id] || []
    delete cache[request_id]

    var default_port
    if (context.url.toLowerCase().startsWith("https")) {
        default_port = 443
    }
    else {
        default_port = 80
    }

    // 不检测不包含hook_info的请求, xml类型除外
    if (hook_info.length == 0 && 
        context.header["scan-request-id"] === undefined && 
        context.header["content-type"] != undefined &&
        context.header["content-type"].indexOf("application/xml") < 0) {
        return
    }

    // 构建 context
    var new_context             = Object.assign({}, context)
    new_context.json            = new_context.json || {}
    new_context.body            = new_context.body || ""
    new_context.parameter       = new_context.parameter || {}
    new_context.querystring     = new_context.querystring || ""
    new_context.body            = bufferToHex(context.body)

    var web_server = {}
    var server_host = new_context.header.host || "unknow_server_addr"
    if (byhost_regex.test(server_host)) {
        server_host = server_host.split(":")
        web_server.host = server_host[0]
        web_server.port = parseInt(server_host[1]) || default_port
    }
    else {
        server_host = server_host.split(":")
        web_server.host = context.nic.ip
        web_server.port = parseInt(server_host[1]) || default_port
    }

    // 将hook点信息发送给扫描服务器
    var data = {
        "web_server":   web_server,
        "context":      new_context,
        "hook_info":    hook_info
    }

    var request_config = {
        "method":       "post",
        "data":         data,
        "url":          algorithmConfig['iast']['fuzz_server'],
        "timeout":      algorithmConfig['iast']['timeout'],
        "maxRedirects": 0,
        "headers": {
            "content-type": "application/json"
        },        
    }
    // console.log("send to:", algorithmConfig['iast']['fuzz_server'])
    RASP.request(request_config)
}

plugin.register('sql', function (params, context) {
    params.tokens = RASP.sql_tokenize(params.query)
    add_hook("sql", params, context)
})

plugin.register('ssrf', function (params, context) {
    add_hook('ssrf', params, context)
})

plugin.register('directory', function (params, context) {
    add_hook('directory', params, context)
})

plugin.register('readFile', function (params, context) {
    add_hook('readFile', params, context)
})

plugin.register('include', function (params, context) {
    add_hook('include', params, context)
})

plugin.register('writeFile', function (params, context) {
    add_hook('writeFile', params, context)
})

plugin.register('fileUpload', function (params, context) {
    add_hook('fileUpload', params, context)
})

plugin.register('webdav', function (params, context) {
    add_hook('webdav', params, context)
})

plugin.register('rename', function (params, context) {
    add_hook('rename', params, context)
})

plugin.register('command', function (params, context) {
    if (! context.url) {
        return clean
    }

    params.tokens = RASP.cmd_tokenize(params.command)
    add_hook('command', params, context)
})

plugin.register('xxe', function (params, context) {
    add_hook('xxe', params, context)
})

plugin.register('ognl', function (params, context) {
    add_hook('ognl', params, context)
})

plugin.register('deserialization', function (params, context) {
    add_hook('deserialization', params, context)
})

plugin.register('eval', function (params, context) {
    add_hook('eval', params, context)
})

plugin.register('requestEnd', function (params, context) {
    send_rasp_result(context)
})

plugin.log('OpenRASP IAST plugin: Initialized, version', plugin_version)

