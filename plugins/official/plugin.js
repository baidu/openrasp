//
// OpenRASP 官方插件已经覆盖了一定的攻击场景，具体检测能力请根据业务来定制
// 如果想了解具体能检测哪些攻击，覆盖哪些已知漏洞，请参考下面两个链接
//
// Web 攻击检测能力说明、零规则检测算法介绍
// https://rasp.baidu.com/doc/usage/web.html
//
// CVE 漏洞覆盖说明
// https://rasp.baidu.com/doc/usage/cve.html
// 
// OpenRASP 最佳实践
// https://rasp.baidu.com/#section-books

'use strict'
var plugin  = new RASP('offical')

const clean = {
    action:     'ignore',
    message:    '无风险',
    confidence: 0
}

var forcefulBrowsing = {
    dotFiles: /\.(7z|tar|gz|bz2|xz|rar|zip|sql|db|sqlite)$/,
    nonUserDirectory: /^\/(proc|sys|root)/,
    unwantedFilenames: [
        // user files
        '.DS_Store',
        'id_rsa', 'id_rsa.pub', 'known_hosts', 'authorized_keys', 
        '.bash_history', '.csh_history', '.zsh_history', '.mysql_history',

        // project files
        '.htaccess', '.user.ini',

        'web.config', 'web.xml', 'build.property.xml', 'bower.json',
        'Gemfile', 'Gemfile.lock',
        '.gitignore',
        'error_log', 'error.log', 'nohup.out',
    ],
    unwantedDirectory: [
        '/',
        '/home',
        '/var/log',
        '/proc',
        '/sys'
    ],
    absolutePaths: [
        '/etc/shadow',
        '/etc/hosts',
        '/etc/apache2/apache2.conf',
        '/root/.bash_history',
        '/root/.bash_profile',
        'C:\\Windows\\system32\\inetsrv\\MetaBase.xml'
    ]
}

var scriptFileRegex = /\.(jspx?|php[345]?|phtml)\.?$/i
var ntfsRegex       = /::\$(DATA|INDEX)$/i // 其他的stream都没啥用

// 常用函数
String.prototype.replaceAll = function(token, tokenValue) {
    var index  = 0;
    var string = this;
    
    do {
        string = string.replace(token, tokenValue);
    } while((index = string.indexOf(token, index + 1)) > -1);

    return string
}

function canonicalPath (path) {
    return path.replaceAll('/./', '/').replaceAll('//', '/').replaceAll('//', '/')
}

function basename (path) {
    var idx = path.lastIndexOf('/')
    return path.substr(idx + 1)
}

function ip2long(ipstr) {
    var items  = ipstr.split('.')
    var result = 0

    for (var i = 0; i < items.length; i ++) {
        result = (result << 8) + parseInt(items[i], 10)
    }
    return result >>> 0

    // 对于 rhino 引擎，function 回调越多，性能越差
    // return ip.split('.').reduce(function(ipInt, octet) { return (ipInt<<8) + parseInt(octet, 10)}, 0) >>> 0;
}

