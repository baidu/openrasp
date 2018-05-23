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
// 
// 如果你发现这个插件可以绕过，请联系我们，或者在 github 提交 ISSUE
// https://rasp.baidu.com/doc/aboutus/support.html
// 

'use strict'
var plugin  = new RASP('offical')

// 检测逻辑总开关
// 
// block  -> 拦截
// log    -> 打印日志，不拦截
// ignore -> 关闭这个算法

var algorithmConfig = {
    // SQL注入算法#1 - 匹配用户输入
    sqli_userinput: {
        action: 'block'
    },
    // SQL注入算法#1 - 是否拦截数据库管理器，默认关闭，有需要可改为 block
    sqli_dbmanager: {
        action: 'ignore'
    },
    // SQL注入算法#2 - 语句规范
    sqli_policy: {
        action:  'block',
        feature: {
            // 是否禁止多语句执行，select ...; update ...;
            'stacked_query':      true,

            // 是否禁止16进制字符串，select 0x41424344
            'no_hex':             true,

            // 禁止版本号注释，select/*!500001,2,*/3
            'version_comment':    true,

            // 函数黑名单，具体列表见下方，select load_file(...)
            'function_blacklist': true,

            // 拦截 union select NULL,NULL 或者 union select 1,2,3,4
            'union_null':         true,

            // 是否禁止常量比较，AND 8333=8555
            // 当代码编写不规范，常量比较算法会造成大量误报，所以默认不再开启此功能
            'constant_compare':   false,
        },
        function_blacklist: {
            // 文件操作
            'load_file':        true,

            // 时间差注入
            'benchmark':        true,
            'sleep':            true,
            'pg_sleep':         true,

            // 探测阶段
            'is_srvrolemember': true,

            // 报错注入
            'updatexml':        true,
            'extractvalue':     true,

            // 盲注函数，如有误报可删掉一些函数
            'hex':              true,
            'char':             true,
            'chr':              true, 
            'mid':              true,
            'ord':              true,
            'ascii':            true,                
            'bin':              true
        }
    },
    // SSRF - 来自用户输入，且为内网地址就拦截
    ssrf_userinput: {
        action: 'block'
    },
    // SSRF - 是否允许访问 aws metadata
    ssrf_aws: {
        action: 'block'
    },
    // SSRF - 是否允许访问 dnslog 地址
    ssrf_common: {
        action: 'block'
    },
    // SSRF - 是否允许访问混淆后的IP地址
    ssrf_obfuscate: {
        action: 'block'
    },

    // 任意文件下载防护 - 来自用户输入
    readFile_userinput: {
        action: 'block'
    },
    // 任意文件下载防护 - 使用 ../../ 跳出 web 目录读取敏感文件
    readFile_traversal: {
        action: 'block'
    }, 
    // 任意文件下载防护 - 读取敏感文件，最后一道防线
    readFile_unwanted: {
        action: 'block'
    },

    // 写文件操作 - NTFS 流
    writeFile_NTFS: {
        action: 'block'
    },
    // 写文件操作 - PUT 上传脚本文件
    writeFile_PUT_script: {
        action: 'block'
    },    
    // 写文件操作 - 脚本文件
    // https://rasp.baidu.com/doc/dev/official.html#case-3
    writeFile_script: {
        action: 'log'
    },

    // 文件管理器 - 反射方式列目录
    directory_reflect: {
        action: 'block'
    },
    // 文件管理器 - 查看敏感目录
    directory_unwanted: {
        action: 'block'
    },
    // 文件管理器 - 列出webroot之外的目录
    directory_outsideWebroot: {
        action: 'block'
    },

    // 文件包含 - 包含 http:// 内容
    include_http: {
        action: 'block'
    },
    // 文件包含 - 包含目录
    include_dir: {
        action: 'block'
    },
    // 文件包含 - 包含敏感文件
    include_unwanted: {
        action: 'block'
    },  
    // 文件包含 - 包含web目录之外的文件
    include_outsideWebroot: {
        action: 'block'
    },

    // XXE - 使用 gopher/ftp/dict/.. 等不常见协议访问外部实体
    xxe_protocol: {
        action: 'block'
    },

    // 文件上传 - COPY/MOVE 方式，仅适合 tomcat
    fileUpload_webdav: {
        action: 'block'
    },
    // 文件上传 - Multipart 表单方式
    fileUpload_multipart: {
        action: 'block'
    },

    // OGNL 代码执行漏洞
    ognl_exec: {
        action: 'block'
    },

    // 命令执行 - 反射，或者 eval 方式
    command_reflect: {
        action: 'block'
    },
    // 命令执行 - 常规方式，如有需求请改为 'ignore'
    command_other: {
        action: 'block'
    },

    // transformer 反序列化攻击
    transformer_deser: {
        action: 'block'
    }
}

