const plugin_version = '2019-0225-1830'
const plugin_name    = 'official'

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

// 常用链接
//
// Web 攻击检测能力说明、零规则检测算法介绍
// https://rasp.baidu.com/doc/usage/web.html
//
// CVE 漏洞覆盖说明
// https://rasp.baidu.com/doc/usage/cve.html

'use strict'
var plugin  = new RASP(plugin_name)

// 检测逻辑开关
//
// block  -> 拦截，并打印报警日志
// log    -> 打印日志，不拦截
// ignore -> 关闭这个算法

// BEGIN ALGORITHM CONFIG //

var algorithmConfig = {
    // 快速设置
    meta: {
        // 若 all_log 开启，表示为观察模式，会将所有的 block 都改为 log
        all_log: true,

        // 若 is_dev 开启，表示为线下环境，将开启更多消耗性能的检测算法
        is_dev:  false
    },

    // SQL注入算法#1 - 匹配用户输入
    // 1. 用户输入长度至少 10
    // 2. 用户输入至少包含一个SQL关键词 - 即 pre_filter，[默认关闭]
    // 3. 用户输入完整的出现在SQL语句中，且会导致SQL语句逻辑发生变化
    sql_userinput: {
        name:       '算法1 - 用户输入匹配算法',
        action:     'block',
        min_length: 10,
        pre_filter: 'select|file|from|;',
        pre_enable: false,
    },

    // SQL注入算法#2 - 语句规范
    sql_policy: {
        name:    '算法2 - 拦截异常SQL语句',
        action:  'block',

        // 粗规则 - 为了减少 tokenize 次数，当SQL语句包含一定特征时才进入
        // 另外，我们只需要处理增删改查的语句，虽然 show 语句也可以报错注入，但是算法2没必要处理
        pre_filter: '^(select|insert|update|delete).*(;|\\/\\*|(?:\\d{1,2}\\s*,\\s*){2}|(?:null\\s*,\\s*){2}|0x[\\da-f]{8}|\\b(information_schema|outfile|dumpfile|load_file|benchmark|pg_sleep|sleep|is_srvrolemember|updatexml|extractvalue|hex|char|chr|mid|ord|ascii|bin))\\b',

        feature: {
            // 是否禁止多语句执行，select ...; update ...;
            stacked_query:      true,

            // 是否禁止16进制字符串，select 0x41424344
            no_hex:             true,

            // 禁止版本号注释，select/*!500001,2,*/3
            version_comment:    true,

            // 函数黑名单，具体列表见下方，select load_file(...)
            function_blacklist: true,

            // 拦截 union select NULL,NULL 或者 union select 1,2,3,4
            union_null:         true,

            // 是否拦截 into outfile 写文件操作
            into_outfile:       true,

            // 是否拦截 information_schema 相关读取操作，默认关闭
            information_schema: false
        },
        function_blacklist: {
            // 文件操作
            load_file:        true,

            // 时间差注入
            benchmark:        true,
            sleep:            true,
            pg_sleep:         true,

            // 探测阶段
            is_srvrolemember: true,

            // 报错注入
            updatexml:        true,
            extractvalue:     true,

            // 盲注函数，如有误报可删掉一些函数
            hex:              true,
            char:             false,
            chr:              true,
            mid:              true,
            ord:              true,
            ascii:            true,
            bin:              true
        }
    },

    sql_exception: {
        name:      '算法3 - 检测SQL语句异常',
        action:    'log',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#sql-exception'
    },

    // SSRF - 来自用户输入，且为内网地址就拦截
    ssrf_userinput: {
        name:   '算法1 - 用户输入匹配算法',
        action: 'block'
    },
    // SSRF - 是否允许访问 aws metadata
    ssrf_aws: {
        name:   '算法2 - 拦截 AWS/Aliyun metadata 访问',
        action: 'block'
    },
    // SSRF - 是否允许访问 dnslog 地址
    ssrf_common: {
        name:    '算法3 - 拦截常见 dnslog 地址',
        action:  'block',
        domains: [
            '.ceye.io',
            '.vcap.me',
            '.xip.name',
            '.xip.io',
            'sslip.io',
            '.nip.io',
            '.burpcollaborator.net',
            '.tu4.org'
        ]
    },
    // SSRF - 是否允许访问混淆后的IP地址
    ssrf_obfuscate: {
        name:   '算法4 - 拦截混淆地址',
        action: 'block'
    },
    // SSRF - 禁止使用 curl 读取 file:///etc/passwd、php://filter/XXXX 这样的内容
    ssrf_protocol: {
        name:      '算法5 - 拦截 php:// 等异常协议',
        action:    'block',
        protocols: [
            'file',
            'gopher',

            // java specific
            'jar',
            'netdoc',

            // php specific
            'dict',
            'php',
            'phar',
            'compress.zlib',
            'compress.bzip2'
        ]
    },

    // 任意文件下载防护 - 来自用户输入
    readFile_userinput: {
        name:   '算法1 - 用户输入匹配算法',
        action: 'block'
    },
    // 任意文件下载防护 - 使用 file_get_contents 等函数读取 http(s):// 内容（注意，这里不区分是否为内网地址）
    readFile_userinput_http: {
        name:   '算法2 - 用户输入匹配算法 + http 协议',
        action: 'block'
    },
    // 任意文件下载防护 - 使用 file_get_contents 等函数读取 file://、php:// 协议
    readFile_userinput_unwanted: {
        name:   '算法3 - 拦截 php:// 等异常协议',
        action: 'block'
    },
    // 任意文件下载防护 - 使用 ../../ 跳出 web 目录读取敏感文件
    readFile_outsideWebroot: {
        name:      '算法4 - 禁止使用 ../../ 访问web目录以外的文件',
        action:    'ignore',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#case-out-webroot'
    },
    // 任意文件下载防护 - 读取敏感文件，最后一道防线
    readFile_unwanted: {
        name:   '算法5 - 文件探针算法',
        action: 'block'
    },

    // 写文件操作 - NTFS 流
    writeFile_NTFS: {
        name:   '算法1 - 拦截 NTFS ::$DATA 写入操作',
        action: 'block'
    },
    // 写文件操作 - PUT 上传脚本文件
    writeFile_PUT_script: {
        name:   '算法2 - 拦截 PUT 方式上传 php/jsp 等脚本文件',
        action: 'block'
    },
    // 写文件操作 - 脚本文件
    // https://rasp.baidu.com/doc/dev/official.html#case-file-write
    writeFile_script: {
        name:      '算法1 - 拦截所有 php/jsp 等脚本文件的写入操作',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#case-file-write',
        action:    'ignore'
    },

    // 重命名监控 - 将普通文件重命名为webshell，
    // 案例有 MOVE 方式上传后门、CVE-2018-9134 dedecms v5.7 后台重命名 getshell
    rename_webshell: {
        name:   '算法1 - 通过重命名方式获取 WebShell',
        action: 'block'
    },
    // copy_webshell: {
    //     action: 'block'
    // },

    // 文件管理器 - 用户输入匹配，仅当直接读取绝对路径时才检测
    directory_userinput: {
        name:   '算法1 - 用户输入匹配算法',
        action: 'block'
    },
    // 文件管理器 - 反射方式列目录
    directory_reflect: {
        name:   '算法2 - 通过反射调用，查看目录内容',
        action: 'block'
    },
    // 文件管理器 - 查看敏感目录
    directory_unwanted: {
        name:   '算法3 - 尝试查看敏感目录',
        action: 'block'
    },

    // 文件包含 - 用户输入匹配
    include_userinput: {
        name:   '算法1 - 用户输入匹配算法',
        action: 'block'
    },
    // 文件包含 - 特殊协议
    include_protocol: {
        name:   '算法2 - 尝试包含 jar:// 等异常协议',
        action: 'block',
        protocols: [
            'file',
            'gopher',

            // java specific
            'jar',
            'netdoc',

            // php stream
            'http',
            'https',

            // php specific
            'dict',
            'php',
            'phar',
            'compress.zlib',
            'compress.bzip2',
            'zip',
            'rar'
        ]
    },

    // XXE - 使用 gopher/ftp/dict/.. 等不常见协议访问外部实体
    xxe_protocol: {
        name:   '算法1 - 使用 ftp:// 等异常协议加载外部实体',
        action: 'block',
        protocols: [
            'ftp',
            'dict',
            'gopher',
            'jar',
            'netdoc'
        ]
    },
    // XXE - 使用 file 协议读取内容，可能误报，默认 log
    xxe_file: {
        name:      '算法2 - 使用 file:// 协议读取文件',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#case-xxe',
        action:    'log',
    },

    // 文件上传 - COPY/MOVE 方式，仅适合 tomcat
    fileUpload_webdav: {
        name:   '算法1 - MOVE 方式文件上传脚本文件',
        action: 'block'
    },
    // 文件上传 - Multipart 方式上传脚本文件
    fileUpload_multipart_script: {
        name:   '算法2 - Multipart 方式文件上传 PHP/JSP 等脚本文件',
        action: 'block'
    },
    // 文件上传 - Multipart 方式上传 HTML/JS 等文件
    fileUpload_multipart_html: {
        name:   '算法3 - Multipart 方式文件上传 HTML/JS 等文件',
        action: 'ignore'
    },

    // OGNL 代码执行漏洞
    ognl_exec: {
        name:   '算法1 - 执行异常 OGNL 语句',
        action: 'block'
    },

    // 命令执行 - java 反射、反序列化，php eval 等方式
    command_reflect: {
        name:   '算法1 - 通过反射执行命令，比如反序列化、加密后门',
        action: 'block'
    },
    // 命令注入 - 命令执行后门，或者命令注入
    command_userinput: {
        name:       '算法2 - 用户输入匹配算法，包括命令注入检测',
        action:     'block',
        min_length: 2
    },
    // 命令注入 - 常见命令
    command_common: {
        name:    '算法3 - 识别常用渗透命令（探针）',
        action:  'log',
        pattern: 'cat.*/etc/passwd|nc.{1,30}-e.{1,100}/bin/(?:ba)?sh|bash\\s-.{0,4}i.{1,20}/dev/tcp/|subprocess.call\\(.{0,6}/bin/(?:ba)?sh|fsockopen\\(.{1,50}/bin/(?:ba)?sh|perl.{1,80}socket.{1,120}open.{1,80}exec\\(.{1,5}/bin/(?:ba)?sh|([\\|\\&`;\\x0d\\x0a]|$\\([^\\(]).{0,3}(ping|nslookup|curl|wget|mail).{1,10}[a-zA-Z0-9_\\-]{1,15}\\.[a-zA-Z0-9_\\-]{1,15}'
    },
    // 命令执行 - 是否拦截所有命令执行？如果没有执行命令的需求，可以改为 block，最大程度的保证服务器安全
    command_other: {
        name:   '算法4 - 记录或者拦截所有命令执行操作',
        action: 'ignore'
    },

    // transformer 反序列化攻击
    deserialization_transformer: {
        name:   '算法1 - 拦截 transformer 反序列化攻击',
        action: 'block'
    },

    // xss 用户输入匹配算法
    // 1. 当用户输入长度超过15，匹配上标签正则，且出现在响应里，直接拦截
    // 2. 当用户输入长度超过15，匹配上标签正则这样的参数个数超过 10，判定为扫描攻击，直接拦截
    xss_userinput: {
        name:   '算法2 - 拦截输出在响应里的反射 XSS',
        action: 'log',

        filter_regex: "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/ >]",
        min_length: 15,
        max_detection_num: 10
    },

    // php 专有算法
    xss_echo: {
        name:   '算法1 - PHP: 禁止直接输出 GPC 参数',
        action: 'log',

        filter_regex: "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/ >]"
    },    

    webshell_eval: {
        name:   '算法1 - 拦截简单的 PHP 中国菜刀后门',
        action: 'block'
    },

    webshell_command: {
        name:   '算法2 - 拦截简单的 PHP 命令执行后门',
        action: 'block'
    },

    webshell_file_put_contents: {
        name:   '算法3 - 拦截简单的 PHP 文件上传后门',
        action: 'block'
    },

    webshell_callable: {
        name:   '算法4 - 拦截简单的 PHP array_map/walk/filter 后门',
        action: 'block',
        functions: [
            'system', 'exec', 'passthru', 'proc_open', 'shell_exec', 'popen', 'pcntl_exec', 'assert'
        ]
    },

    webshell_ld_preload: {
        name:   '算法5 - 拦截基于 LD_PRELOAD 的后门',
        action: 'block'
    }
}