if (RASP.get_jsengine() !== 'v8') {
    // 在java语言下面，为了提高性能，SQLi/SSRF检测逻辑改为原生实现
    // 通过修改这个 algorithmConfig 来控制检测逻辑是否开启

    var algorithmConfig = {
        // SQL注入算法#1 - 匹配用户输入
        'sqli_userinput': {
            action: 'block'
        },
        // SQL注入算法#2 - 语句规范
        'sqli_policy': {
            action:  'block',
            feature: [
                'stacked_query', 
                'no_hex', 
                'constant_compare', 
                'version_comment', 
                'function_blacklist',
                'union_null'
            ],
            function_blacklist: [
                // 文件操作
                'load_file',
                // 时间差注入
                'benchmark', 'pg_sleep', 'sleep',
                // 探测阶段
                'is_srvrolemember',
                // 报错注入
                'updatexml', 'extractvalue',
                // 盲注
                'hex',
                // 可能误报，暂不开启 - 批量验证中
                // 'char', 'chr', 'mid', 'ord', 'ascii'
            ]
        },
        // SSRF - 是否允许访问 aws metadata
        'ssrf_aws': {
            action: 'block'
        },
        // SSRF - 是否允许访问 dnslog 地址
        'ssrf_common': {
            action: 'block'
        },
        // SSRF - 是否允许访问混淆后的IP地址
        'ssrf_obfuscate': {
            action: 'block'
        },
        // SSRF - 是否直接禁止访问内网
        'ssrf_intranet': {
            action: 'ignore'
        }
    }

    RASP.config_set('algorithm.config', JSON.stringify(algorithmConfig))

} else {
    // 对于PHP + V8，性能还不错，我们保留JS检测逻辑

    plugin.register('sql', function (params, context) {
        var reason     = false
        var parameters = context.parameter
        var tokens     = RASP.sql_tokenize(params.query, params.server)

        // console.log(tokens)

        // 算法1: 匹配用户输入
        // 1. 简单识别逻辑是否发生改变
        // 2. 识别数据库管理器   
        if (1) {
            Object.keys(parameters).some(function (name) {
                var value = parameters[name][0]

                // 请求参数长度超过10才考虑
                if (value.length <= 10) {
                    return
                }

                // 判断是否为数据库管理器
                if (value.length == params.query.length && value == params.query) {
                    reason = '算法2: WebShell - 数据库管理器'
                    return true
                }

                // 简单识别用户输入
                if (params.query.indexOf(value) == -1) {
                    return
                }

                // 去掉用户输入再次匹配
                var tokens2 = RASP.sql_tokenize(params.query.replace(value, ''), params.server)
                if (tokens.length - tokens2.length > 2) {
                    reason = '算法1: 数据库查询逻辑发生改变'
                    return true
                }
            })
            if (reason !== false) {
                return {
                    'action':     'block',
                    'confidence': 90,
                    'message':    reason
                }
            }
        }

        // 算法2: SQL语句策略检查（模拟SQL防火墙功能）
        if (1) {
            var func_list = {
                'load_file':        true,
                'benchmark':        true,
                'sleep':            true,
                'pg_sleep':         true,
                'is_srvrolemember': true,
                'updatexml':        true,
                'extractvalue':     true,
                'hex':              true,
            }
            var tokens_lc = tokens.map(v => v.toLowerCase())

            for (var i = 1; i < tokens_lc.length; i ++) {

                if (1 && tokens_lc[i] === 'select') {
                    var null_count = 0

                    // 寻找连续的逗号、NULL或者数字
                    for (var j = i + 1; j < tokens_lc.length && j < i + 6; j ++) {
                        if (tokens_lc[j] === ',' || tokens_lc[j] == 'null' || ! isNaN(parseInt(tokens_lc[j]))) {
                            null_count ++
                        } else {
                            break
                        }
                    }

                    // NULL,NULL,NULL == 5个token
                    // 1,2,3          == 5个token
                    if (null_count >= 5) {
                        reason = 'UNION-NULL 方式注入 - 字段类型探测'
                        break
                    }
                    continue
                }

                if (tokens_lc[i] == ';' && i != tokens_lc.length - 1) {
                    reason = '禁止多语句查询'
                    break
                } else if (tokens_lc[i][0] === '0' && tokens_lc[i][1] === 'x') {
                    reason = '禁止16进制字符串'
                    break
                } else if (tokens_lc[i][0] === '/' && tokens_lc[i][1] === '*' && tokens_lc[i][2] === '!') {
                    reason = '禁止MySQL版本号注释'
                    break
                } else if (i > 0 && i < tokens_lc.length - 1 && 
                    (tokens_lc[i] === 'xor'
                        || tokens_lc[i][0] === '<'
                        || tokens_lc[i][0] === '>' 
                        || tokens_lc[i][0] === '=')) {
                    // @FIXME: 可绕过，暂时不更新
                    // 简单识别 NUMBER (>|<|>=|<=|xor) NUMBER
                    //          i-1         i          i+2    
                        
                    var op1  = tokens_lc[i - 1]
                    var op2  = tokens_lc[i + 1]

                    // @TODO: strip quotes
                    var num1 = parseInt(op1)
                    var num2 = parseInt(op2)

                    if (! isNaN(num1) && ! isNaN(num2)) {
                        // 允许 1=1 这样的常量对比以避免误报
                        // 来自 wooyun-2015-0131345 的截图
                        // 
                        // SQLmap 是随机4位数字，不受影响
                        if (num1 == 1 && num2 == 1 && tokens_lc[i][0] === '=')
                        {
                            continue;
                        }

                        reason = '禁止常量比较操作'
                        break
                    }                    
                } else if (i > 0 && tokens_lc[i][0] === '(') {
                    // @FIXME: 可绕过，暂时不更新
                    if (func_list[tokens_lc[i - 1]]) {
                        reason = '禁止执行敏感函数: ' + tokens_lc[i - 1]
                        break
                    }
                }
            }

            if (reason !== false) {
                return {
                    action:     'block',
                    message:    '算法3: 数据库语句异常: ' + reason,
                    confidence: 100
                }
            }
        }

        // 算法3: 简单正则匹配（即将移除）
        if (0) {
            var sqlRegex = /\bupdatexml\s*\(|\bextractvalue\s*\(|\bunion.*select.*(from|into|benchmark).*\b/i

            if (sqlRegex.test(params.query)) {
                return {
                    action:     'block',
                    message:    'SQL 注入攻击（算法4）',
                    confidence: 100
                }
            }
        }

        return clean
    })


    plugin.register('ssrf', function (params, context) {
        var hostname = params.hostname
        var reason   = false

        // 检查常见探测域名
        if (hostname == 'requestb.in' 
            || hostname.endsWith('.vcap.me') 
            || hostname.endsWith('.xip.name') || hostname.endsWith('.xip.io') || hostname.endsWith('.nip.io') 
            || hostname.endsWith('.burpcollaborator.net')) 
        {
            reason = '访问已知的内网探测域名'    
        } 
        // 检测AWS私有地址，如有需求可注释掉
        else if (1 && hostname == '169.254.169.254') {        
            reason = '尝试读取 AWS metadata'
        }
        // 检查混淆: 
        // http://2130706433
        // 
        // 以下混淆方式没有检测，容易误报
        // http://0x7f.0x0.0x0.0x1
        // http://0x7f.0.0.0    
        else if (Number.isInteger(hostname)) {
            reason = '尝试使用纯数字IP'
        }
        // 检查混淆: 
        // http://0x7f001
        else if (hostname.startsWith('0x') && hostname.indexOf('.') === -1) {
            reason = '尝试使用16进制IP'
        }

        if (reason) {
            return {
                action:    'block',
                message:   'SSRF攻击: ' + reason,
                confidence: 100
            }
        }
        return clean
    })

}

// 主要用于识别webshell里的文件管理器
// 通常程序不会主动列目录或者查看敏感目录，e.g /home /etc /var/log 等等
// 
// 若有特例可调整
// 可结合业务定制: e.g 不能超出应用根目录
plugin.register('directory', function (params, context) {
    var path        = params.path
    var realpath    = params.realpath
    var appBasePath = context.appBasePath

    for (var i = 0; i < forcefulBrowsing.unwantedDirectory.length; i ++) {
        if (realpath == forcefulBrowsing.unwantedDirectory[i]) {
            return {
                action:     'block',
                message:    '疑似WebShell文件管理器 - 读取敏感目录',
                confidence: 100
            }
        }
    }

    if (canonicalPath(path).indexOf('/../../') != -1 && realpath.indexOf(appBasePath) == -1) {
        return {
            action:     'log',
            message:    '尝试列出Web目录以外的目录',
            confidence: 90
        }
    }

    return clean
})

plugin.register('readFile', function (params, context) {

    // 算法1: 检查是否为成功的目录扫描
    var filename_1 = basename(context.url)
    var filename_2 = basename(params.realpath)

    if (filename_1 == filename_2) {
        var matched = false

        // 尝试下载压缩包、SQL文件等等
        if (forcefulBrowsing.dotFiles.test(filename_1)) {
            matched = true
        } else {
            // 尝试访问敏感文件
            for (var i = 0; i < forcefulBrowsing.unwantedFilenames; i ++) {
                if (forcefulBrowsing.unwantedFilenames[i] == filename_1) {
                    matched = true
                }
            }
        }

        if (matched) {
            return {
                action:     'log',
                message:    '尝试下载敏感文件 (' + context.method.toUpperCase() + ' 方式): ' + params.realpath,

                // 如果是HEAD方式下载敏感文件，100% 扫描器攻击
                confidence: context.method == 'head' ? 100 : 90
            }
        }
    }

    // 算法2: 如果使用绝对路径访问敏感文件，判定为 webshell
    if (params.realpath == params.path) {
        for (var j = 0; j < forcefulBrowsing.absolutePaths.length; j ++) {
            if (forcefulBrowsing.absolutePaths[j] == params.realpath) {
                return {
                    action:     'block',
                    message:    'WebShell/文件管理器 - 尝试读取系统文件: ' + params.realpath,
                    confidence: 90
                }
            }
        }
    }

    // 算法3: 检查文件遍历，看是否超出web目录范围
    var path        = canonicalPath(params.path)
    var appBasePath = context.appBasePath

    if (params.realpath.indexOf(appBasePath) == -1 && (path.indexOf('/../') !== -1 || path.indexOf('\\..\\') !== -1)) {
        return {
            action:     'block',
            message:    '目录遍历攻击，跳出web目录范围 (' + appBasePath + ')',
            confidence: 90
        }
    }

    return clean
})

plugin.register('webdav', function (params, context) {
    
    // 源文件不是脚本 && 目标文件是脚本，判定为MOVE方式写后门
    if (! scriptFileRegex.test(params.source) && scriptFileRegex.test(params.dest)) {
        return {
            action:    'block',
            message:   '尝试通过 ' + context.method + ' 方式上传脚本文件: ' + params.dest,
            confidence: 100
        }
    }

    return clean
})

plugin.register('include', function (params, context) {
    var items = params.url.split('://')

    // 必须有协议的情况下才能利用漏洞
    // JSTL import 类型的插件回调，已经在Java层面做了过滤
    if (items.length != 2) {
        return clean
    }

    // http 方式 SSRF
    if (items[0].toLowerCase() == 'http') {
        return {
            action:     'block',
            message:    'SSRF漏洞: ' + params.function + ' 方式',
            confidence: 70
        }
    }

    // file 协议
    if (items[0].toLowerCase() == 'file') {
        var basename = items[1].split('/').pop()

        if (items[1].endsWith('/')) {
            return {
                action:     'block',
                message:    '敏感目录访问: ' + params.function + ' 方式',
                confidence: 100
            }
        }

        for (var i = 0; i < forcefulBrowsing.unwantedFilenames.length; i ++) {
            if (basename == forcefulBrowsing.unwantedFilenames[i]) {
                return {
                    action:     'block',
                    message:    '敏感文件下载: ' + params.function + ' 方式',
                    confidence: 100
                }
            }
        }
    }

    return clean
})


plugin.register('writeFile', function (params, context) {
    if (ntfsRegex.test(params.realpath)) {
        return {
            action:     'block',
            message:    '尝试利用NTFS流上传后门: ' + params.realpath,
            confidence: 90
        }
    }

    if (scriptFileRegex.test(params.realpath)) {
        return {
            action:     'log',
            message:    '尝试写入脚本文件: ' + params.realpath,
            confidence: 90
        }
    }
    return clean
})


plugin.register('fileUpload', function (params, context) {
    if (scriptFileRegex.test(params.filename) || ntfsRegex.test(params.filename)) {
        return {
            action:     'block',
            message:    '尝试上传脚本文件: ' + params.filename,
            confidence: 90
        }
    }

    if (params.filename == ".htaccess" || params.filename == ".user.ini") {
        return {
            action:     'block',
            message:    '尝试上传 Apache/PHP 配置文件: ' + params.filename,
            confidence: 90
        } 
    }

    return clean
})


plugin.register('command', function (params, context) {

    // 算法1: 根据堆栈，检查是否为反序列化攻击
    // 
    // 如果你在服务器有执行命令的需求，我们建议你修改 算法2
    var message = undefined
    var known   = {
        'java.lang.reflect.Method.invoke': '尝试通过反射执行命令',
        'ognl.OgnlRuntime.invokeMethod': '尝试通过 OGNL 代码执行命令',
        'com.thoughtworks.xstream.XStream.unmarshal': '尝试通过 xstream 反序列化执行命令',
        'org.apache.commons.collections4.functors.InvokerTransformer.transform': '尝试通过 transformer 反序列化执行命令'
    }
    
    for (var i = 0; i < params.stack.length; i ++) {
        var method = params.stack[i]
        if (known[method]) {
            message = known[method]
            // break
        }
    }

    if (message) {
        return {
            action:     'block',
            message:    message,
            confidence: 100
        }
    }

    // 算法2: 默认禁止命令执行，如有需要可改成 log 或者 ignore
    return {
        action:     'block',
        message:    '尝试执行命令',
        confidence: 90
    }
})

// 注意: PHP 不支持XXE检测
plugin.register('xxe', function (params, context) {
    var items = params.entity.split('://')

    if (items.length >= 2) {
        var protocol = items[0]

        if (protocol === 'gopher' || protocol === 'ftp' || protocol === 'dict' || protocol === 'expect') {
            return {
                action:     'block',
                message:    'SSRF/Blind XXE 攻击 (' + protocol + ' 协议)',
                confidence: 100
            }
        }

        if (protocol === 'file') {
            return {
                action:     'log',
                message:    '尝试读取外部实体 (file 协议)',
                confidence: 90
            }
        }
    }
    return clean
})

// 默认情况下，当OGNL表达式长度超过30才会进入检测点，此长度可配置
plugin.register('ognl', function (params, context) {
    // 常见 struts payload 语句特征
    var ognlPayloads = [
        'ognl.OgnlContext',
        'ognl.TypeConverter',
        'ognl.MemberAccess',
        '_memberAccess',
        'ognl.ClassResolver',
        'java.lang.Runtime',
        'java.lang.Class',
        'java.lang.ClassLoader',
        'java.lang.System',
        'java.lang.ProcessBuilder',
        'java.lang.Object', 
        'java.lang.Shutdown',
        'java.io.File',
        'javax.script.ScriptEngineManager',
        'com.opensymphony.xwork2.ActionContext'
    ]

    var ognlExpression = params.expression
    for (var index in ognlPayloads) {
        if (ognlExpression.indexOf(ognlPayloads[index]) > -1) {
            return {
                action:     'block',
                message:    '尝试ognl远程命令执行',
                confidence: 100
            }
        }

    }
    return clean
})


// [[ 近期调整~ ]]
plugin.register('deserialization', function (params, context) {
    var deserializationInvalidClazz = [
        'org.apache.commons.collections.functors.InvokerTransformer',
        'org.apache.commons.collections.functors.InstantiateTransformer',
        'org.apache.commons.collections4.functors.InvokerTransformer',
        'org.apache.commons.collections4.functors.InstantiateTransformer',
        'org.codehaus.groovy.runtime.ConvertedClosure',
        'org.codehaus.groovy.runtime.MethodClosure',
        'org.springframework.beans.factory.ObjectFactory',
        'xalan.internal.xsltc.trax.TemplatesImpl'
    ]

    var clazz = params.clazz
    for (var index in deserializationInvalidClazz) {
        if (clazz === deserializationInvalidClazz[index]) {
            return {
                action:     'block',
                message:    '尝试反序列化攻击',
                confidence: 100
            }
        }
    }
    return clean
})

plugin.log('官方插件: 初始化成功')