// OpenRASP 大部分算法都不依赖规则，我们主要使用调用堆栈、编码规范、用户输入匹配的思路来检测漏洞。
// 
// 目前，只有文件访问 - 算法#4 加了一个探针，作为最后一道防线
// 当应用读取了这些文件，通常意味着服务器已经被入侵
// 这些配置是通用的，一般不需要定制

const clean = {
    action:     'ignore',
    message:    '无风险',
    confidence: 0
}

var forcefulBrowsing = {
    dotFiles: /\.(7z|tar|gz|bz2|xz|rar|zip|sql|db|sqlite)$/,
    nonUserDirectory: /^\/(proc|sys|root)/,

    // webdav 文件探针 - 最常被下载的文件
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

    // 目录探针 - webshell 查看频次最高的目录
    unwantedDirectory: [
        '/',
        '/home',
        '/var/log',
        '/private/var/log',
        '/proc',
        '/sys',
        'C:\\',
        'D:\\',
        'E:\\'
    ],

    // 文件探针 - webshell 查看频次最高的文件
    absolutePaths: [
        '/etc/shadow',
        '/etc/passwd',
        '/etc/hosts',
        '/etc/apache2/apache2.conf',
        '/root/.bash_history',
        '/root/.bash_profile',
        'c:\\windows\\system32\\inetsrv\\metabase.xml',
        'c:\\windows\\system32\\drivers\\etc\\hosts'
    ]
}

// 如果你配置了非常规的扩展名映射，比如让 .abc 当做PHP脚本执行，那你可能需要增加更多扩展名
var scriptFileRegex = /\.(aspx?|jspx?|php[345]?|phtml)\.?$/i

// 其他的 stream 都没啥用
var ntfsRegex       = /::\$(DATA|INDEX)$/i

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

function validate_stack_php(stacks) {
    var verdict = false

    for (var i = 0; i < stacks.length; i ++) {
        var stack = stacks[i]

        // 来自 eval/assert/create_function/...
        if (stack.indexOf('eval()\'d code') != -1 
            || stack.indexOf('runtime-created function') != -1
            || stack.indexOf('assert code@') != -1
            || stack.indexOf('@call_user_func') != -1
            || stack.indexOf('regexp code@') != -1) {
            verdict = true
            break
        }
    }

    return verdict
}

function is_outside_webroot(appBasePath, realpath, path) {
    var verdict = false

    if (realpath.indexOf(appBasePath) == -1 && (path.indexOf('/../') !== -1 || path.indexOf('\\..\\') !== -1)) {
        return {
            action:     'block',
            message:    '目录遍历攻击，跳出web目录范围 (' + appBasePath + ')',
            confidence: 90
        }
    }

    return verdict
}

function is_from_userinput(parameter, target) {
    var verdict = false

    Object.keys(parameter).some(function (key) {
        var value = parameter[key]

        // 只处理非数组、hash情况
        if (value[0] == target) {
            verdict = true
            return true
        }
    })

    return verdict
}

// 开始