// END ALGORITHM CONFIG //

// 将所有拦截开关设置为 log; 如果是单元测试模式，忽略此选项
if (algorithmConfig.meta.all_log && ! RASP.is_unittest) {
    Object.keys(algorithmConfig).forEach(function (name) {
        if (algorithmConfig[name].action == 'block') {
           algorithmConfig[name].action = 'log'
        }
    })
}

// 配置挂载到全局 RASP 变量
RASP.algorithmConfig = algorithmConfig

const clean = {
    action:     'ignore',
    message:    'Looks fine to me',
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

// 正常文件
var cleanFileRegex = /\.(jpg|jpeg|png|gif|bmp|txt|rar|zip)$/i

// 匹配 HTML/JS 等可以用于钓鱼、domain-fronting 的文件
var htmlFileRegex   = /\.(htm|html|js)$/i

// 其他的 stream 都没啥用
var ntfsRegex       = /::\$(DATA|INDEX)$/i

// 已知用户输入匹配算法误报: 传入 1,2,3,4 -> IN(1,2,3,4)
var commaNumRegex   = /^[0-9, ]+$/

// SQL注入算法1 - 预过滤正则
var sqliPrefilter1  = new RegExp(algorithmConfig.sql_userinput.pre_filter)

// SQL注入算法2 - 预过滤正则
var sqliPrefilter2  = new RegExp(algorithmConfig.sql_policy.pre_filter)

// 命令执行探针 - 常用渗透命令
var cmdPostPattern  = new RegExp(algorithmConfig.command_common.pattern)

// 常用函数
String.prototype.replaceAll = function(token, tokenValue) {
    // 空值判断，防止死循环
    if (! token || token.length == 0) {
        return this
    }

    var index  = 0;
    var string = this;

    do {
        string = string.replace(token, tokenValue);
    } while((index = string.indexOf(token, index)) > -1);

    return string
}

// function canonicalPath (path) {
//     return path.replaceAll('/./', '/').replaceAll('//', '/').replaceAll('//', '/')
// }

// 我们不再需要简化路径，当出现两个 /../ 或者两个 \..\ 就可以判定为路径遍历攻击了，e.g
// /./././././home/../../../../etc/passwd
// \\..\\..\\..
// \/..\/..\/..
function has_traversal (path) {

    // 左右斜杠，一视同仁
    var path2 = "/" + path.replaceAll('\\', '/') + "/"
    // 覆盖 ../../
    // 以及 /../../
    var left  = path2.indexOf('/../')
    var right = path2.lastIndexOf('/../')

    if (left != -1 && right != -1 && left != right)
    {
        return true
    }

    return false
}

function is_hostname_dnslog(hostname) {
    var domains = algorithmConfig.ssrf_common.domains

    if (hostname == 'requestb.in' || hostname == 'transfer.sh')
    {
        return true
    }

    for (var i = 0; i < domains.length; i ++)
    {
        if (hostname.endsWith(domains[i]))
        {
            return true
        }
    }

    return false
}

// function basename (path) {
//     // 简单处理，同时支持 windows/linux
//     var path2 = path.replaceAll('\\', '/')
//     var idx   = path2.lastIndexOf('/')
//     return path.substr(idx + 1)
// }

// function has_file_extension(path) {
//     var filename = basename(path)
//     var index    = filename.indexOf('.')

//     if (index > 0 && index != filename.length - 1) {
//         return true
//     }

//     return false
// }

function validate_stack_php(stacks) {
    var verdict = false

    for (var i = 0; i < stacks.length; i ++) {
        var stack = stacks[i]

        // 来自 eval/assert/create_function/...
        if (stack.indexOf('eval()\'d code') != -1
            || stack.indexOf('runtime-created function') != -1
            || stack.indexOf('assert code@') != -1
            || stack.indexOf('regexp code@') != -1) {
            verdict = true
            break
        }

        // call_user_func/call_user_func_array 两个函数调用很频繁
        // 必须是 call_user_func 直接调用 system/exec 等函数才拦截，否则会有很多误报
        if (stack.indexOf('@call_user_func') != -1) {
            if (i <= 1) {
                verdict = true
                break
            }
        }
    }

    return verdict
}

function is_absolute_path(path, is_windows) {

    // Windows - C:\\windows
    if (is_windows) {

        if (path[1] == ':')
        {
            var drive = path[0].toLowerCase()
            if (drive >= 'a' && drive <= 'z')
            {
                return true
            }
        }
    }

    // Unices - /root/
    return path[0] === '/'
}

function is_outside_webroot(appBasePath, realpath, path) {
    var verdict = false

    // 如果指定path 为 null 则不校验目录穿越
    if (path == null || has_traversal(path)) {
        // servlet 3.X 之后可能会获取不到 appBasePath，或者为空
        // 提前加个判断，防止因为bug导致误报
        if (! appBasePath || appBasePath.length == 0) {
            verdict = false
        }
        else if (realpath.indexOf(appBasePath) == -1) {
            verdict = true
        }
    }

    return verdict
}

// 路径是否来自用户输入
// file_get_contents("/etc/passwd");
// file_get_contents("../../../../../../../etc/passwd");
//
// 或者以用户输入结尾
// file_get_contents("/data/uploads/" . "../../../../../../../etc/passwd");
function is_path_endswith_userinput(parameter, target, realpath, is_windows)
{
    var verdict = false

    Object.keys(parameter).some(function (key) {
        // 只处理非数组、hash情况
        var value = parameter[key]
            value = value[0]

        // 只处理字符串类型的
        if (typeof value != 'string') {
            return
        }
        // 如果应用做了特殊处理， 比如传入 file:///etc/passwd，实际看到的是 /etc/passwd
        if (value.startsWith('file://') && 
            is_absolute_path(target, is_windows) && 
            value.endsWith(target)) 
        {
            verdict = true
            return true
        }

        // 去除多余/ 和 \ 的路径
        var simplifiedValue

        // Windows 下面
        // 传入 ../../../conf/tomcat-users.xml
        // 看到 c:\tomcat\webapps\root\..\..\conf\tomcat-users.xml
        if (is_windows) {
            value = value.replaceAll('/', '\\')
            target = target.replaceAll('/', '\\')
            realpath = realpath.replaceAll('/', '\\')
            simplifiedValue = value.replaceAll('\\\\','\\')
        } else{
            simplifiedValue = value.replaceAll('//','/')
        }

        // 参数必须有跳出目录，或者是绝对路径
        if ((target.endsWith(value) || target.endsWith(simplifiedValue))
            && (has_traversal(value) || value == realpath || simplifiedValue == realpath))
        {
            verdict = true
            return true
        }
    })

    return verdict
}

// 检查是否包含用户输入 - 适合目录
function is_path_containing_userinput(parameter, target, is_windows)
{
    var verdict = false

    Object.keys(parameter).some(function (key) {
        var value = parameter[key]
            value = value[0]

        // 只处理字符串类型的
        if (typeof value != 'string') {
            return
        }

        if (is_windows) {
            value = value.replaceAll('/', '\\')
        }

        // java 下面，传入 /usr/ 会变成 /usr，所以少匹配一个字符
        var value_noslash = value.substr(0, value.length - 1)

        // 只处理非数组、hash情况
        if (has_traversal(value) && target.indexOf(value_noslash) != -1) {
            verdict = true
            return true
        }

    })
    return verdict
}

// 是否来自用户输入 - 适合任意类型参数
function is_from_userinput(parameter, target)
{
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

// 检查SQL逻辑是否被用户参数所修改
function is_token_changed(raw_tokens, userinput_idx, userinput_length, distance) 
{
    // 当用户输入穿越了多个token，就可以判定为代码注入，默认为2
    var start = -1, end = raw_tokens.length, distance = distance || 2

    // 寻找 token 起始点，可以改为二分查找
    for (var i = 0; i < raw_tokens.length; i++)
    {
        if (raw_tokens[i].stop >= userinput_idx)
        {
            start = i
            break
        }
    }

    // 寻找 token 结束点
    // 另外，最多需要遍历 distance 个 token
    for (var i = start; i < start + distance && i < raw_tokens.length; i++)
    {
        if (raw_tokens[i].stop >= userinput_idx + userinput_length - 1)
        {
            end = i
            break
        }
    }

    if (end - start > distance) {
        return true
    }
    return false
}

// 下个版本将会支持翻译，目前还需要暴露一个 getText 接口给插件
function _(message, args) 
{
    args = args || []

    for (var i = 0; i < args.length; i ++) 
    {
        var symbol = '%' + (i + 1) + '%'
        message = message.replace(symbol, args[i])
    }

    return message
}

// 开始

if (RASP.get_jsengine() !== 'v8') {
    // 在java语言下面，为了提高性能，SQLi/SSRF检测逻辑改为java实现
    // 所以，我们需要把一部分配置传递给java

    // 1.0.0 RC2 会删除 RASP.config_set，统一在全局的 RASP.algorithmConfig 获取配置
    if (RASP.config_set) {
        RASP.config_set('algorithm.config', JSON.stringify(algorithmConfig))
    }
} else {
    // 对于PHP + V8，性能还不错，我们保留JS检测逻辑
    plugin.register('sql', function (params, context) {

        var reason     = false
        var min_length = algorithmConfig.sql_userinput.min_length
        var parameters = context.parameter || {}
        var json_parameters = context.json || {}
        var raw_tokens = []

        function _run(values, name) {
            var reason = false
            values.some(function (value) {
                // 不处理3维及以上的数组
                if (typeof value != "string") {
                    return false
                }

                // 最短长度限制
                if (value.length < min_length) {
                    return false
                }

                // 检查用户输入是否存在于SQL中
                var userinput_idx = params.query.indexOf(value)
                if (userinput_idx == -1) {
                    return false
                }

                // 过滤已知误报
                // 1,2,3,4,5 -> IN(1,2,3,4,5)
                if (commaNumRegex.test(value)) {
                    return false
                }

                // 预过滤正则，如果开启
                if (algorithmConfig.sql_userinput.pre_enable && ! sqliPrefilter1.test(value)) {
                    return false
                }

                // 懒加载，需要的时候初始化 token
                if (raw_tokens.length == 0) {
                    raw_tokens = RASP.sql_tokenize(params.query, params.server)
                }

                if (is_token_changed(raw_tokens, userinput_idx, value.length)) {
                    reason = _("SQLi - SQL query structure altered by user input, request parameter name: %1%", [name])
                    return true
                }
            })
            return reason
        }

        // 算法1: 匹配用户输入，简单识别逻辑是否发生改变
        if (algorithmConfig.sql_userinput.action != 'ignore') {

            // 匹配 GET/POST/multipart 参数
            Object.keys(parameters).some(function (name) {
                // 覆盖场景，后者仅PHP支持
                // ?id=XXXX
                // ?data[key1][key2]=XXX
                var value_list

                if (typeof parameters[name][0] == 'string') {
                    value_list = parameters[name]
                } else {
                    value_list = Object.values(parameters[name][0])
                }

                reason = _run(value_list, name)
                if (reason) {
                    return true
                }
            })

            if(Object.keys(json_parameters).length > 0) {
                var jsons = [ [json_parameters, "input_json"] ]
                while(jsons.length > 0 && reason === false) {
                    var json_arr = jsons.pop()
                    var crt_json_key = json_arr[1]
                    var json_obj = json_arr[0]
                    for (item in json_obj) {
                        if(typeof json_obj[item] == "string") {
                            reason = _run([json_obj[item]], crt_json_key + "->" + item)
                            if(reason !== false) {
                                break;
                            }
                        }
                        else if(typeof json_obj[item] == "object") {
                            jsons.push([json_obj[item], crt_json_key + "->" + item])
                        }
                    }
                }
            }

            if (reason !== false)
            {
                return {
                    action:     algorithmConfig.sql_userinput.action,
                    confidence: 90,
                    message:    reason,
                    algorithm:  'sql_userinput'
                }
            }
        }

        // 算法2: SQL语句策略检查（模拟SQL防火墙功能）
        if (algorithmConfig.sql_policy.action != 'ignore') {

            // 懒加载，需要时才处理
            if (raw_tokens.length == 0) {
                var query_lc = params.query.toLowerCase().trim()

                if (sqliPrefilter2.test(query_lc)) {
                    raw_tokens = RASP.sql_tokenize(params.query, params.server)
                }
            }

            var features  = algorithmConfig.sql_policy.feature
            var func_list = algorithmConfig.sql_policy.function_blacklist

            // 转换小写，避免大小写绕过
            var tokens_lc = raw_tokens.map(function(v) {
                return v.text.toLowerCase()
            })

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
                        reason = _("SQLi - Detected UNION-NULL phrase in sql query")
                        break
                    }
                    continue
                }

                if (features['stacked_query'] && tokens_lc[i] == ';' && i != tokens_lc.length - 1)
                {
                    reason = _("SQLi - Detected stacked queries")
                    break
                }
                else if (features['no_hex'] && tokens_lc[i][0] === '0' && tokens_lc[i][1] === 'x')
                {
                    reason = _("SQLi - Detected hexadecimal values in sql query")
                    break
                }
                else if (features['version_comment'] && tokens_lc[i][0] === '/' && tokens_lc[i][1] === '*' && tokens_lc[i][2] === '!')
                {
                    reason = _("SQLi - Detected MySQL version comment in sql query")
                    break
                }
                else if (features['function_blacklist'] && i > 0 && tokens_lc[i][0] === '(')
                {
                    var func_name = tokens_lc[i - 1]

                    if (func_list[func_name]) {
                        reason = _("SQLi - Detected dangerous method call %1%() in sql query", [func_name])
                        break
                    }
                }
                else if (features['into_outfile'] && i < tokens_lc.length - 2 && tokens_lc[i] == 'into')
                {
                    if (tokens_lc[i + 1] == 'outfile' || tokens_lc[i + 1] == 'dumpfile')
                    {
                        reason = _("SQLi - Detected INTO OUTFILE phrase in sql query")
                        break
                    }
                }
                else if (features['information_schema'] && i < tokens_lc.length - 1 && tokens_lc[i] == 'from')
                {
                    // `information_schema`.tables
                    // information_schema  .tables
                    var parts = tokens_lc[i + 1].replaceAll('`', '').split('.')
                    if (parts.length == 2)
                    {
                        if (parts[0].trim() == 'information_schema' && parts[1].trim() == 'tables')
                        {
                            reason = _("SQLi - Detected access to MySQL information_schema.tables table")
                            break
                        }
                    }
                }
            }

            if (reason !== false) {
                return {
                    action:     algorithmConfig.sql_policy.action,
                    message:    reason,
                    confidence: 100,
                    algorithm:  'sql_policy'
                }
            }
        }

        // 加入缓存，对 prepared sql 特别有效
        return clean
    })

    plugin.register('ssrf', function (params, context) {
        var hostname = params.hostname
        var url      = params.url
        var ip       = params.ip

        var reason   = false
        var action   = 'ignore'

        // 1.0 RC1 没有过滤 hostname 为空的情况，为了保证兼容性在这里加个判断
        if (hostname.length === 0)
        {
            return clean
        }

        // 算法1 - 当参数来自用户输入，且为内网IP，判定为SSRF攻击
        if (algorithmConfig.ssrf_userinput.action != 'ignore')
        {
            if (is_from_userinput(context.parameter, url))
            {
                if (ip.length && /^(127|192|172|10)\./.test(ip[0]))
                {
                    return {
                        action:     algorithmConfig.ssrf_userinput.action,
                        message:    _("SSRF - Requesting intranet address: %1%", [ ip[0] ]),
                        confidence: 100,
                        algorithm:  'ssrf_userinput'
                    }
                }
                else if (hostname == '[::]') 
                {
                    return {
                        action:     algorithmConfig.ssrf_userinput.action,
                        message:    _("SSRF - Requesting intranet address: %1%", [ hostname ]),
                        confidence: 100,
                        algorithm:  'ssrf_userinput'
                    }
                }
            }
        }

        // 算法2 - 检查常见探测域名
        if (algorithmConfig.ssrf_common.action != 'ignore')
        {
            if (is_hostname_dnslog(hostname))
            {
                return {
                    action:     algorithmConfig.ssrf_common.action,
                    message:    _("SSRF - Requesting known DNSLOG address: %1%", [hostname]),
                    confidence: 100,
                    algorithm:  'ssrf_common'
                }
            }
        }

        // 算法3 - 检测 AWS/Aliyun 私有地址
        if (algorithmConfig.ssrf_aws.action != 'ignore')
        {
            if (hostname == '169.254.169.254' || hostname == '100.100.100.200')
            {
                return {
                    action:     algorithmConfig.ssrf_aws.action,
                    message:    _("SSRF - Requesting AWS metadata address"),
                    confidence: 100,
                    algorithm:  'ssrf_aws'
                }
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
        if (algorithmConfig.ssrf_obfuscate.action != 'ignore')
        {
            var reason = false

            if (!isNaN(hostname))
            {
                reason = _("SSRF - Requesting numeric IP address: %1%", [hostname])
            }
            // else if (hostname.startsWith('0x') && hostname.indexOf('.') === -1)
            // {
            //     reason = _("SSRF - Requesting hexadecimal IP address: %1%", [hostname])
            // }

            if (reason)
            {
                return {
                    action:     algorithmConfig.ssrf_obfuscate.action,
                    message:    reason,
                    confidence: 100,
                    algorithm:  'ssrf_obfuscate'
                }
            }
        }

        // 算法5 - 特殊协议检查
        if (algorithmConfig.ssrf_protocol.action != 'ignore')
        {
            // 获取协议
            var proto = url.split(':')[0].toLowerCase()

            if (algorithmConfig.ssrf_protocol.protocols.indexOf(proto) != -1)
            {
                return {
                    action:     algorithmConfig.ssrf_protocol.action,
                    message:    _("SSRF - Using dangerous protocol: %1%://", [proto]),
                    confidence: 100,
                    algorithm:  'ssrf_protocol'
                }
            }
        }

        return clean
    })

}


plugin.register('directory', function (params, context) {

    var path        = params.path
    var realpath    = params.realpath
    var appBasePath = context.appBasePath
    var server      = context.server
    var parameter   = context.parameter

    var is_windows  = server.os.indexOf('Windows') != -1
    var language    = server.language

    // 算法1 - 读取敏感目录
    if (algorithmConfig.directory_unwanted.action != 'ignore')
    {
        for (var i = 0; i < forcefulBrowsing.unwantedDirectory.length; i ++) {
            if (realpath == forcefulBrowsing.unwantedDirectory[i]) {
                return {
                    action:     algorithmConfig.directory_unwanted.action,
                    message:    _("WebShell activity - Accessing sensitive folder: %1%", [realpath]),
                    confidence: 100,
                    algorithm:  'directory_unwanted'
                }
            }
        }
    }

    // 算法2 - 用户输入匹配。
    if (algorithmConfig.directory_userinput.action != 'ignore')
    {
        if (is_path_containing_userinput(parameter, params.path, is_windows))
        {
            return {
                action:     algorithmConfig.directory_userinput.action,
                message:    _("Path traversal - Accessing folder specified by userinput, folder is %1%", [realpath]),
                confidence: 90,
                algorithm:  'directory_userinput'
            }
        }
    }

    // 算法3 - 检查PHP菜刀等后门
    if (algorithmConfig.directory_reflect.action != 'ignore')
    {
        // 目前，只有 PHP 支持通过堆栈方式，拦截列目录功能
        if (language == 'php' && validate_stack_php(params.stack))
        {
            return {
                action:     algorithmConfig.directory_reflect.action,
                message:    _("WebShell activity - Using file manager function with China Chopper WebShell"),
                confidence: 90,
                algorithm:  'directory_reflect'
            }
        }
    }

    return clean
})


plugin.register('readFile', function (params, context) {
    var server    = context.server
    var parameter = context.parameter
    var is_win    = server.os.indexOf('Windows') != -1

    // weblogic 下面，所有war包读取操作全部忽略
    if (server['server'] === 'weblogic' && params.realpath.endsWith('.war'))
    {
        return clean
    }

    //
    // 算法1: 简单用户输入识别，拦截任意文件下载漏洞
    //
    // 不影响正常操作，e.g
    // ?path=download/1.jpg
    //
    if (algorithmConfig.readFile_userinput.action != 'ignore')
    {
        // ?path=/etc/./hosts
        // ?path=../../../etc/passwd
        if (is_path_endswith_userinput(parameter, params.path, params.realpath, is_win))
        {
            return {
                action:     algorithmConfig.readFile_userinput.action,
                message:    _("Path traversal - Downloading files specified by userinput, file is %1%", [params.realpath]),
                confidence: 90,
                algorithm: 'readFile_userinput'
            }
        }
        // @FIXME: 用户输入匹配了两次，需要提高效率
        if (is_from_userinput(parameter, params.path))
        {
            // 获取协议，如果有
            var proto = params.path.split('://')[0].toLowerCase()
            // 1. 读取 http(s):// 内容
            // ?file=http://www.baidu.com
            if (proto === 'http' || proto === 'https')
            {
                if (algorithmConfig.readFile_userinput_http.action != 'ignore')
                {
                    return {
                        action:     algorithmConfig.readFile_userinput_http.action,
                        message:    _("SSRF - Requesting http/https resource with file streaming functions, URL is %1%", [params.path]),
                        confidence: 90,
                        algorithm:  'readFile_userinput_http'
                    }
                }
            }

            // 2. 读取特殊协议内容
            // ?file=file:///etc/passwd
            // ?file=php://filter/read=convert.base64-encode/resource=XXX
            if (proto === 'file' || proto === 'php')
            {
                if (algorithmConfig.readFile_userinput_unwanted.action != 'ignore')
                {
                    return {
                        action:     algorithmConfig.readFile_userinput_unwanted.action,
                        message:    _("Path traversal - Requesting unwanted protocol %1%://", [proto]),
                        confidence: 90,
                        algorithm:  'readFile_userinput_unwanted'
                    }
                }
            }
        }
    }

    //
    // 算法2: 文件、目录探针
    // 如果应用读取了列表里的文件，比如 /root/.bash_history，这通常意味着后门操作
    //
    if (algorithmConfig.readFile_unwanted.action != 'ignore')
    {
        var realpath_lc = params.realpath.toLowerCase()
        for (var j = 0; j < forcefulBrowsing.absolutePaths.length; j ++) {
            if (forcefulBrowsing.absolutePaths[j] == realpath_lc) {
                return {
                    action:     algorithmConfig.readFile_unwanted.action,
                    message:    _("WebShell activity - Accessing sensitive file %1%", [params.realpath]),
                    confidence: 90,
                    algorithm:  'readFile_unwanted'
                }
            }
        }
    }

    //
    // 算法3: 检查文件遍历，看是否超出web目录范围 [容易误报~]
    //
    if (algorithmConfig.readFile_outsideWebroot.action != 'ignore')
    {
        var path        = params.path
        var appBasePath = context.appBasePath

        if (is_outside_webroot(appBasePath, params.realpath, path)) {
            return {
                action:     algorithmConfig.readFile_outsideWebroot.action,
                message:    _("Path traversal - accessing files outside webroot (%1%), file is %2%", [appBasePath, params.realpath]),
                confidence: 90,
                algorithm:  'readFile_outsideWebroot'
            }
        }
    }


    return clean
})

plugin.register('include', function (params, context) {
    var url       = params.url
    var server    = context.server
    var parameter = context.parameter
    var is_win    = server.os.indexOf('Windows') != -1
    var realpath  = params.realpath

    // 用户输入检查
    // ?file=/etc/passwd
    // ?file=../../../../../var/log/httpd/error.log
    if (algorithmConfig.include_userinput.action != 'ignore')
    {
        if (is_path_endswith_userinput(parameter, url, realpath, is_win))
        {
            return {
                action:     algorithmConfig.include_userinput.action,
                message:    _("File inclusion - including files specified by user input"),
                confidence: 100,
                algorithm:  'include_userinput'
            }
        }
    }

    // 如果有协议
    // include ('http://xxxxx')
    var items = url.split('://')
    var proto = items[0].toLowerCase()

    // 特殊协议，
    // include('file://XXX')
    // include('php://XXX')
    if (algorithmConfig.include_protocol.action != 'ignore')
    {
        if (algorithmConfig.include_protocol.protocols.indexOf(proto) != -1)
        {
            return {
                action:     algorithmConfig.include_protocol.action,
                message:    _("File inclusion - using unwanted protocol '%1%://' with funtion %2%()", [proto, params.function]),
                confidence: 90,
                algorithm:  'include_protocol'
            }
        }
    }

    return clean
})

plugin.register('writeFile', function (params, context) {

    // 写 NTFS 流文件，通常是为了绕过限制
    if (algorithmConfig.writeFile_NTFS.action != 'ignore')
    {
        if (ntfsRegex.test(params.realpath))
        {
            return {
                action:     algorithmConfig.writeFile_NTFS.action,
                message:    _("File write - Writing NTFS alternative data streams", [params.realpath]),
                confidence: 95,
                algorithm:  'writeFile_NTFS'
            }
        }
    }

    // PUT 上传脚本文件
    if (context.method == 'put' &&
        algorithmConfig.writeFile_PUT_script.action != 'ignore')
    {
        if (scriptFileRegex.test(params.realpath))
        {
            return {
                action:     algorithmConfig.writeFile_PUT_script.action,
                message:    _("File upload - Using HTTP PUT method to upload a webshell", [params.realpath]),
                confidence: 95,
                algorithm:  'writeFile_PUT_script'
            }
        }
    }

    // 关于这个算法，请参考这个插件定制文档
    // https://rasp.baidu.com/doc/dev/official.html#case-file-write
    if (algorithmConfig.writeFile_script.action != 'ignore')
    {
        if (scriptFileRegex.test(params.realpath))
        {
            return {
                action:     algorithmConfig.writeFile_script.action,
                message:    _("File write - Creating or appending to a server-side script file, file is %1%", [params.realpath]),
                confidence: 85,
                algorithm:  'writeFile_script'
            }
        }
    }

    return clean
})



plugin.register('fileUpload', function (params, context) {

    // 是否禁止使用 multipart 上传脚本文件，或者 apache/php 服务器配置文件
    if (algorithmConfig.fileUpload_multipart_script.action != 'ignore')
    {
        if (scriptFileRegex.test(params.filename) || ntfsRegex.test(params.filename))
        {
            return {
                action:     algorithmConfig.fileUpload_multipart_script.action,
                message:    _("File upload - Uploading a server-side script file with multipart/form-data protocol, filename: %1%", [params.filename]),
                confidence: 95,
                algorithm:  'fileUpload_multipart_script'
            }
        }

        if (params.filename == ".htaccess" || params.filename == ".user.ini")
        {
            return {
                action:     algorithmConfig.fileUpload_multipart_script.action,
                message:    _("File upload - Uploading a server-side config file with multipart/form-data protocol, filename: %1%", [params.filename]),
                confidence: 95,
                algorithm:  'fileUpload_multipart_script'
            }
        }
    }

    // 是否禁止 HTML/JS 文件，主要是对抗钓鱼、CORS绕过等问题
    if (algorithmConfig.fileUpload_multipart_html.action != 'ignore')
    {
        if (htmlFileRegex.test(params.filename))
        {
            return {
                action:     algorithmConfig.fileUpload_multipart_html.action,
                message:    _("File upload - Uploading a HTML/JS file with multipart/form-data protocol", [params.filename]),
                confidence: 90,
                algorithm:  'fileUpload_multipart_html'
            }
        }
    }

    return clean
})



if (algorithmConfig.fileUpload_webdav.action != 'ignore')
{
    plugin.register('webdav', function (params, context) {

        // 源文件不是脚本 && 目标文件是脚本，判定为MOVE方式写后门
        if (! scriptFileRegex.test(params.source) && scriptFileRegex.test(params.dest))
        {
            return {
                action:    algorithmConfig.fileUpload_webdav.action,
                message:   _("File upload - Uploading a server-side script file with HTTP method %1%, file is %2%", [
                    context.method, params.dest
                ]),
                confidence: 100,
                algorithm:  'fileUpload_webdav'
            }
        }

        return clean
    })
}

if (algorithmConfig.rename_webshell.action != 'ignore')
{
    plugin.register('rename', function (params, context) {
        // 目标文件在webroot内才认为是写后门
        if (!is_outside_webroot(context.appBasePath, params.dest, null)) {
            // 源文件是干净的文件，目标文件是脚本文件，判定为重命名方式写后门
            if (cleanFileRegex.test(params.source) && scriptFileRegex.test(params.dest))
            {
                return {
                    action:    algorithmConfig.rename_webshell.action,
                    message:   _("File upload - Renaming a non-script file to server-side script file, source file is %1%", [
                        params.source
                    ]),
                    confidence: 90,
                    algorithm:  'rename_webshell'
                }
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
                'java.lang.reflect.Method.invoke':                                              _("Reflected command execution - Unknown vulnerability detected"),
                'ognl.OgnlRuntime.invokeMethod':                                                _("Reflected command execution - Using OGNL library"),
                'com.thoughtworks.xstream.XStream.unmarshal':                                   _("Reflected command execution - Using xstream library"),
                'org.apache.commons.collections4.functors.InvokerTransformer.transform':        _("Reflected command execution - Using Transformer library (v4)"),
                'org.apache.commons.collections.functors.InvokerTransformer.transform':         _("Reflected command execution - Using Transformer library"),
                'org.apache.commons.collections.functors.ChainedTransformer.transform':         _("Reflected command execution - Using Transformer library"),
                'org.jolokia.jsr160.Jsr160RequestDispatcher.dispatchRequest':                   _("Reflected command execution - Using JNDI library"),
                'com.alibaba.fastjson.parser.deserializer.JavaBeanDeserializer.deserialze':     _("Reflected command execution - Using fastjson library"),
                'org.springframework.expression.spel.support.ReflectiveMethodExecutor.execute': _("Reflected command execution - Using SpEL expressions"),
                'freemarker.template.utility.Execute.exec':                                     _("Reflected command execution - Using FreeMarker template"),
                'org.jboss.el.util.ReflectionUtil.invokeMethod':                                _("Reflected command execution - Using JBoss EL method"),
                'com.sun.jndi.rmi.registry.RegistryContext.lookup':                             _("Reflected command execution - Using JNDI registry service"),
            }

            for (var i = 2; i < params.stack.length; i ++) {
                var method = params.stack[i]

                if (method.startsWith('ysoserial.Pwner')) {
                    message = _("Reflected command execution - Using YsoSerial tool")
                    break
                }

                if (method == 'org.codehaus.groovy.runtime.ProcessGroovyMethods.execute') {
                    message = _("Reflected command execution - Using Groovy library")
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
            message = _("WebShell activity - Detected reflected command execution")
        }

        if (message)
        {
            return {
                action:     algorithmConfig.command_reflect.action,
                message:    message,
                confidence: 100,
                algorithm:  'command_reflect'
            }
        }
    }

    // 从 v0.31 开始，当命令执行来自非HTTP请求的，我们也会检测反序列化攻击
    // 但是不应该拦截正常的命令执行，所以这里加一个 context.url 检查
    if (! context.url) {
        return clean
    }

    // 算法2: 检测命令注入，或者命令执行后门
    if (algorithmConfig.command_userinput.action != 'ignore') {
        var cmd        = params.command
        var reason     = false
        var min_length = algorithmConfig.command_userinput.min_length
        var parameters = context.parameter || {}
        var raw_tokens = []

        // 检查命令逻辑是否被用户参数所修改
        function _run(values, name)
        {
            var reason = false

            values.some(function (value) {
                if (value.length <= min_length) {
                    return false
                }

                // 检查用户输入是否存在于命令中
                var userinput_idx = cmd.indexOf(value)
                if (userinput_idx == -1) {
                    return false
                }

                if (cmd.length == value.length) {
                    reason = _("WebShell detected - Executing command: %1%", [cmd])
                    return true
                }

                // 懒加载，需要的时候初始化 token
                if (raw_tokens.length == 0) {
                    raw_tokens = RASP.cmd_tokenize(cmd)
                }

                if (is_token_changed(raw_tokens, userinput_idx, value.length)) {
                    reason = _("Command injection - command structure altered by user input, request parameter name: %1%", [name])
                    return true
                }
            })

            return reason
        }

        // 匹配 GET/POST/multipart 参数
        Object.keys(parameters).some(function (name) {
            // 覆盖场景，后者仅PHP支持
            // ?id=XXXX
            // ?data[key1][key2]=XXX
            var value_list
            if (typeof parameters[name][0] == 'string') {
                value_list = parameters[name]
            } else {
                value_list = Object.values(parameters[name][0])
            }                
            reason = _run(value_list, name)
            if (reason) {
                return true
            }
        })

        if (reason !== false)
        {
            return {
                action:     algorithmConfig.command_userinput.action,
                confidence: 90,
                message:    reason,
                algorithm:  'command_userinput'
            }
        }
    }

    // 算法3: 常用渗透命令
    if (algorithmConfig.command_common.action != 'ignore')
    {
        var reason = false
        if (cmdPostPattern.test(params.command))
        {           
            return {
                action:     algorithmConfig.command_common.action,
                message:    _("Webshell detected - Executing potentially dangerous command, command is %1%", [cmd]),
                confidence: 95,
                algorithm:  'command_common'
            }    
        }     
    }

    // 算法4: 记录所有的命令执行
    if (algorithmConfig.command_other.action != 'ignore') 
    {
        return {
            action:     algorithmConfig.command_other.action,
            message:    _("Command execution - Logging all command execution by default, command is %1%", [cmd]),
            confidence: 90,
            algorithm:  'command_other'
        }
    }

    return clean
})


// 注意: 由于libxml2无法挂钩，所以PHP暂时不支持XXE检测
plugin.register('xxe', function (params, context) {
    var server    = context.server
    var is_win    = server.os.indexOf('Windows') != -1
    var items     = params.entity.split('://')

    if (algorithmConfig.xxe_protocol.action != 'ignore') {
        // 检查 windows + SMB 协议，防止泄露 NTLM 信息
        if (params.entity.startsWith('\\\\')) {
            return {
                action:     algorithmConfig.xxe_protocol.action,
                message:    _("XXE - Using dangerous protocol SMB"),
                confidence: 100,
                algorithm:  'xxe_protocol'
            }
        }
    }

    if (items.length >= 2) {
        var protocol = items[0]
        var address  = items[1]

        // 拒绝特殊协议
        if (algorithmConfig.xxe_protocol.action != 'ignore') {
            if (algorithmConfig.xxe_protocol.protocols.indexOf(protocol) != -1) {
                return {
                    action:     algorithmConfig.xxe_protocol.action,
                    message:    _("XXE - Using dangerous protocol %1%", [protocol]),
                    confidence: 100,
                    algorithm:  'xxe_protocol'
                }
            }

        }

        // file 协议 + 绝对路径, e.g
        // file:///etc/passwd
        //
        // 相对路径容易误报, e.g
        // file://xwork.dtd
        if (algorithmConfig.xxe_file.action != 'ignore')
        {
            if (address.length > 0 && protocol === 'file' && is_absolute_path(address, is_win) )
            {
                var address_lc = address.toLowerCase()

                // 过滤掉 xml、dtd
                if (! address_lc.endsWith('.xml') &&
                    ! address_lc.endsWith('.dtd'))
                {
                    return {
                        action:     algorithmConfig.xxe_file.action,
                        message:    _("XXE - Accessing file %1%", [address]),
                        confidence: 90,
                        algorithm:  'xxe_file'
                    }
                }
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
                    message:    _("OGNL exec - Trying to exploit a OGNL expression vulnerability"),
                    confidence: 100,
                    algorithm:  'ognl_exec'
                }
            }

        }
        return clean
    })
}

if (algorithmConfig.deserialization_transformer.action != 'ignore') {
    plugin.register('deserialization', function (params, context) {
        var deserializationInvalidClazz = [
            'org.apache.commons.collections.functors.ChainedTransformer.transform',
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
                    action:     algorithmConfig.deserialization_transformer.action,
                    message:    _("Transformer deserialization - unknown deserialize vulnerability detected"),
                    confidence: 100,
                    algorithm:  'deserialization_transformer'
                }
            }
        }
        return clean
    })
}

plugin.log('OpenRASP official plugin: Initialized, version', plugin_version)

