const plugin_version = '2019-1220-1800'
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

// BEGIN ALGORITHM CONFIG //

var algorithmConfig = {
    // 防止前端报错
    meta: {},
    iast: {
        fuzz_server:     'http://127.0.0.1:25931/openrasp-result',
        request_timeout: 5000,
        byhost_regex:    '.*'
    }
}

// END ALGORITHM CONFIG //

var byhost_regex
if (algorithmConfig.iast.byhost_regex.length > 0){
    byhost_regex = new RegExp(byhost_regex)
}

var ip_regex = new RegExp(String.raw`^(\d{1,3}\.){3}\d{1,3}$`)

function bufferToHex (buffer) {
    return Array.from (new Uint8Array (buffer)).map (b => b.toString (16).padStart (2, "0")).join ("");
}

function get_stack_hash (stack) {
    var s = stack.join(",")
    var hashes = Array(4);
    hashes.fill(0)

    for (let i = 0; i < s.length; i += 1) {
        hashes[i%4] = hashes[i%4] ^ s.charCodeAt(i);
        hashes[i%4] = ((hashes[i%4] >> 24 ) | (hashes[i%4] << 8))
    }

    var ret = ""
    for (let i = 0; i < hashes.length; i += 1) {
        ret += (hashes[i] >>> 0).toString(16).padStart(8, "0")
    }

    return ret;
}

function add_hook(hook_type, params, context) {
    if (context.header == null) {
        return
    }
    if ( context.header["scan-request-id"] != undefined) {
        if (is_scanning_hook(hook_type, params, context)) {
            params.stack = params.stack
        }
        else {
            return
        }
    }
    else {
        params.stack = get_stack_hash(params.stack)
    }

    if (context.hook_info == undefined) {
        context.hook_info = []
    }
    params.hook_type = hook_type
    context.hook_info.push(params)
}

function is_scanning_hook(hook_type, params, context) {
    /*
    [
        {
            "type": "sql", 
            "filter": [
                {
                    "query": "openrasp"
                },
                ...
            ]
        },
        ...
    ]
    */

    if (context.filter === undefined) {
        try {
            let filter_ascii = context.header["x-iast-filter"]
            context.filter = JSON.parse(unescape(filter_ascii))
        }
        catch (e) {
            context.filter = false
            return true
        }
    }

    if (context.filter === false) {
        return true
    }

    for (let item of Object.values(context.filter)) {
        if (item.type == hook_type) {
            for (let [key, value] of Object.entries(item.filter)) {
                if (params[key] !== undefined && params[key].indexOf(value) != -1) {
                    return true
                }
            }
        }
    }
    return false
}

function send_rasp_result(context) {

    if (context.header == null) {
        return
    }

    var hook_info  = context.hook_info || []
    delete context.hook_info
    delete context.filter

    // 不检测不包含hook_info的请求, xml类型除外
    if (hook_info.length == 0 && 
        context.header["scan-request-id"] === undefined && 
        context.header["content-type"] != undefined &&
        context.header["content-type"].indexOf("application/xml") < 0) {
        return
    }

    var default_port
    if (context.url.toLowerCase().startsWith("https")) {
        default_port = 443
    }
    else {
        default_port = 80
    }

    // 构建 context
    var new_context             = Object.assign({}, context)
    new_context.json            = new_context.json || {}
    new_context.parameter       = new_context.parameter || {}
    new_context.querystring     = new_context.querystring || ""

    if (context.header["scan-request-id"] === undefined) {
        new_context.body = bufferToHex(context.body).substr(0, 200)
    }
    else{
        new_context.body = ""
    }

    var web_server = {}
    var server_host = new_context.header.host
    if (!server_host) {
        msg = "Agent with rasp id:" + context.raspId + " get host from http header failed! "
        plugin.log(msg)
        return
    }
    else {
        if (byhost_regex && byhost_regex.test(server_host)) {
            server_host = server_host.split(":")
            web_server.host = server_host[0]
            web_server.port = parseInt(server_host[1]) || default_port
        }
        else {
            server_host = server_host.split(":")
            for (var i in context.nic) {
                if (context.nic[i].ip && 
                    ip_regex.test(context.nic[i].ip) && 
                    context.nic[i].ip != "127.0.0.1") 
                {
                    web_server.host = context.nic[i].ip
                    break
                }
            }
            if (!web_server.host) {
                msg = "Agent with rasp id:" + context.raspId + " get ip failed! "
                plugin.log(msg)
                return
            }
            else {
                web_server.port = parseInt(server_host[1]) || default_port
            }
        }
    }

    // 将hook点信息发送给扫描服务器
    var data = {
        "web_server":     web_server,
        "context":        new_context,
        "hook_info":      hook_info,
        "plugin_version": plugin_version
    }

    var request_config = {
        "method":       "post",
        "data":         data,
        "timeout":      algorithmConfig['iast']['timeout'],
        "maxRedirects": 0,
        "headers": {
            "content-type": "application/json"
        },        
    }
    if (context.header["scan-request-server"] !== undefined) {
        request_config.url = context.header["scan-request-server"]
    }
    else {
        request_config.url = algorithmConfig['iast']['fuzz_server']
    }

    // console.log("send to:", algorithmConfig['iast']['fuzz_server'])
    RASP.request(request_config).catch(console.log)
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
    if (context.url) {
        params.tokens = RASP.cmd_tokenize(params.command)
        add_hook('command', params, context)
    }
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

plugin.register('mongodb', function (params, context) {
    add_hook('mongodb', params, context)
})

plugin.register('deleteFile', function (params, context) {
    add_hook('deleteFile', params, context)
})

plugin.register('request', function (params, context) {
})

plugin.register('requestEnd', function (params, context) {
    send_rasp_result(context)
})

plugin.log('OpenRASP IAST plugin: Initialized, version', plugin_version)