if (RASP.get_jsengine() !== 'v8') {
    // 在java语言下面，为了提高性能，SQLi/SSRF检测逻辑改为java实现
    // 所以，我们需要把一部分配置传递给java
    RASP.config_set('algorithm.config', JSON.stringify(algorithmConfig))
} else {
    // 对于PHP + V8，性能还不错，我们保留JS检测逻辑

    plugin.register('sql', function (params, context) {
        var reason     = false
        var parameters = context.parameter || {}
        var tokens     = RASP.sql_tokenize(params.query, params.server)

        // console.log(tokens)

        // 算法1: 匹配用户输入
        // 1. 简单识别逻辑是否发生改变
        // 2. 识别数据库管理器   
        if (algorithmConfig.sqli_userinput.action != 'ignore') {
            Object.keys(parameters).some(function (name) {
                // 覆盖两种情况，后者仅PHP支持
                // 
                // ?id=XXXX
                // ?filter[category_id]=XXXX
                var value_list

                if (typeof parameters[name][0] == 'string') {
                    value_list = parameters[name]
                } else {
                    value_list = Object.values(parameters[name][0])
                }

                for (var i = 0; i < value_list.length; i ++) {
                    var value = value_list[i]

                    // 请求参数长度超过15才考虑，任何跨表查询都至少需要20个字符，其实可以写的更大点
                    // SELECT * FROM admin
                    // and updatexml(....)
                    if (value.length <= 15) {
                        continue
                    }
                   
                    if (value.length == params.query.length && value == params.query) {
                        // 是否拦截数据库管理器，有需要请改为 1
                        if (algorithmConfig.sqli_dbmanager.action != 'ignore') {
                            reason = '算法2: WebShell - 拦截数据库管理器 - 攻击参数: ' + name
                            return true
                        } else {
                            continue
                        }
                    }

                    // 简单识别用户输入
                    if (params.query.indexOf(value) == -1) {
                        continue
                    }

                    // 去掉用户输入再次匹配
                    var tokens2 = RASP.sql_tokenize(params.query.replaceAll(value, ''), params.server)
                    if (tokens.length - tokens2.length > 2) {
                        reason = '算法1: 数据库查询逻辑发生改变 - 攻击参数: ' + name
                        return true
                    }
                }
            })
            if (reason !== false) {
                return {
                    'action':     algorithmConfig.sqli_userinput.action,
                    'confidence': 90,
                    'message':    reason
                }
            }
        }

        // 算法2: SQL语句策略检查（模拟SQL防火墙功能）
        if (algorithmConfig.sqli_policy.action != 'ignore') {
            var features  = algorithmConfig.sqli_policy.feature
            var func_list = algorithmConfig.sqli_policy.function_blacklist

            var tokens_lc = tokens.map(v => v.toLowerCase())

            for (var i = 1; i < tokens_lc.length; i ++) 
            {
                if (features['union_null'] && tokens_lc[i] === 'select') 
                {
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

                if (features['stacked_query'] && tokens_lc[i] == ';' && i != tokens_lc.length - 1) 
                {
                    reason = '禁止多语句查询'
                    break
                } 
                else if (features['no_hex'] && tokens_lc[i][0] === '0' && tokens_lc[i][1] === 'x') 
                {
                    reason = '禁止16进制字符串'
                    break
                } 
                else if (features['version_comment'] && tokens_lc[i][0] === '/' && tokens_lc[i][1] === '*' && tokens_lc[i][2] === '!') 
                {
                    reason = '禁止MySQL版本号注释'
                    break
                } 
                else if (features['constant_compare'] &&
                    i > 0 && i < tokens_lc.length - 1 && 
                    (tokens_lc[i] === 'xor'
                        || tokens_lc[i][0] === '<'
                        || tokens_lc[i][0] === '>' 
                        || tokens_lc[i][0] === '=')) 
                {
                    // @FIXME: 可绕过，暂时不更新
                    // 简单识别 NUMBER (>|<|>=|<=|xor) NUMBER
                    //          i-1         i          i+2    
                        
                    var op1  = tokens_lc[i - 1]
                    var op2  = tokens_lc[i + 1]

                    // @TODO: strip quotes
                    var num1 = parseInt(op1)
                    var num2 = parseInt(op2)

                    if (! isNaN(num1) && ! isNaN(num2)) {
                        // 允许 1=1, 2=0, 201801010=0 这样的常量对比以避免误报，只要有一个小于10就先忽略掉
                        // 
                        // SQLmap 是随机4位数字，不受影响
                        if (tokens_lc[i][0] === '=' && (num1 < 10 || num2 < 10))
                        {
                            continue;
                        }

                        reason = '禁止常量比较操作: ' + num1 + ' vs ' + num2
                        break
                    }                    
                } 
                else if (features['function_blacklist'] && i > 0 && tokens_lc[i][0] === '(') 
                {
                    // @FIXME: 可绕过，暂时不更新
                    if (func_list[tokens_lc[i - 1]]) {
                        reason = '禁止执行敏感函数: ' + tokens_lc[i - 1]
                        break
                    }
                }
            }

            if (reason !== false) {
                return {
                    action:     algorithmConfig.sqli_policy.action,
                    message:    '算法3: 数据库语句异常: ' + reason,
                    confidence: 100
                }
            }
        }

        return clean
    })

    plugin.register('ssrf', function (params, context) {
        var hostname = params.hostname
        var url      = params.url
        var ip       = params.ip

        var reason   = false
        var action   = 'ignore'

        // 算法1 - ssrf_userinput
        // 当参数来自用户输入，且为内网IP，判定为SSRF攻击
        if (algorithmConfig.ssrf_userinput.action != 'ignore') 
        {
            if (ip.length && 
                is_from_userinput(context.parameter, url) &&
                /^(192|172|10)\./.test(ip[0]))
            {
                reason = '访问内网地址: ' + ip[0]
                action = algorithmConfig.ssrf_userinput.action
            }
        }

        // 算法2 - ssrf_common
        // 检查常见探测域名
        else if (algorithmConfig.ssrf_common.action != 'ignore')
        {
            if (hostname == 'requestb.in' 
            || hostname.endsWith('.vcap.me') 
            || hostname.endsWith('.xip.name') || hostname.endsWith('.xip.io') || hostname.endsWith('.nip.io') 
            || hostname.endsWith('.burpcollaborator.net'))
            {
                reason = '访问已知的内网探测域名'
                action = algorithmConfig.ssrf_common.action
            }
        } 

        // 算法3 - ssrf_aws
        // 检测AWS私有地址，如有需求可注释掉
        else if (algorithmConfig.ssrf_aws.action != 'ignore') 
        {
            if (hostname == '169.254.169.254') 
            {
                reason = '尝试读取 AWS metadata'
                action = algorithmConfig.ssrf_aws.action
            }
        }

        // 算法4 - ssrf_obfuscate
        // 
        // 检查混淆: 
        // http://2130706433
        // http://0x7f001
        // 
        // 以下混淆方式没有检测，容易误报
        // http://0x7f.0x0.0x0.0x1
        // http://0x7f.0.0.0    
        else if (algorithmConfig.ssrf_obfuscate.action != 'ignore') 
        {
            if (Number.isInteger(hostname))
            {
                reason = '尝试使用纯数字IP'
                action = algorithmConfig.ssrf_obfuscate.action
            }
            else if (hostname.startsWith('0x') && hostname.indexOf('.') === -1) 
            {
                reason = '尝试使用16进制IP'
                action = algorithmConfig.ssrf_obfuscate.action
            }
        }

        if (reason) 
        {
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
    var server      = context.server

    // 算法1 - 读取敏感目录
    if (algorithmConfig.directory_unwanted.action != 'ignore') 
    {
        for (var i = 0; i < forcefulBrowsing.unwantedDirectory.length; i ++) {
            if (realpath == forcefulBrowsing.unwantedDirectory[i]) {
                return {
                    action:     algorithmConfig.directory_unwanted.action,
                    message:    'WebShell文件管理器 - 读取敏感目录',
                    confidence: 100
                }
            }
        }
    }

    // 算法2 - 使用至少2个/../，且跳出web目录
    if (algorithmConfig.directory_outsideWebroot.action != 'ignore') 
    {
        if (canonicalPath(path).indexOf('/../../') != -1 && realpath.indexOf(appBasePath) == -1) 
        {
            return {
                action:     algorithmConfig.directory_outsideWebroot.action,
                message:    '尝试列出Web目录以外的目录',
                confidence: 90
            }
        }
    }

    if (algorithmConfig.directory_reflect.action != 'ignore') 
    {

        // 目前，只有 PHP 支持通过堆栈方式，拦截列目录功能
        if (server.language == 'php' && validate_stack_php(params.stack)) 
        {
            return {
                action:     algorithmConfig.directory_reflect.action,
                message:    '发现 Webshell，或者其他eval类型的后门',
                confidence: 90
            }            
        }
    }

    return clean
})


plugin.register('readFile', function (params, context) {
    var server = context.server

    // 算法1: 和URL比较，检查是否为成功的目录扫描。仅适用于 java webdav 方式
    // 
    // 注意: 此方法受到 readfile.extension.regex 和资源文件大小的限制
    // https://rasp.baidu.com/doc/setup/others.html#java-common
    if (1 && server.language == 'java') {
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
    }

    // 算法2: 文件、目录探针
    // 如果应用读取了列表里的文件，比如 /root/.bash_history，这通常意味着后门操作
    if (algorithmConfig.readFile_unwanted.action != 'ignore')
    {
        var realpath_lc = params.realpath.toLowerCase()

        for (var j = 0; j < forcefulBrowsing.absolutePaths.length; j ++) {
            if (forcefulBrowsing.absolutePaths[j] == realpath_lc) {
                return {
                    action:     algorithmConfig.readFile_unwanted.action,
                    message:    'WebShell/文件管理器 - 尝试读取系统文件: ' + params.realpath,
                    confidence: 90
                }
            }
        }
    }

    // 算法3: 检查文件遍历，看是否超出web目录范围
    // e.g 使用 ../../../etc/passwd 跨目录读取文件
    if (algorithmConfig.readFile_traversal.action != 'ignore') 
    {
        var path        = params.path
        var appBasePath = context.appBasePath

        if (is_outside_webroot(appBasePath, params.realpath, path)) {
            return {
                action:     algorithmConfig.readFile_traversal.action,
                message:    '目录遍历攻击，跳出web目录范围 (' + appBasePath + ')',
                confidence: 90
            }
        }
    }

    // 算法4: 拦截任意文件下载漏洞，要读取的文件来自用户输入
    // ?file=/etc/./hosts
    if (algorithmConfig.readFile_userinput.action != 'ignore') 
    {
        if (is_from_userinput(context.parameter, params.path)) {
            return {
                action:     algorithmConfig.readFile_userinput.action,
                message:    '任意文件下载攻击，目标文件: ' + params.realpath,
                confidence: 90
            }        
        }
    }

    return clean
})

plugin.register('include', function (params, context) {
    var url = params.url    

    // 如果没有协议
    // ?file=../../../../../var/log/httpd/error.log
    if (url.indexOf('://') == -1) {
        var path        = canonicalPath(url)
        var realpath    = params.realpath
        var appBasePath = context.appBasePath

        // 是否跳出 web 目录？
        if (algorithmConfig.include_outsideWebroot.action != 'ignore' &&
            is_outside_webroot(appBasePath, realpath, path)) 
        {
            return {
                action:     algorithmConfig.include_outsideWebroot.action,
                message:    '任意文件包含攻击，包含web目录范围之外的文件 (' + appBasePath + ')',
                confidence: 100
            }
        }

        return clean
    }

    // 如果有协议
    // include ('http://xxxxx')
    var items = url.split('://')

    // http 方式 SSRF/RFI
    if (items[0].toLowerCase() == 'http') 
    {
        if (algorithmConfig.include_http.action != 'ignore')
        {
            return {
                action:     algorithmConfig.include_http.action,
                message:    'SSRF漏洞: ' + params.function + ' 方式',
                confidence: 70
            }  
        }        
    }

    // file 协议
    if (items[0].toLowerCase() == 'file') {
        var basename = items[1].split('/').pop()

        // 是否为目录？
        if (items[1].endsWith('/')) {
            // 部分应用，如果直接包含目录，会把这个目录内容列出来
            if (algorithmConfig.include_dir.action != 'ignore') {
                return {
                    action:     algorithmConfig.include_dir.action,
                    message:    '敏感目录访问: ' + params.function + ' 方式',
                    confidence: 100
                }
            }
        }

        // 是否为敏感文件？
        if (algorithmConfig.include_unwanted.action != 'ignore') {
            for (var i = 0; i < forcefulBrowsing.unwantedFilenames.length; i ++) {
                if (basename == forcefulBrowsing.unwantedFilenames[i]) {
                    return {
                        action:     algorithmConfig.include_unwanted.action,
                        message:    '敏感文件下载: ' + params.function + ' 方式',
                        confidence: 100
                    }
                }
            }
        }
    }

    return clean
})


plugin.register('writeFile', function (params, context) {

    // 写 NTFS 流文件，肯定不正常
    if (algorithmConfig.writeFile_NTFS.action != 'ignore') 
    {
        if (ntfsRegex.test(params.realpath)) {
            return {
                action:     algorithmConfig.writeFile_NTFS.action,
                message:    '尝试利用NTFS流上传后门: ' + params.realpath,
                confidence: 90
            }
        }
    }

    // PUT 上传
    if (context.method == 'put' &&
        algorithmConfig.writeFile_PUT_script.action != 'ignore') 
    {
        if (scriptFileRegex.test(params.realpath)) {
            return {
                action:     algorithmConfig.writeFile_PUT_script.action,
                message:    '使用 PUT 方式上传脚本文件，路径: ' + params.realpath,
                confidence: 90
            }
        }        
    }

    // 关于这个算法，请参考这个插件定制文档
    // https://rasp.baidu.com/doc/dev/official.html#case-3    
    if (algorithmConfig.writeFile_script.action != 'ignore') 
    {
        if (scriptFileRegex.test(params.realpath)) {
            return {
                action:     algorithmConfig.writeFile_script.action,
                message:    '尝试写入脚本文件，路径: ' + params.realpath,
                confidence: 90
            }
        }
    }
    return clean
})


if (algorithmConfig.fileUpload_multipart.action != 'ignore') 
{
    plugin.register('fileUpload', function (params, context) {

        if (scriptFileRegex.test(params.filename) || ntfsRegex.test(params.filename)) {
            return {
                action:     algorithmConfig.fileUpload_multipart.action,
                message:    '尝试上传脚本文件: ' + params.filename,
                confidence: 90
            }
        }

        if (params.filename == ".htaccess" || params.filename == ".user.ini") {
            return {
                action:     algorithmConfig.fileUpload_multipart.action,
                message:    '尝试上传 Apache/PHP 配置文件: ' + params.filename,
                confidence: 90
            } 
        }

        return clean
    })
}


if (algorithmConfig.fileUpload_webdav.action != 'ignore')
{
    plugin.register('webdav', function (params, context) {
        
        // 源文件不是脚本 && 目标文件是脚本，判定为MOVE方式写后门
        if (! scriptFileRegex.test(params.source) && scriptFileRegex.test(params.dest)) 
        {
            return {
                action:    algorithmConfig.fileUpload_webdav.action,
                message:   '尝试通过 ' + context.method + ' 方式上传脚本文件: ' + params.dest,
                confidence: 100
            }
        }

        return clean
    })
}


plugin.register('command', function (params, context) {
    var server  = context.server
    var message = undefined

    // 算法1: 根据堆栈，检查是否为反序列化攻击。
    // 理论上，此算法不存在误报

    if (algorithmConfig.command_reflect.action != 'ignore') {
        // Java 检测逻辑
        if (server.language == 'java') {
            var userCode = false
            var known    = {
                'java.lang.reflect.Method.invoke':                                              '尝试通过反射执行命令',
                'ognl.OgnlRuntime.invokeMethod':                                                '尝试通过 OGNL 代码执行命令',
                'com.thoughtworks.xstream.XStream.unmarshal':                                   '尝试通过 xstream 反序列化执行命令',
                'org.apache.commons.collections4.functors.InvokerTransformer.transform':        '尝试通过 transformer 反序列化执行命令',
                'org.jolokia.jsr160.Jsr160RequestDispatcher.dispatchRequest':                   '尝试通过 JNDI 注入方式执行命令',
                'com.alibaba.fastjson.parser.deserializer.JavaBeanDeserializer.deserialze':     '尝试通过 fastjson 反序列化方式执行命令',
                'org.springframework.expression.spel.support.ReflectiveMethodExecutor.execute': '尝试通过 Spring SpEL 表达式执行命令'
            }
            
            for (var i = 2; i < params.stack.length; i ++) {
                var method = params.stack[i]

                if (method.startsWith('ysoserial.Pwner')) {
                    message = 'YsoSerial 漏洞利用工具 - 反序列化攻击'
                    break
                }

                if (method == 'org.codehaus.groovy.runtime.ProcessGroovyMethods.execute') {
                    message = '尝试通过 Groovy 脚本执行命令'
                    break
                }

                // 仅当命令本身来自反射调用才拦截
                // 如果某个类是反射调用，这个类再主动执行命令，则忽略
                if (! method.startsWith('java.') && ! method.startsWith('sun.') && !method.startsWith('com.sun.')) {
                    userCode = true
                }

                if (known[method]) {
                    // 同上，如果反射调用和命令执行之间，包含用户代码，则不认为是反射调用
                    if (userCode && method == 'java.lang.reflect.Method.invoke') {
                        continue
                    }

                    message = known[method]
                    // break
                }
            }
        }

        // PHP 检测逻辑
        else if (server.language == 'php' && validate_stack_php(params.stack)) 
        {
            message = '发现 Webshell，或者基于 eval/assert/create_function/preg_replace/.. 等类型的代码执行漏洞'
        }

        if (message) 
        {
            return {
                action:     algorithmConfig.command_reflect.action,
                message:    message,
                confidence: 100
            }
        }
    }

    // 算法2: 默认禁止命令执行
    // 如有需要可改成 log 或者 ignore
    // 或者根据URL来决定是否允许执行命令

    // 从 v0.31 开始，当命令执行来自非HTTP请求的，我们也会检测反序列化攻击
    // 但是不应该拦截正常的命令执行，所以这里加一个 context.url 检查
    if (! context.url) {
        return clean
    }

    if (algorithmConfig.command_other.action == 'ignore') {
        return clean
    } else {
        return {
            action:     algorithmConfig.command_other.action,
            message:    '尝试执行命令',
            confidence: 90
        } 
    }

})


// 注意: PHP 不支持XXE检测
plugin.register('xxe', function (params, context) {
    var items = params.entity.split('://')

    if (items.length >= 2) {
        var protocol = items[0]
        var address  = items[1]

        if (algorithmConfig.xxe_protocol.action != 'ignore') {
            if (protocol === 'gopher' || protocol === 'ftp' || protocol === 'dict' || protocol === 'expect') {
                return {
                    action:     algorithmConfig.xxe_protocol.action,
                    message:    'SSRF/Blind XXE 攻击 (' + protocol + ' 协议)',
                    confidence: 100
                }
            }
        }

        // file 协议 + 绝对路径, e.g
        // file:///etc/passwd
        //
        // 相对路径容易误报, e.g
        // file://xwork.dtd
        if (address.length > 0 && protocol === 'file' && address[0] == '/') {
            return {
                action:     'log',
                message:    '尝试读取外部实体 (file 协议)',
                confidence: 90
            }
        }
    }
    return clean
})

if (algorithmConfig.ognl_exec.action != 'ignore') 
{
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
        for (var index in ognlPayloads) 
        {
            if (ognlExpression.indexOf(ognlPayloads[index]) > -1) 
            {
                return {
                    action:     algorithmConfig.ognl_exec.action,
                    message:    '尝试ognl远程命令执行',
                    confidence: 100
                }
            }

        }
        return clean
    })
}


// [[ 近期调整~ ]]
if (algorithmConfig.transformer_deser.action != 'ignore') {
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
                    action:     algorithmConfig.transformer_deser.action,
                    message:    '尝试反序列化攻击',
                    confidence: 100
                }
            }
        }
        return clean
    })
}

plugin.log('官方插件: 初始化成功')
