//
// 官方插件仅作为 DEMO
// 具体检测能力请根据业务来定制~
//
// 10月底我们将更新一版官方插件，公开一些零规则检测攻击的插件，以及一些应用的专用插件
// 

'use strict'
var plugin = new RASP('offical')

var clean = {
    action: 'ignore',
    message: '无风险',
    confidence: 0
}

var forcefulBrowsing = {
    dotFiles: /\.(gz|7z|xz|tar|rar|zip|sql|db)$/,    
    systemFiles: /^\/(etc|proc|sys|var\/log)(\/|$)/,
    unwanted: [
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
    absolutePaths: [
        '/etc/shadow',
        '/etc/hosts',
        '/etc/apache2/apache2.conf',
        '/root/.bash_history',
        '/root/.bash_profile',
    ]
}

var scannerUA = [
    "attack", "scan", "vulnerability", "injection", "xss", "exploit", "grabber", 
    "cgichk", "bsqlbf", "sqlmap", "nessus", "arachni", "metis", "sql power injector", 
    "bilbo", "absinthe", "black widow", "n-stealth", "brutus", "webtrends security analyzer",
    "netsparker", "jaascois", "pmafind", ".nasl", "nsauditor", "paros", "dirbuster", 
    "pangolin", "nmap nse", "sqlninja", "nikto", "webinspect", "blackwidow", 
    "grendel-scan", "havij", "w3af", "hydra"
]

var xssRegex        = /<script|script>|<iframe|iframe>|javascript:(?!(?:history\.(?:go|back)|void\(0\)))/i
var scriptFileRegex = /\.(jspx?|php[345]?|phtml)\.?$/i
var ntfsRegex       = /::\$(DATA|INDEX)$/i

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

function canonicalPath (path) {
    return path.replace(/\/\.\//g, '/').replace(/\/{2,}/g, '/')
}

plugin.register('directory', function (params, context) {
    var path = canonicalPath(params.path)

    // 简单判断目录遍历，可结合业务定制: e.g 不能超出应用根目录
    if (path.indexOf('/../../../') !== -1 || path.indexOf('\\..\\..\\..\\') !== -1) {
        return {
            action: 'block',
            message: '目录遍历攻击',
            confidence: 90
        }
    }

    if (forcefulBrowsing.systemFiles.test(path)) {
        return {
            action: 'block',
            message: '读取系统目录',
            confidence: 100
        }
    }
    return clean
})

plugin.register('readFile', function (params, context) {
    // 检查是否为成功的目录扫描
    var filename_1 = context.url.replace(/.*\//, '')
    var filename_2 = params.realpath.replace(/.*\//, '')

    plugin.log('readFile:', filename_1, filename_2)

    if (filename_1 == filename_2) {
        var matched = false

        // 尝试下载压缩包、SQL文件等等
        if (forcefulBrowsing.dotFiles.test(filename_1)) {
            matched = true
        } else {
            // 尝试访问敏感文件
            for (var i = 0; i < forcefulBrowsing.unwanted; i ++) {
                if (forcefulBrowsing.unwanted[i] == filename_1) {
                    matched = true
                }
            }
        }

        if (matched) {
            return {
                action: 'block',
                message: '尝试下载敏感文件(' + context.method.toUpperCase() + ' 方式): ' + params.realpath,

                // 如果是HEAD方式下载敏感文件，100% 扫描器攻击
                confidence: context.method == 'head' ? 100 : 90
            }
        }
    }

    // 如果使用绝对路径访问敏感文件
    // 判定为 webshell
    if (params.realpath == params.path) {
        for (var j = 0; j < forcefulBrowsing.absolutePaths; j ++) {
            if (forcefulBrowsing.absolutePaths[i] == params.realpath) {
                return {
                    action:  'block',
                    message: '疑似webshell - 尝试读取系统文件: ' + params.realpath
                }
            }
        }
    }

    // 检查目录遍历（弱）
    var path = canonicalPath(params.path)
    if (path.indexOf('/../../../') !== -1 || path.indexOf('\\..\\..\\..\\') !== -1) {
        return {
            action: 'block',
            message: '目录遍历攻击',
            confidence: 90
        }
    }

    if (forcefulBrowsing.systemFiles.test(params.realpath)) {
        return {
            action: 'block',
            message: '读取系统文件',
            confidence: 100
        }
    }
    return clean
})

plugin.register('writeFile', function (params, context) {
    if (scriptFileRegex.test(params.filename) || ntfsRegex.test(params.filename)) {
        return {
            action: 'block',
            message: '尝试上传脚本文件: ' + params.filename,
            confidence: 90
        }
    }
    return clean
})

plugin.register('fileUpload', function (params, context) {
    if (scriptFileRegex.test(params.filename) || ntfsRegex.test(params.filename)) {
        return {
            action: 'block',
            message: '尝试上传脚本文件: ' + params.filename,
            confidence: 90
        }
    }
    return clean
})

// [[ 近期调整~ ]]
plugin.register('sql', function (params, context) {
    // SQLi 检测 demo
    var sqlRegex = /\bupdatexml\s*\(|\bextractvalue\s*\(|\bunion.*select.*(from|into|benchmark).*\b/i

    if (sqlRegex.test(params.query)) {
        return {
            action: 'block',
            message: 'SQL 注入攻击',
            confidence: 100
        }
    }
    return clean
})

plugin.register('command', function (params, context) {
    return {
        action: 'block',
        message: '尝试执行命令',
        confidence: 100
    }
})

plugin.register('xxe', function (params, context) {
    var items = params.entity.split('://')

    if (items.length >= 2) {
        var protocol = items[0]

        if (protocol === 'gopher' || protocol === 'dict') {
            return {
                action: 'block',
                message: 'SSRF 攻击 (' + protocol + ' 协议)',
                confidence: 100
            }
        }

        if (protocol === 'file') {
            return {
                action: 'log',
                message: '尝试读取外部实体 (file 协议)',
                confidence: 90
            }
        }
    }
    return clean
})

plugin.register('ognl', function (params, context) {
    var ognlExpression = params.expression
    for (var index in ognlPayloads) {
        if (ognlExpression.indexOf(ognlPayloads[index]) > -1) {
            return {
                action: 'block',
                message: '尝试ognl远程命令执行',
                confidence: 100
            }
        }

    }
    return clean
})

// [[ 近期调整~ ]]
plugin.register('deserialization', function (params, context) {
    var clazz = params.clazz
    for (var index in deserializationInvalidClazz) {
        if (clazz === deserializationInvalidClazz[index]) {
            return {
                action: 'block',
                message: '尝试反序列化攻击',
                confidence: 100
            }
        }
    }
    return clean
})

plugin.register('reflection', function(params, context) {
    var title = '异常的执行流'
    var known = {
        'com.thoughtworks.xstream.XStream.unmarshal': 'xstream 反序列化攻击',
        'org.apache.commons.collections4.functors.InvokerTransformer.transform': 'transformer 反序列化攻击'
    }

    // 是否为已知类型的反序列化攻击？
    params.stack.some(function (method) {
        if (known[method]) {
            title = known[method]
            return true;
        }
    });

    return {
        action:  'block',
        message: title + ':' + params.clazz + '.' + params.method,
        confidence: 100
    }
})

// [[ 近期调整~ ]]
plugin.register('request', function(params, context) {

    // 已知扫描 UA    
    var foundScanner = false

    if (context.header['acunetix-product'] || context.header['x-wipp']) {
        foundScanner = true
    } else {
        var ua = context.header['user-agent']
        if (ua) {
            for (var i = 0; i < scannerUA.length; i ++) {
                if (ua.indexOf(scannerUA[i].toLowerCase()) != -1) {
                    foundScanner = true
                    break
                }
            }
        }
    }

    if (foundScanner) {
        return {
            action:  'block',
            message: '已知的扫描器UA: ' + scannerUA[i],
            confidence: 90
        }
    }

    // xss 检测 DEMO
    var parameters = context.parameter;
    var message    = '';

    Object.keys(parameters).some(function (name) {
        parameters[name].some(function (value) {
            if (xssRegex.test(value)) {
                message = 'XSS 攻击: ' + value;
                return true;
            }
        });

        if (message.length) {
            return true;
        }
    });

    if (! message.length) {
        return clean;
    }

    return {
        action: 'block',
        message: message
    }
})

plugin.log('初始化成功')
