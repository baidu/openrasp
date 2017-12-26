//
// OpenRASP 官方插件已经覆盖了一定的攻击场景，具体检测能力请根据业务来定制
// 如果想了解具体能检测哪些攻击，覆盖哪些已知漏洞，请参考下面两个链接
//
// Web 攻击检测能力说明
// https://rasp.baidu.com/doc/usage/web.html
//
// CVE 漏洞覆盖说明
// https://rasp.baidu.com/doc/usage/cve.html
// 

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
    return ip.split('.').reduce(function(ipInt, octet) { return (ipInt<<8) + parseInt(octet, 10)}, 0) >>> 0;
}

// 在这里配置SQLi/SSRF检测逻辑是否开启
// 为了提高性能，这些检测逻辑均在在 java/php 层面实现
var algorithmConfig = {
    'sqli_userinput': {
        action: 'block'
    },
    'sqli_policy': {
        action:             'block',
        feature:            ['stacked_query', 'no_hex', 'constant_compare', 'version_comment', 'function_blacklist'],
        function_blacklist: ['load_file', 'benchmark', 'pg_sleep', 'sleep']
    },
    'ssrf_aws': {
        action: 'block'
    },
    'ssrf_common': {
        action: 'block'
    },
    'ssrf_obfuscate': {
        action: 'block'
    },
    'ssrf_intranet': {
        action: 'ignore'
    }
}
RASP.config_set('algorithm.config', JSON.stringify(algorithmConfig))

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
                action:     'block',
                message:    '尝试下载敏感文件(' + context.method.toUpperCase() + ' 方式): ' + params.realpath,

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
    // 默认禁止命令执行
    return {
        action:    'block',
        message:   '尝试执行命令',
        confidence: 90
    }
})

plugin.register('xxe', function (params, context) {
    var items = params.entity.split('://')

    if (items.length >= 2) {
        var protocol = items[0]

        if (protocol === 'gopher' || protocol === 'dict' || protocol === 'expect') {
            return {
                action:     'block',
                message:    'SSRF 攻击 (' + protocol + ' 协议)',
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
        action:     'block',
        message:    title + ': ' + params.clazz + '.' + params.method,
        confidence: 100
    }
})

plugin.log('初始化成功')
