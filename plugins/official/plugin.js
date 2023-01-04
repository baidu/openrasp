const plugin_version = '2022-0908-1700'
const plugin_name    = 'official'
const plugin_desc    = '官方插件'

/*
 * Copyright 2017-2021 Baidu Inc.
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
        is_dev:  false,

        // 若 log_event 开启，将打印应用行为信息到 plugin.log
        log_event: false,

        // schema 版本
        schema_version: 1
    },

    // SQL注入算法#1 - 匹配用户输入
    // 1. 用户输入长度至少 8
    // 2. 用户输入至少包含一个SQL关键词 - 即 pre_filter，[默认关闭]
    // 3. 用户输入完整的出现在SQL语句中，且会导致SQL语句逻辑发生变化
    sql_userinput: {
        name:       '算法1 - 用户输入匹配算法',
        action:     'block',
        min_length: 8,
        pre_filter: 'select|file|from|;',
        pre_enable: false,
        anti_detect_filter: 'add|all|alter|analyze|and|any|as|asc|avg|begin|between|by|case|create|count|delete|desc|do|dumpfile|else|elseif|end|exists|false|file|float|flush|follows|from|group|having|identified|if|in|insert|interval|into|join|last|like|limit|loop|not|null|on|or|order|procedure|regexp|return|rlike|select|then|true|union|update|values|xor',
        anti_detect_enable: true,
        lcs_search: false,

        // 是否允许数据库管理器 - 前端直接提交SQL语句
        allow_full: true
    },
    
    // SQL注入算法#2 - 语句规范
    sql_policy: {
        name:    '算法2 - 拦截异常SQL语句',
        action:  'block',

        // 粗规则 - 为了减少 tokenize 次数，当SQL语句包含一定特征时才进入
        // 另外，我们只需要处理增删改查的语句，虽然 show 语句也可以报错注入，但是算法2没必要处理
        pre_filter: ';|\\/\\*|(?:\\d{1,2}\\s*,\\s*){2}|(?:null\\s*,\\s*){2}|0x[\\da-f]{8}|\\W(information_schema|outfile|dumpfile|load_file|benchmark|pg_sleep|sleep|is_srvrolemember|updatexml|extractvalue|hex|char|chr|mid|ord|ascii|bin)\\W',

        feature: {
            // 是否禁止多语句执行，select ...; update ...;
            stacked_query:      false,

            // 是否禁止16进制字符串，select 0x41424344
            no_hex:             false,

            // 禁止版本号注释，select/*!500001,2,*/3
            version_comment:    true,

            // 函数黑名单，具体列表见下方，select load_file(...)
            function_blacklist: true,

            // 敏感函数频次， 具体列表见下方，select chr(123)||chr(123)||chr(123)=chr(123)||chr(123)||chr(123)
            function_count:     false,

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
            hex:              false,
            mid:              false,
            ord:              false,
            ascii:            false,
            bin:              false
        },
        function_count: {
            chr:              5,
            char:             5
        }
    },

    sql_exception: {
        name:      '算法3 - 记录数据库异常',
        action:    'log',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#sql-exception',

        // error_code 最多允许 100 个，超过直接清空
        mysql: {
	        error_code: [
	            // 1045, // Access denied for user 'bae'@'10.10.1.1'
                // 1690, // DOUBLE value is out of range in 'exp(~((select 'root@localhost' from dual)))'
	            1060, // Duplicate column name '5.5.60-0ubuntu0.14.04.1'
	            1062, // Duplicate entry '::root@localhost::1' for key 'group_key'
	            1064, // You have an error in your SQL syntax
	            1105, // XPATH syntax error: '~root@localhost~'
	            1367  // Illegal non geometric 'user()' value found during parsing
	        ]
        },
        pgsql: {
            error_code: [
                "42601", // normal syntax error
                "22P02", // ERROR:  invalid input syntax for type double precision: "DATABASE: test1"
            ],
            error_state: [
                "42601", // normal syntax error
                "22P02", // ERROR:  invalid input syntax for type double precision: "DATABASE: test1"
            ]
        },
        sqlite: {
            error_code: [
                1, // generic error, like syntax error、malformed MATCH expression: ["3.6.23.1] and other
            ]
        },
        oracle: {
            error_code: [
                933,   // SQL command not properly ended
                29257, // host string unknown
                20000, // Oracle Text error
                904,   // invalid identifier
                19202, // Error occurred in XML processing
                1756,  // quoted string not properly terminated
                1740,  // missing double quote in identifier
                920,   // invalid relational operator
                907,   // missing right parenthesis
                911,   // invalid character
            ]
        },
        hsql: {
            error_code: [
                -5583, // malformed quoted identifier
                -5584, // malformed string
                -5590, // unexpected end of statement
            ],
            error_state: [
                "42583", // malformed quoted identifier
                "42584", // malformed string
                "42590", // unexpected end of statement
            ]
        },
        mssql: {
            error_code: [
                105, // Unclosed quotation mark after the character string '%.*ls'.
                245, // Conversion failed when converting the %ls value '%.*ls' to data type %ls.
            ]
        },
        db2: {
            error_state: [
                "42603", // The string constant beginning with "'xxx" does not have an ending string
            ]
        }
    },

    // 此算法仅用于应急，默认不开启，用户编写时应注意redos风险
    sql_regex: {
        name:      '算法4 - 正则表达式算法',
        action:    'ignore',
        regex:     'information_schema'
    },

    // SSRF - 来自用户输入，且为内网地址就拦截
    ssrf_userinput: {
        name:   '算法1 - 用户输入匹配算法（支持 rebind 检测）',
        action: 'block'
    },
    // SSRF - 是否允许访问 aws metadata
    ssrf_aws: {
        name:   '算法2 - 拦截 AWS/Aliyun/GCP metadata 访问',
        action: 'block'
    },
    // SSRF - 是否允许访问 dnslog 地址
    ssrf_common: {
        name:    '算法3 - 拦截常见 dnslog 地址',
        action:  'block'
    },
    // SSRF - 是否允许访问混淆后的IP地址
    ssrf_obfuscate: {
        name:   '算法4 - 拦截混淆地址',
        action: 'ignore'
    },
    // SSRF - 禁止使用 curl 读取 file:///etc/passwd、php://filter/XXXX 这样的内容
    ssrf_protocol: {
        name:      '算法5 - 拦截 php:// 等异常协议',
        action:    'block',
        protocols: [
            'file',
            'gopher',

            // python specific
            'local_file',
            'local-file',

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
        name:       '算法1 - 用户输入匹配算法',
        action:     'block',
        lcs_search: false
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
        action: 'log'
    },

    // 写文件操作 - NTFS 流
    writeFile_NTFS: {
        name:   '算法1 - 拦截 NTFS ::$DATA 写入操作',
        action: 'block'
    },
    // 写文件操作 - PUT 上传脚本文件 - 无法关联实际上传的文件和写文件操作，暂时注释掉
    // writeFile_PUT_script: {
    //     name:   '算法2 - 拦截 PUT 方式上传 php/jsp 等脚本文件',
    //     action: 'block'
    // },
    // 写文件操作 - 脚本文件
    // https://rasp.baidu.com/doc/dev/official.html#case-file-write
    writeFile_script: {
        name:      '算法2 - 拦截 php/jsp 等脚本文件的写入操作',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#case-file-write',
        action:    'block',
        userinput:  true,
        lcs_search: false
    },

    writeFile_reflect: {
        name:      '算法3 - 拦截通过反射、反序列化执行的文件写入操作',
        action:    'log'
    },

    // 任意文件删除 - 使用 ../跳出目录
    deleteFile_userinput: {
        name:      '算法1 - 用户输入匹配，禁止使用 ../ 删除文件',
        action:    'block',
        lcs_search: false
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

    link_webshell: {
        name:   '算法1 - 通过链接方式获取 WebShell',
        action: 'block'
    },

    // 文件管理器 - 用户输入匹配，仅当直接读取绝对路径时才检测
    directory_userinput: {
        name:       '算法1 - 用户输入匹配算法',
        action:     'block',
        lcs_search: false
    },
    // 文件管理器 - 反射方式列目录
    directory_reflect: {
        name:   '算法2 - 通过反射调用，查看目录内容',
        action: 'block'
    },
    // 文件管理器 - 查看敏感目录
    directory_unwanted: {
        name:   '算法3 - 尝试查看敏感目录',
        action: 'log'
    },

    // 文件包含 - 用户输入匹配
    include_userinput: {
        name:       '算法1 - 用户输入匹配算法',
        action:     'block',
        lcs_search: false
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
            // 'phar',
            'compress.zlib',
            'compress.bzip2',
            'zip',
            'rar'
        ]
    },

    // XXE - 代码安全开关，通过调用相关函数直接禁止外部实体
    xxe_disable_entity: {
        name:   '算法1 - 禁止外部实体加载（记录日志等同于完全忽略）',
        action: 'ignore',
        clazz:  {
            // com/sun/org/apache/xerces/internal/jaxp/DocumentBuilderFactoryImpl
            java_dom:   true,

            // org/dom4j/io/SAXReader
            java_dom4j: true,

            // org/jdom/input/SAXBuilder,org/jdom2/input/SAXBuilder
            java_jdom:  true,

            // com/sun/org/apache/xerces/internal/jaxp/SAXParserFactoryImpl
            java_sax:   true,

            // javax/xml/stream/XMLInputFactory
            java_stax:  true
        }
    },

    // XXE - 使用 gopher/ftp/dict/.. 等不常见协议访问外部实体
    xxe_protocol: {
        name:   '算法2 - 使用 ftp:// 等异常协议加载外部实体',
        action: 'block',
        protocols: [
            'ftp',
            'dict',
            'gopher',
            // 'jar', // jenkins下存在误报
            'netdoc',
            'mailto'
        ]
    },
    // XXE - 使用 file 协议读取内容，可能误报，默认 log
    xxe_file: {
        name:      '算法3 - 使用 file:// 协议读取文件',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#case-xxe',
        action:    'log',
    },

    // 文件上传 - COPY/MOVE 方式，仅适合 tomcat
    fileUpload_webdav: {
        name:   '算法1 - MOVE 方式上传脚本文件',
        action: 'block'
    },
    // 文件上传 - Multipart 方式上传脚本文件
    fileUpload_multipart_script: {
        name:   '算法2 - Multipart 方式上传 PHP/JSP 等脚本文件',
        action: 'block'
    },
    // 文件上传 - Multipart 方式上传 HTML/JS 等文件
    fileUpload_multipart_html: {
        name:   '算法3 - Multipart 方式上传 HTML/JS 等文件',
        action: 'ignore'
    },
    // 文件上传 - Multipart 方式上传 DLL/EXE 等文件
    fileUpload_multipart_exe: {
        name:   '算法3 - Multipart 方式上传 DLL/EXE 等文件',
        action: 'ignore'
    },    

    // OGNL 代码执行漏洞
    ognl_blacklist: {
        name:   '算法1 - OGNL语句黑名单',
        action: 'block',
        expression: [
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
            'excludedClasses',
            'excludedPackageNamePatterns',
            'excludedPackageNames',
            'com.opensymphony.xwork2.ActionContext'
        ]
    },
    // OGNL 表达式长度限制
    ognl_length_limit: {
        name: '算法2 - OGNL表达式长度限制',
        action: 'log',
        max_length: 400
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
        min_length: 2,
        java_unexploitable_filter: true,
    },
    // 命令注入 - 常见命令
    command_common: {
        name:    '算法3 - 识别常用渗透命令（探针）',
        action:  'log',
        pattern: 'cat.{1,5}/etc/passwd|nc.{1,30}-e.{1,100}/bin/(?:ba)?sh|bash\\s-.{0,4}i.{1,20}/dev/tcp/|subprocess.call\\(.{0,6}/bin/(?:ba)?sh|fsockopen\\(.{1,50}/bin/(?:ba)?sh|perl.{1,80}socket.{1,120}open.{1,80}exec\\(.{1,5}/bin/(?:ba)?sh|\\{echo,.{10,400}{base64,-d}'
    },
    // 命令执行 - 语法错误和敏感操作
    command_error: {
        name:   '算法4 - 查找语法错误和敏感操作',
        action: 'log',

        unbalanced_quote_enable: true,

        sensitive_cmd_enable: true,
        concat_char: ["|", ";"],
        sensitive_cmd: ["curl", "bash", "cat", "sh"],

        alarm_token_enable: true,
        alarm_token: ["$IFS", "${IFS}"]
    },
    // 命令执行 - 是否拦截所有命令执行？如果没有执行命令的需求，可以改为 block，最大程度的保证服务器安全
    command_other: {
        name:   '算法5 - 记录或者拦截所有命令执行操作',
        action: 'ignore'
    },
    // 命令注入 - dnslog
    command_dnslog: {
        name:    '算法6 - dnslog类命令',
        action:  'log',
        pattern_cmd: '(^|\\W)(curl|ping|wget|nslookup|dig)\\W',
        pattern_domain: '\\.((ceye|exeye|sslip|nip)\\.io|dnslog\\.cn|(vcap|bxss)\\.me|xip\\.(name|io)|burpcollaborator\\.net|tu4\\.org|2xss\\.cc|request\\.bin|requestbin\\.net|pipedream\\.net)'
    },

    // transformer 反序列化攻击
    deserialization_blacklist: {
        name:   '算法1 - 反序列化黑名单过滤',
        action: 'block',
		clazz: [
            'org.apache.commons.collections.functors.ChainedTransformer',
            'org.apache.commons.collections.functors.InvokerTransformer',
            'org.apache.commons.collections.functors.InstantiateTransformer',
            'org.apache.commons.collections4.functors.InvokerTransformer',
            'org.apache.commons.collections4.functors.InstantiateTransformer',
            'org.codehaus.groovy.runtime.ConvertedClosure',
            'org.codehaus.groovy.runtime.MethodClosure',
            'org.springframework.beans.factory.ObjectFactory',
            'org.apache.xalan.xsltc.trax.TemplatesImpl',
            'com.sun.org.apache.xalan.internal.xsltc.trax.TemplatesImpl',
            'com.mchange.v2.c3p0.impl.PoolBackedDataSourceBase'
        ]
    },

    jndi_disable_all: {
        name:   '算法1 - 拦截所有JNDI调用',
        action: 'block'
    },

    dns_blacklist: {
        name:   '算法1 - 拦截DNS黑名单查询(比如DNSLog)',
        action: 'block'
    },

    // xss 用户输入匹配算法
    // 1. 当用户输入长度超过15，匹配上标签正则，且出现在响应里，直接拦截
    // 2. 当用户输入长度超过15，匹配上标签正则这样的参数个数超过 10，判定为扫描攻击，直接拦截（v1.1.2 之后废弃）
    xss_userinput: {
        name:   '算法2 - 拦截输出在响应里的反射XSS',
        action: 'ignore',

        filter_regex: "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/>\\x00-\\x20]",
        min_length:   15,

        // v1.1.2 之后废弃
        max_detection_num: 10
    },

    // php 专有算法
    xss_echo: {
        name:   '算法1 - PHP: 禁止直接输出GPC参数',
        action: 'log',

        filter_regex: "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/>\\x00-\\x20]"
    },    

    webshell_eval: {
        name:   '算法1 - 拦截简单的PHP中国菜刀后门',
        action: 'block'
    },

    webshell_command: {
        name:   '算法2 - 拦截简单的PHP命令执行后门',
        action: 'block'
    },

    webshell_file_put_contents: {
        name:   '算法3 - 拦截简单的PHP文件上传后门',
        action: 'block'
    },

    webshell_callable: {
        name:   '算法4 - 拦截简单的PHP array_map/walk/filter 后门',
        action: 'block',
        functions: [
            'system', 'exec', 'passthru', 'proc_open', 'shell_exec', 'popen', 'pcntl_exec', 'assert'
        ]
    },

    webshell_ld_preload: {
        name:   '算法5 - 拦截PHP putenv 相关后门',
        action: 'block',
        env: [
            'LD_PRELOAD',
            'LD_AUDIT',
            'GCONV_PATH'
        ]
    },

    eval_regex: {
        name:   '算法1 - 正则表达式',
        action: 'ignore',
        regex:  'base64_decode|gzuncompress|create_function'
    },

    loadLibrary_unc: {
        name:   '算法1 - 拦截 UNC 路径类库加载',
        action: 'block'
    },

    // loadLibrary_other: {
    //     name:   '算法2 - 记录或者拦截所有类库加载',
    //     action: 'ignore'
    // },

    response_dataLeak: {
        name:   '算法1 - 检查响应里是否有敏感信息',
        action: 'ignore',

        // 检查类型
        kind: {
            phone:         true,
            identity_card: true,
            bank_card:     true
        },

        // Content-Type 过滤
        content_type: 'html|json|xml'
    }
}

// END ALGORITHM CONFIG //

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
	    '/etc/issue',
        '/etc/shadow',
        '/etc/passwd',
        // '/etc/hosts',
        '/etc/apache2/apache2.conf',
        '/root/.bash_history',
        '/root/.bash_profile',
        'c:\\windows\\system32\\inetsrv\\metabase.xml',
        'c:\\windows\\system32\\drivers\\etc\\hosts'
    ]
}

// 指定检测header注入时检测的header名, 统一使用小写
var headerInjection = ["user-agent", "referer", "x-forwarded-for"]

// 如果你配置了非常规的扩展名映射，比如让 .abc 当做PHP脚本执行，那你可能需要增加更多扩展名
var scriptFileRegex = /\.(aspx?|jspx?|php[345]?|phar|phtml|sh|py|pl|rb|so|dll|dylib)\.?$/i

// 正常文件
var cleanFileRegex  = /\.(jpg|jpeg|png|gif|bmp|txt|rar|zip)$/i

// 文件读取扩展名白名单，包含 压缩文件 office文件 图片文件
var readFileWhiteExt = new RegExp(/\.(do[c|t][x|m|]?|xl[s|t][x|m|b]?|pp[t|s|a][x|m]?|pot[x|m]|7z|tar|gz|bz2|xz|rar|zip|jpg|jpeg|png|gif|bmp|txt|)$/, 'i')

// 匹配 HTML/JS 等可以用于钓鱼、domain-fronting 的文件
var htmlFileRegex   = /\.(htm|html|js)$/i

// 匹配 EXE/DLL 等可以执行的文件
var exeFileRegex    = /\.(exe|dll|scr|vbs|cmd|bat)$/i

// 其他的 stream 都没啥用
var ntfsRegex       = /::\$(DATA|INDEX)$/

// 已知用户输入匹配算法误报: 传入 1,2,3,4 -> IN(1,2,3,4) 和 传入 column_name, column_pass -> select column_name, column_pass from xxx
var commaSeparatedRegex   = /^(, *)?(([a-zA-Z_]\w*|[0-9+\-x\.]+) *, *)+([a-zA-Z_]\w*|[0-9+\-x\.]+)$/

// 匹配内网地址
var internalRegex   = /^(0\.0\.0|127|10|192\.168|172\.(1[6-9]|2[0-9]|3[01]))\./

// ssrf白名单主机名
var whiteHostName   = /\.bcebos\.com$|(^|\.)oss-[\d\w\-]{0,30}\.aliyuncs\.com$/

var dnsLogDomains   = [
    '.vuleye.pw', '.ceye.io', '.exeye.io', '.vcap.me', '.xip.name', '.xip.io', '.sslip.io', '.nip.io', '.oastify.com', '.eyes.sh',
    '.burpcollaborator.net', '.tu4.org', '.2xss.cc', '.bxss.me', '.godns.vip', '.dnslog.cn', '.0kee.360.cn', '.r87.me','.ngrok.io',
    // yumusb/DNSLog-Platform-Golang
    '.xn--9tr.com', 
    // requestbin 新地址
    '.pipedream.net',
    // 端口转发工具
    '.vxtrans.com', '.vxtrans.link',
    // 免费DDNS厂商
    '.hopto.org', '.zapto.org', '.sytes.net', '.ddns.net',
    // 其他
    '.canarytokens.com'
]

// SQL注入算法1 - 预过滤正则
var sqliPrefilter1  = new RegExp(algorithmConfig.sql_userinput.pre_filter, 'i')

// SQL注入算法1 - 反探测正则
var sqliAntiDetect = new RegExp(algorithmConfig.sql_userinput.anti_detect_filter, 'i')

// SQL注入算法2 - 预过滤正则
var sqliPrefilter2  = new RegExp(algorithmConfig.sql_policy.pre_filter, 'i')

// SQL注入算法 - 管理器白名单
var sqliWhiteManager  = new RegExp(/phpmyadmin/, 'i')

// java 匹配可能可利用的命令注入
var cmdJavaExploitable = new RegExp(/^[^ ]*sh.{1,12}-c/, 'i')

// 命令执行探针 - 常用渗透命令
var cmdPostPattern  = new RegExp(algorithmConfig.command_common.pattern, 'i')

// 命令执行探针 - dnslog命令
var cmdDNSlogPatternCmd  = new RegExp(algorithmConfig.command_dnslog.pattern_cmd)
var cmdDNSlogPatternDomain  = new RegExp(algorithmConfig.command_dnslog.pattern_domain, 'i')

// 敏感信息泄露 - Content Type 正则
var dataLeakContentType = new RegExp(algorithmConfig.response_dataLeak.content_type, 'i')

if (! RASP.is_unittest)
{
   // 记录日志模式: 将所有 block 改为 log
   if (algorithmConfig.meta.all_log)
   {
        Object.keys(algorithmConfig).forEach(function (name) {
            // XXE 外部实体开关不受影响
            if (name != 'xxe_disable_entity')
            {
                if (algorithmConfig[name].action == 'block') 
                {
                    algorithmConfig[name].action = 'log'
                }
            }
        })
    }

    // 研发模式: 
    // 1. 开启更多消耗性能的检测算法
    // 2. 非攻击情况，检测到漏洞也报警
    if (algorithmConfig.meta.is_dev) 
    {
        // 关闭 select 预过滤正则
        algorithmConfig.sql_userinput.pre_enable = false

        // 关闭 1,2,3 误报过滤
        commaSeparatedRegex = /^$/

        // 关闭 xss_echo 非攻击过滤
        algorithmConfig.xss_echo.filter_regex = ""
    }
}
else {
    algorithmConfig.eval_regex.action = "log"
}

// 校验 sql_regex 正则是否合法
if (algorithmConfig.sql_regex.action != 'ignore') {
    if (! algorithmConfig.sql_regex.regex.trim()) {
        plugin.log ("algorithmConfig.sql_regex.regex is empty, algorithm disabled")
        algorithmConfig.sql_regex.action = 'ignore'
    } else {
        try {
            new RegExp(algorithmConfig.sql_regex)
        } catch (e) {
            plugin.log ("Invalid regex in algorithmConfig.sql_regex.regex: ", e)
            algorithmConfig.sql_regex.action = 'ignore'
        } 
    }
}

// 校验 eval_regex 正则是否合法
if (algorithmConfig.eval_regex.action != 'ignore') {
    if (! algorithmConfig.eval_regex.regex.trim()) {
        plugin.log ("algorithmConfig.eval_regex.regex is empty, algorithm disabled")
        algorithmConfig.eval_regex.action = 'ignore'
    } else {
        try {
            new RegExp(algorithmConfig.eval_regex)
        } catch (e) {
            plugin.log ("Invalid regex in algorithmConfig.eval_regex.regex: ", e)
            algorithmConfig.eval_regex.action = 'ignore'
        } 
    }
}


// 常用函数
String.prototype.replaceAll = function(token, tokenValue, maxLength) {
    if (maxLength === undefined) {
        if (this.length * 2 < 4096) {
            maxLength = 4096
        } else {
            maxLength = this.length * 2
        }
    }
    // 空值判断，防止死循环
    if (! token || token.length == 0 || this.length > maxLength) {
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

// 判断参数是否包含路径穿越，比path更严格
function param_has_traversal (param) {
    // 左右斜杠，一视同仁
    var path = "/" + param.replaceAll('\\', '/') + "/"

    if (path.indexOf("/../") != -1)
    {
        return true
    }
    return false
}

function is_hostname_dnslog(hostname) {
    for (var i = 0; i < dnsLogDomains.length; i ++)
    {
        if (hostname.toLowerCase().endsWith(dnsLogDomains[i]))
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

function is_method_from_rasp(stack) {
    // 检查栈顶 -> rasp堆栈之间，是否包含用户代码，即非 JDK相关的函数
    for (; i < stacks.length; i ++) {
        var method = stacks[i]                
        if (! method.startsWith('java.') 
            && !method.startsWith('sun.') 
            && !method.startsWith('com.sun.'))
        {
            return false
        }
    }
    return true
}

function validate_stack_java(stacks) {
    var known    = {
        'com.thoughtworks.xstream.XStream.unmarshal':                                   "Using xstream library",
        'java.beans.XMLDecoder.readObject':                                             "Using WebLogic XMLDecoder library",
        'org.apache.commons.collections4.functors.InvokerTransformer.transform':        "Using Transformer library (v4)",
        'org.apache.commons.collections.functors.InvokerTransformer.transform':         "Using Transformer library",
        'org.apache.commons.collections.functors.ChainedTransformer.transform':         "Using Transformer library",
        'org.jolokia.jsr160.Jsr160RequestDispatcher.dispatchRequest':                   "Using JNDI library (JSR 160)",
        'com.sun.jndi.rmi.registry.RegistryContext.lookup':                             "Using JNDI registry service",
        'org.apache.xbean.propertyeditor.JndiConverter':                                "Using JNDI binding class",
        'com.ibatis.sqlmap.engine.transaction.jta.JtaTransactionConfig':                "Using JTA transaction manager",
        'com.sun.jndi.url.ldap.ldapURLContext.lookup':                                  "Using LDAP factory service",
        'com.alibaba.fastjson.JSON.parse':                                              "Using fastjson library",
        'com.alibaba.fastjson.JSON.parseObject':                                        "Using fastjson library",
        'com.alibaba.fastjson.JSON.parseArray':                                         "Using fastjson library",
        'org.springframework.expression.spel.support.ReflectiveMethodExecutor.execute': "Using SpEL expressions",
        'freemarker.template.utility.Execute.exec':                                     "Using FreeMarker template",
        'org.jboss.el.util.ReflectionUtil.invokeMethod':                                "Using JBoss EL method",
        'org.codehaus.groovy.runtime.ProcessGroovyMethods.execute':                     "Using Groovy library",
        'bsh.Reflect.invokeMethod':                                                     "Using BeanShell library",
        'jdk.scripting.nashorn/jdk.nashorn.internal.runtime.ScriptFunction.invoke':     "Using Nashorn engine",
        'org.apache.shiro.io.DefaultSerializer.deserialize':                            "Using Shiro framework (DefaultSerializer)",
        'com.mchange.v2.c3p0.impl.PoolBackedDataSourceBase.readObject':                 "Using C3p0 library"
    }

    var userCode = false, reachedInvoke = false, i = 0, message = undefined

    // v1.1.1 要求在堆栈里过滤 com.baidu.openrasp 相关的类，因为没有实现正确而产生了多余的反射堆栈，这里需要兼容下防止误报
    // v1.1.2 修复了这个问题，即堆栈顶部为命令执行的方法
    if (stacks.length > 3
        && stacks[0].startsWith('sun.reflect.GeneratedMethodAccessor')
        && stacks[1] == 'sun.reflect.GeneratedMethodAccessorImpl.invoke'
        && stacks[2] == 'java.lang.reflect.Method.invoke')
    {
        i = 3
    }

    for (; i < stacks.length; i ++) {
        var method = stacks[i]                

        // 检查反射调用 -> 命令执行之间，是否包含用户代码
        if (! reachedInvoke) {
            if (method == 'java.lang.reflect.Method.invoke') {
                reachedInvoke = true
            }

            // 用户代码，即非 JDK、com.baidu.openrasp 相关的函数
            if (! method.startsWith('java.') 
                && !method.startsWith('sun.') 
                && !method.startsWith('com.sun.') 
                && !method.startsWith('com.baidu.openrasp.')) 
            {
                userCode = true
            }
        }

        if (method.startsWith('ysoserial.Pwner')) {
            message = "Using YsoSerial tool"
            break
        }

        if (method.startsWith('org.su18')) {
            message = "Using ysuserial tool"
            break
        }

        if (method.startsWith('net.rebeyond.behinder')) {
            message = "Using BeHinder defineClass webshell"
            break
        }

        if (method.startsWith('com.fasterxml.jackson.databind.')) {
            message = "Using Jackson deserialze method"
            break
        }

        // 对于如下类型的反射调用:
        // 1. 仅当命令直接来自反射调用才拦截
        // 2. 如果某个类是反射生成，这个类再主动执行命令，则忽略
        if (! userCode) {
            if (method == 'ognl.OgnlRuntime.invokeMethod') {
                message = "Using OGNL library"
                break
            }  else if (method == 'java.lang.reflect.Method.invoke') {
                message = "Unknown vulnerability detected"
            }
        }

        if (known[method]) {
            message = known[method]
        }
    }
    return message
}

function validate_stack_php(stacks) {
    var verdict = false
    var eval_count = 0

    for (var i = 0; i < stacks.length; i ++) {
        var stack = stacks[i]

        // 来自 eval/assert/create_function/...
        if (stack.indexOf('runtime-created function') != -1
            || stack.indexOf('regexp code@') != -1) {
            verdict = true
            break
        }
        // eval/assert 出现两次以上才认为是webshell
        if (stack.indexOf('eval()\'d code') != -1
            || stack.indexOf('assert code@') != -1) {
            eval_count++
            if (eval_count > 1) {
                verdict = true
                break
            }
        }

            // call_user_func/call_user_func_array 两个函数调用很频繁
        // 必须是 call_user_func 直接调用 system/exec 等函数才拦截，否则会有很多误报
        if (stack.indexOf('@call_user_func') != -1) {
            // 过滤内部安全编码库
            if (stack.indexOf('safesdk-php') != -1) {
                continue
            }
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
function is_path_endswith_userinput(parameter, target, realpath, is_windows, is_lcs_search)
{
    var verdict = false

    Object.keys(parameter).some(function (key) {
        // 只处理非数组、hash情况
        Object.values(parameter[key]).some(function (value){
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
            var simplifiedTarget

            // Windows 下面
            // 传入 ../../../conf/tomcat-users.xml
            // 看到 c:\tomcat\webapps\root\..\..\conf\tomcat-users.xml
            if (is_windows) {
                value = value.replaceAll('/', '\\')
                target = target.replaceAll('/', '\\')
                realpath = realpath.replaceAll('/', '\\')
                simplifiedTarget = target.replaceAll('\\\\','\\').replaceAll('\\.\\', '\\')
                simplifiedValue = value.replaceAll('\\\\','\\').replaceAll('\\.\\', '\\')
            } else{
                simplifiedTarget = target.replaceAll('//','/').replaceAll('/./', '/')
                simplifiedValue = value.replaceAll('//','/').replaceAll('/./', '/')
            }
            var simplifiedValues
            if ( is_lcs_search ) {
                simplifiedValues = lcs_search( simplifiedValue, simplifiedTarget )
            }
            else {
                simplifiedValues = [simplifiedValue]
            }
            for(var i = 0, len = simplifiedValues.length; i < len; i++) {
                simplifiedValue = simplifiedValues[i]
                // 参数必须有跳出目录，或者是绝对路径
                if ((target.endsWith(value) || simplifiedTarget.endsWith(simplifiedValue))
                    && (param_has_traversal(value) || value == realpath || simplifiedValue == realpath))
                {
                    verdict = true
                    return true
                }
            }
        })
        if (verdict){
            return true
        }
    })
    return verdict
}

// 检查是否包含用户输入 - 适合目录
function is_path_containing_userinput(parameter, target, is_windows, is_lcs_search)
{
    var verdict = false
    if (is_windows) {
        target = target.replaceAll('/', '\\').replaceAll('\\\\', '\\')
    }
    else{
        target = target.replaceAll('//', '/')
    }

    Object.keys(parameter).some(function (key) {
        var values = parameter[key]
        Object.values(values).some(function(value){
            // 只处理字符串类型的
            if (typeof value != 'string') {
                return
            }
            if (is_windows) {
                value = value.replaceAll('/', '\\').replaceAll('\\\\', '\\')
            }
            else {
                value = value.replaceAll('//', '/')
            }
            var values
            if (is_lcs_search) {
                values = lcs_search(value, target)
            }
            else {
                // java 下面，传入 /usr/ 会变成 /usr，所以少匹配一个字符
                if ( value.charAt(value.length - 1) == "/" || 
                    value.charAt(value.length - 1) == "\\" ) {
                    value = value.substr(0, value.length - 1)
                }
                values = [value]
            }
            for(var i = 0, len = values.length; i < len; i++) {
                // 只处理非数组、hash情况
                if (param_has_traversal(values[i]) && target.indexOf(values[i]) != -1) {
                    verdict = true
                    return true
                }
            }
        })
        if (verdict){
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
        var values = parameter[key]
        Object.values(values).some(function(value){
            // 只处理非数组、hash情况
            if (value == target) {
                verdict = true
                return true
            }
        })
    })
    return verdict
}

// 是否包含于用户输入 - 适合任意类型参数
function is_include_in_userinput(parameter, target)
{
    var verdict = false
    Object.keys(parameter).some(function (key) {
        var values = parameter[key]
        Object.values(values).some(function(value){
            // 只处理非数组、hash情况
            if (value.indexOf(target) != -1) {
                verdict = true
                return true
            }
        })
    })
    return verdict
}

// 检查逻辑是否被用户参数所修改
function is_token_changed(raw_tokens, userinput_idx, userinput_length, distance, is_sql)
{
    if (is_sql === undefined) {
        is_sql = false
    }
    // 当用户输入穿越了多个token，就可以判定为代码注入，默认为2
    var start = -1, end = raw_tokens.length, distance = distance || 2

    // 寻找 token 起始点，可以改为二分查找
    for (var i = 0; i < raw_tokens.length; i++)
    {
        if (raw_tokens[i].stop > userinput_idx)
        {
            start = i
            break
        }
    }

    // 注释可能在结尾，防止去掉注释导致的越界
    if (start == -1) {
        return false
    }

    // 寻找 token 结束点

    if (raw_tokens[start].stop >= userinput_idx + userinput_length) {
        // 大部分用户输入都只包含在一个token中，只需一次判定
        end = start
    } else {
        // 不在一个token内，按顺序查找
        // 这里需要返回真实distance, 删除 最多需要遍历 distance 个 token  i < start + distance 条件
        for (var i = start + 1; i < raw_tokens.length; i++)
        {
            if (raw_tokens[i].stop >= userinput_idx + userinput_length)
            {
                if (raw_tokens[i].start >= userinput_idx + userinput_length) {
                    end = i - 1
                    break
                } else {
                    end = i
                    break
                }
            }
        }
    }
    
    var diff = end - start + 1
    if (diff >= distance) {
        if (is_sql && algorithmConfig.sql_userinput.anti_detect_enable && diff < 10) {
            var non_kw = 0
            for (var i = start; i <= end; i++) {
                sqliAntiDetect.test(raw_tokens[i].text) || non_kw ++
                if (non_kw >= 2) {
                    return true
                }
            }
            return false
        }
        return true
    }
    return false
}

// 查找str1和str2的最长公共子串，返回为所有最长子串组成的数组
function lcs_search(str1, str2){
    var len1 = str1.length;
    var len2 = str2.length;
    var dp_arr = [[],[]]
    var pre = 1
    var now = 0
    var result =0
    var result_pos = []

    for (var i = 0; i <= len2+1; i ++) {
        dp_arr[0][i] = 0
        dp_arr[1][i] = 0
    }
    for (var i = 0; i <= len1; i ++) {
        for (var j = 0; j <= len2; j ++) {
            if ( i == 0 || j == 0 ){
                dp_arr[now][j] = 0
            }
            else if ( str1[i-1] == str2[j-1] ) {
                dp_arr[now][j] = dp_arr[pre][j-1] + 1
                if (dp_arr[now][j] > result){
                    result = dp_arr[now][j]
                    result_pos = [i - result]
                }else if (dp_arr[now][j] == result){
                    result_pos.push( i - result )
                }
            }
            else {
                dp_arr[now][j] = 0
            }
        }
        if( now == 0 ){
            now = 1
            pre = 0
        }
        else {
            now = 0
            pre = 1
        }
    }
    var result_pos_set = new Set(result_pos)
    var result_str = new Set()
    for (var item of result_pos_set) {
        result_str.add(str1.substr(item, result))
    }
    return Array.from(result_str)
}

// 从字符串中解析cookie
function get_cookies(cookie_str) {
    cookie_items = cookie_str.split(';')
    var result = {}
    for(i = 0; i < cookie_items.length; i++) {
        item = cookie_items[i].trim()
        if (item.length == 0) {
            continue
        }
        else {
            key_len = item.indexOf("=")
            if (key_len <= 0) {
                continue
            }
            key = unescape(item.substr(0, key_len))
            value = unescape(item.substr(key_len + 1))
            result[key] = value
        }
    }
    return result
}

// 合并context.parameter中 header、cookie、parameter、json参数， header、cookie的key会被重命名
function get_all_parameter(context) {
    if (context.get_all_parameter !== undefined) {
        return context.parameter || {}
    }
    context.get_all_parameter = true
    var key_num = 0
    var parameter = context.parameter || {}
    if (context.header != null) {
        for (name in context.header) {
            if (name.toLowerCase() == "cookie") {
                var cookies = get_cookies(context.header.cookie)
                for (name in cookies) {
                    while ("cookie" + key_num + "_" + name in parameter) {
                        key_num++
                    }
                    parameter["cookie" + key_num + "_" + name] = [cookies[name]]
                }
            } else if (headerInjection.indexOf(name.toLowerCase()) != -1) {
                while ("header" + key_num + "_" + name in parameter) {
                    key_num++
                }
                parameter["header" + key_num + "_" + name] = [context.header[name]]
            }
        }
    }
    var jsons = [
        [context.json || {}, "input_json"]
    ]
    while (jsons.length > 0) {
        var json_arr = jsons.pop()
        var crt_json_key = json_arr[1]
        var json_obj = json_arr[0]
        for (item in json_obj) {
            if (typeof json_obj[item] == "string") {
                while ("json" + key_num + "_" + crt_json_key + "->" + item in parameter) {
                    key_num++
                }
                parameter["json" + key_num + "_" + crt_json_key + "->" + item] = [json_obj[item]]
            } else if (typeof json_obj[item] == "object") {
                jsons.push([json_obj[item], crt_json_key + "->" + item])
            }
        }
    }
    return parameter
}

function check_internal_ip(ip, origin_ip) {
    // origin_ip不为空且全部为内网地址则跳过
    if (origin_ip && origin_ip.every(function(value) {
            return internalRegex.test(value)
        })){ return }

    for (var i=0; i<ip.length; i++) {
        if (internalRegex.test(ip[i]))
        {
            return {
                action:     algorithmConfig.ssrf_userinput.action,
                message:    _("SSRF - Requesting intranet address: %1%", [ ip[i] ]),
                confidence: 100,
                algorithm:  'ssrf_userinput'
            }
        }
    }
}

function check_internal_hostname(hostname, origin_hostname) {
    if ((origin_hostname) && (origin_hostname == '[::]' || origin_hostname == '[::1]')) {
        return
    }
    if (hostname == '[::]' || hostname == '[::1]') 
    {
        return {
            action:     algorithmConfig.ssrf_userinput.action,
            message:    _("SSRF - Requesting intranet address: %1%", [ hostname ]),
            confidence: 100,
            algorithm:  'ssrf_userinput'
        }
    }
}

function check_internal(params, context, is_redirect) {
    var ret
    var all_parameter = get_all_parameter(context)
    if (is_redirect) {
        ret = check_internal_ip(params.ip, params.origin_ip)
        if (ret && !whiteHostName.test(params.hostname)) {return ret}
        ret = check_internal_hostname(params.hostname, params.origin_hostname)
        if (ret) {return ret}
    }
    else if (is_from_userinput(all_parameter, params.url)) {
        // 非重定向，判定用户输入
        ret = check_internal_ip(params.ip, undefined)
        if (ret && !whiteHostName.test(params.hostname)) {return ret}
        ret = check_internal_hostname(params.hostname, undefined)
        if (ret) {return ret}
    }
}

function check_ssrf(params, context, is_redirect) {
    var hostname  = params.hostname
    var url       = params.url
    var ip        = params.ip
    var reason    = false

    // 算法1 - 当参数来自用户输入，且为内网IP，判定为SSRF攻击
    if (algorithmConfig.ssrf_userinput.action != 'ignore')
    {
        var ret
        ret = check_internal(params, context, is_redirect)
        // 过滤非HTTP请求（dubbo)
        var header = context.header || {}
        if (ret && Object.keys(header).length != 0) {
            return ret
        }
    }

    // 算法2 - 检查常见探测域名
    if (algorithmConfig.ssrf_common.action != 'ignore')
    {
        if (is_hostname_dnslog(hostname) || ['requestb.in', 'transfer.sh'].includes(hostname.toLowerCase()))
        {
            return {
                action:     algorithmConfig.ssrf_common.action,
                message:    _("SSRF - Requesting known DNSLOG address: %1%", [hostname]),
                confidence: 100,
                algorithm:  'ssrf_common'
            }
        }
    }

    // 算法3 - 检测 AWS/Aliyun/GoogleCloud 私有地址: 拦截IP访问、绑定域名访问两种方式
    if (algorithmConfig.ssrf_aws.action != 'ignore')
    {
        if (ip == '169.254.169.254' || ip == '100.100.100.200' || ip == '168.63.129.16'
            || hostname == '169.254.169.254' || hostname == '100.100.100.200' || hostname == '168.63.129.16' 
            || hostname == 'metadata.google.internal')
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

        if (!isNaN(hostname) && hostname.length != 0)
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

// 如果开启记录日志，先打印日志，再执行后续逻辑
if (algorithmConfig.meta.log_event) {

    plugin.register('directory', function (params, context) {
        plugin.log('Listing directory content: ' + params.realpath, params.stack)
        return clean
    })

    plugin.register('fileUpload', function (params, context) {
        plugin.log('File upload: ' + params.filename)
        return clean
    })

    plugin.register('rename', function (params, context) {
        plugin.log('Rename file - From ' + params.source + ' to ' + params.dest)  
        return clean
    })

    plugin.register('ssrf', function (params, context) {
        plugin.log('SSRF requesting ' + params.url + ' (IP: ' + params.ip + ')')
        return clean
    })

    plugin.register('command', function (params, context) {
        plugin.log('Execute command: ' + params.command, params.stack)
        return clean
    })

    plugin.register('ognl', function (params, context) {
        plugin.log('Evaluating OGNL expression: ' + params.expression)
        return clean
    })

    plugin.register('xxe', function (params, context) {
        plugin.log('Loading XML entity: ' + params.entity)
        return clean
    })

    plugin.register('eval', function (params, context) {
        plugin.log('Evaluating code: ' + params.code)
        return clean
    })

    plugin.register('loadLibrary', function (params, context) {
        plugin.log('Loading library: ' + params.path)
        return clean
    })

    plugin.register('include', function (params, context) {
        plugin.log('Include file: ' + params.url)
        return clean
    })

    plugin.register('readFile', function (params, context) {
        plugin.log('Read file: ' + params.realpath)
        return clean
    })

    plugin.register('writeFile', function (params, context) {
        plugin.log('Write file: ' + params.realpath)
        return clean
    })

    plugin.register('sql', function (params, context) {
        plugin.log('SQL query: ' + params.query)
        return clean
    })

    plugin.register('jndi', function (params, context) {
        plugin.log('JNDI lookup: ' + params.name, params.stack)
        return clean
    })

    plugin.register('dns', function (params, context) {
        plugin.log('dns lookup: ' + params.host, params.stack)
        return clean
    })    
}


// 若开启「研发模式」，将只使用JS插件
if (! algorithmConfig.meta.is_dev && RASP.get_jsengine() !== 'v8') {
    // v1.1 之前的版本，SQL/SSRF 使用 java 原生实现，需要将插件配置传递给 java
    // v1.0 RC1 之前仍然需要使用 RASP.config_set 传递配置
    if (RASP.config_set) {
        RASP.config_set('algorithm.config', JSON.stringify(algorithmConfig))
    }
    // 用于 v1.0 rhino 
    Object.values = function (obj){
        var result = Array()
        for (key in obj) {
            result.push(obj[key])
        }
        return result
    }
} else {
    // 对于PHP + V8，性能还不错，我们保留JS检测逻辑
    plugin.register('sql', function (params, context) {

        var reason          = false
        var min_length      = algorithmConfig.sql_userinput.min_length
        var allow_full      = algorithmConfig.sql_userinput.allow_full
        var parameters      = context.parameter || {}
        var json_parameters = context.json || {}
        var raw_tokens      = []

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

                // 使用lcs查找或直接查找
                if (algorithmConfig.sql_userinput.lcs_search) {
                    check_value = lcs_search(params.query, value)
                }
                else{
                    check_value = [value]
                }

                // 检查用户输入是否存在于SQL中
                for(var i = 0, len = check_value.length; i < len; i++) {
                    value = check_value[i]

                    // 过滤超短参数
                    if (value.length < 3) {
                        continue
                    }
                
                    var userinput_idx = params.query.indexOf(value)
                    if (userinput_idx == -1) {
                        return false
                    }

                    // 如果允许数据库管理器
                    if (allow_full && params.query.length == value.length)
                    {
                        return false
                    }

                    // 过滤已知误报
                    // 1,2,3,4,5 和 user_id, user_name, user_pass
                    if (commaSeparatedRegex.test(value)) {
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

                    //distance用来屏蔽identifier token解析误报 `dbname`.`table`，请在1.2版本后删除
                    var distance = 2
                    if (value.length > 20) {
                        distance = 3
                    }
                    if (is_token_changed(raw_tokens, userinput_idx, value.length, distance, is_sql=true)) {
                        reason = _("SQLi - SQL query structure altered by user input, request parameter name: %1%, value: %2%", [name, value])
                        return true
                    }
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
                var value_list = []
                Object.values(parameters[name]).forEach(function (value){
                    if (typeof value == 'string') {
                        value_list.push(value)
                    } else {
                        value_list = value_list.concat(Object.values(value))
                    }
                })
                reason = _run(value_list, name)
                if (reason) {
                    return true
                }
            })

            // 匹配 header 参数
            if (reason == false && context.header != null) {
                Object.keys(context.header).some(function (name) {
                    if ( name.toLowerCase() == "cookie") {
                        var cookies = get_cookies(context.header.cookie)
                        for (name in cookies) {
                            reason = _run([cookies[name]], "cookie:" + name)
                            if (reason) {
                                return true
                            }
                        }
                    }
                    else if ( headerInjection.indexOf(name.toLowerCase()) != -1) {
                        reason = _run([context.header[name]], "header:" + name)
                        if (reason) {
                            return true
                        }
                    }
                    
                })
            }

            // 匹配json参数
            if (reason == false && Object.keys(json_parameters).length > 0) {
                var jsons = [ [json_parameters, "input_json"] ]
                while (jsons.length > 0 && reason === false) {
                    var json_arr = jsons.pop()
                    var crt_json_key = json_arr[1]
                    var json_obj = json_arr[0]
                    for (item in json_obj) {
                        if (typeof json_obj[item] == "string") {
                            reason = _run([json_obj[item]], crt_json_key + "->" + item)
                            if(reason !== false) {
                                break;
                            }
                        }
                        else if (typeof json_obj[item] == "object") {
                            jsons.push([json_obj[item], crt_json_key + "->" + item])
                        }
                    }
                }
            }

            if (reason !== false && !sqliWhiteManager.test(params.stack[0])) {
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
            if ((raw_tokens.length == 0) && 
                (sqliPrefilter2.test(params.query))) {
                raw_tokens = RASP.sql_tokenize(params.query, params.server)
            }

            var features        = algorithmConfig.sql_policy.feature
            var func_list       = algorithmConfig.sql_policy.function_blacklist
            var func_count_list = algorithmConfig.sql_policy.function_count

            // 黑名单函数计数
            var func_count_arr  = {}

            // 转换小写，避免大小写绕过
            var tokens_lc = raw_tokens.map(function(v) {
                return v.text.substr(0, 50).toLowerCase()
            })

            // 是否在union select 语句中
            var union_state = false

            for (var i = 1; i < tokens_lc.length; i ++)
            {
                if (features['union_null']) 
                {
                    if (tokens_lc[i] === 'union')
                    {
                        union_state = true
                    }
                    else if (tokens_lc[i] === 'from')
                    {
                        union_state = false
                    }
                    else if (tokens_lc[i] === 'select' && union_state)
                    {
                        var null_count = 0
                        var num_count = 0

                        // 寻找连续的逗号、NULL或者数字
                        for (var j = i + 1; j < tokens_lc.length && j < i + 6; j ++) {
                            if ((tokens_lc[j] === ',' || tokens_lc[j] == 'null') && tokens_lc[j] != tokens_lc[j+1]) {
                                null_count ++
                            } else {
                                break
                            }
                        }
                        for (var j = i + 1; j < tokens_lc.length && j < i + 6; j ++) {
                            if ((tokens_lc[j] === ',' || ! isNaN(parseInt(tokens_lc[j]))) && tokens_lc[j] != tokens_lc[j+1]) {
                                num_count++
                            } else {
                                break
                            }
                        }

                        // NULL,NULL,NULL == 5个token
                        // 1,2,3          == 5个token
                        if (null_count >= 5 || num_count >= 5) {
                            reason = _("SQLi - Detected UNION-NULL phrase in sql query")
                            break
                        }
                        continue
                    }
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
                    if (func_list[func_name])
                    {
                        reason = _("SQLi - Detected dangerous method call %1%() in sql query", [func_name])
                        break
                    }

                    if (features['function_count'] && func_count_list[func_name])
                    {
                        if (! func_count_arr[func_name])
                        {
                            func_count_arr[func_name] = 1
                        }
                        else
                        {
                            func_count_arr[func_name] ++
                        }

                        // 超过次数拦截
                        if (func_count_arr[func_name] >= func_count_list[func_name]) 
                        {
                            reason = _("SQLi - Detected multiple call to dangerous method %1%() in sql query (%2% times)", [func_name, func_count_arr[func_name]])
                            break
                        }
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
                    var part = tokens_lc[i + 1].replaceAll('`', '', 40)
                    // 正常的antlr和flex返回1个token
                    if (part == 'information_schema.tables')
                    {
                        reason = _("SQLi - Detected access to MySQL information_schema.tables table")
                        break
                    }
                    // flex在1.1.2以前会产生3个token
                    else if (part == 'information_schema' && i < tokens_lc.length - 3)
                    {
                        var part2 = tokens_lc[i + 3].replaceAll('`', '', 10)
                        if (part2 == "tables")
                        {
                            reason = _("SQLi - Detected access to MySQL information_schema.tables table")
                            break
                        }
                    }
                }
            }

            if (reason !== false && !sqliWhiteManager.test(params.stack[0])) 
            {
                return {
                    action:     algorithmConfig.sql_policy.action,
                    message:    reason,
                    confidence: 100,
                    algorithm:  'sql_policy'
                }
            }
        }

        // 算法4: SQL正则表达式
        if (algorithmConfig.sql_regex.action != 'ignore') {
            var regex_filter = new RegExp(algorithmConfig.sql_regex.regex, 'i')
            
            if (regex_filter.test(params.query)) {
                return {
                    action:     algorithmConfig.sql_regex.action,
                    confidence: 60,
                    message:    reason,
                    algorithm:  'sql_regex'
                }
            }
        }

        // 加入缓存，对 prepared sql 特别有效
        return clean
    })

    plugin.register('ssrf', function(params, context) {
        var ret = check_ssrf(params, context, false)
        if (ret !== false) {
            return ret
        }
        return clean
    })

    plugin.register('ssrfRedirect', function(params, context) {
        var params2 = {
            // 使用原始url，用于检测用户输入
            origin_hostname: params.hostname,
            origin_ip: params.ip,

            url: params.url2,
            hostname: params.hostname2,
            ip: params.ip2,
            port: params.port2,
            function: params.function,
            stack: params.stack
        }
        var ret2 = check_ssrf(params2, context, true)
        if (ret2 !== false) {
            ret = check_ssrf(params, context, false)
            if (ret === false) {
                return ret2
            }
        }
        return clean
    })
}

plugin.register('sql_exception', function(params, context) {
    // 为了提高效率，异常代码在 agent 端过滤，插件仅负责过滤掉可能的误报和拼接消息，e.g
    // mysql error 1367 detected: XXX
    var error_code = parseInt(params.error_code)
    var message    = _("%1% error %2% detected: %3%", [params.server, params.error_code, params.error_msg])
    // 过滤phpmyadmin
    if (sqliWhiteManager.test(params.stack[0])) {
        return clean
    }
    if (params.server == "mysql") {
        // 1062 Duplicated key 错误会有大量误报问题，仅当语句里包含 rand 字样报警
        if (error_code == 1062) {
            // 忽略大小写匹配
            if ( !/rand/i.test(params.query)) {
                return clean
            }
        }
        
        else if (error_code == 1064) {
            if ( /in\s*(\(\s*\)|[^\(\w])/i.test(params.query)) {
                return clean
            }
            // 过滤非语法错误
            if (! /syntax/i.test(params.error_msg)) {
                return clean
            }
        }
    }
    else if (params.server == 'sqlite') {
        if (error_code == 1) {
            // 忽略大小写匹配
            if ( !/syntax/i.test(params.error_msg) && !  /malformed MATCH/i.test(params.error_msg)) {
                return clean
            }
        }
    }
    return {
        action:     algorithmConfig.sql_exception.action,
        message:    message,
        confidence: 70,
        algorithm:  'sql_exception'
    }
})

plugin.register('directory', function (params, context) {

    var realpath    = params.realpath
    var server      = context.server

    var is_windows  = server.os.indexOf('Windows') != -1
    var language    = server.language

    // 算法2 - 检查PHP菜刀等后门
    if (algorithmConfig.directory_reflect.action != 'ignore')
    {
        if (language == 'php' && validate_stack_php(params.stack))
        {
            return {
                action:     algorithmConfig.directory_reflect.action,
                message:    _("WebShell activity - Using file manager function with China Chopper WebShell"),
                confidence: 90,
                algorithm:  'directory_reflect'
            }
        }
        else if (language == 'java' && validate_stack_java(params.stack) && !is_method_from_rasp(params.stack))
        {
            return {
                action:     algorithmConfig.directory_reflect.action,
                message:    _("WebShell activity - Using file manager function with Java WebShell"),
                confidence: 90,
                algorithm:  'directory_reflect'
            }
        }
    }

    // 算法1 - 用户输入匹配。
    if (algorithmConfig.directory_userinput.action != 'ignore')
    {
       var all_parameter = get_all_parameter(context)

        if (is_path_containing_userinput(all_parameter, params.path, is_windows, algorithmConfig.directory_userinput.lcs_search))
        {
            return {
                action:     algorithmConfig.directory_userinput.action,
                message:    _("Path traversal - Accessing folder specified by userinput, folder is %1%", [realpath]),
                confidence: 90,
                algorithm:  'directory_userinput'
            }
        }
    }

    // 算法3 - 读取敏感目录
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

    return clean
})


plugin.register('readFile', function (params, context) {
    var server    = context.server
    var is_win    = server.os.indexOf('Windows') != -1

    // weblogic/tongweb 下面，所有war包读取操作全部忽略
    if (server['server'] === 'weblogic' || server['server'] == 'tongweb')
    {
        if (params.realpath.endsWith('.war') || params.realpath.endsWith('.ear'))
        {
            return clean;
        }
    }

    // 获取协议，如果有
    var path_parts = params.path.split('://')
    var proto = ""
    if (path_parts.length > 1) {
        proto = path_parts[0].toLowerCase()
    }

    //
    // 算法1: 简单用户输入识别，拦截任意文件下载漏洞
    //
    // 不影响正常操作，e.g
    // ?path=download/1.jpg
    //
    if (algorithmConfig.readFile_userinput.action != 'ignore')
    {
        var all_parameter = get_all_parameter(context)

        // ?path=/etc/./hosts
        // ?path=../../../etc/passwd
        if ( (proto == "" || proto == "file" ) && 
             !readFileWhiteExt.test(params.realpath) &&
             is_path_endswith_userinput(all_parameter, params.path, params.realpath, is_win, algorithmConfig.readFile_userinput.lcs_search)
           )
        {
            return {
                action:     algorithmConfig.readFile_userinput.action,
                message:    _("Path traversal - Downloading files specified by userinput, file is %1%", [params.realpath]),
                confidence: 90,
                algorithm: 'readFile_userinput'
            }
        }
        // @FIXME: 用户输入匹配了两次，需要提高效率
        if (is_from_userinput(all_parameter, params.path))
        {
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
    if ( (proto == "" || proto == "file" ) && algorithmConfig.readFile_outsideWebroot.action != 'ignore')
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
    var is_win    = server.os.indexOf('Windows') != -1
    var realpath  = params.realpath

    // 用户输入检查
    // ?file=/etc/passwd
    // ?file=../../../../../var/log/httpd/error.log
    if (algorithmConfig.include_userinput.action != 'ignore')
    {
        var all_parameter = get_all_parameter(context)

        if (is_path_endswith_userinput(all_parameter, url, realpath, is_win, algorithmConfig.include_userinput.lcs_search))
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

    // PUT 上传脚本文件 - 有个关联问题需要解决，暂时注释掉
    // if (context.method == 'put' &&
    //     algorithmConfig.writeFile_PUT_script.action != 'ignore')
    // {
    //     if (scriptFileRegex.test(params.realpath))
    //     {
    //         return {
    //             action:     algorithmConfig.writeFile_PUT_script.action,
    //             message:    _("File upload - Using HTTP PUT method to upload a webshell", [params.realpath]),
    //             confidence: 95,
    //             algorithm:  'writeFile_PUT_script'
    //         }
    //     }
    // }

    // 关于这个算法，请参考这个插件定制文档
    // https://rasp.baidu.com/doc/dev/official.html#case-file-write
    if (algorithmConfig.writeFile_script.action != 'ignore')
    {
        var all_parameter = get_all_parameter(context)
        var is_win = context.server.os.indexOf('Windows') != -1
        if (scriptFileRegex.test(params.realpath))
        {
            if (!(algorithmConfig.writeFile_script.userinput) ||
                ((algorithmConfig.writeFile_script.userinput) &&
                (is_path_endswith_userinput(all_parameter, params.path, params.realpath, is_win, algorithmConfig.writeFile_script.lcs_search)))
            ) {
                return {
                    action:     algorithmConfig.writeFile_script.action,
                    message:    _("File write - Creating or appending to a server-side script file, file is %1%", [params.realpath]),
                    confidence: 85,
                    algorithm:  'writeFile_script'
                }
            }
        }
    }

    if (algorithmConfig.writeFile_reflect.action != 'ignore') {
        if (context.server.language == 'java' && 
            (params.realpath.endsWith(".jsp") || params.realpath.endsWith(".jspx"))
        ) {
            var message = validate_stack_java(params.stack)
            if (message) {
                return {
                    action:     algorithmConfig.writeFile_reflect.action,
                    message:    _("Reflect file write - %1%, file is %2%", [message, params.realpath]),
                    confidence: 85,
                    algorithm:  'writeFile_reflect'
                }
            }
        }
    }

    return clean
})

plugin.register('deleteFile', function (params, context) {

    if (algorithmConfig.deleteFile_userinput.action != 'ignore')
    {
        var all_parameter = get_all_parameter(context)
        var is_win = context.server.os.indexOf('Windows') != -1
        if (is_path_endswith_userinput(all_parameter, params.path, params.realpath, is_win, algorithmConfig.deleteFile_userinput.lcs_search)) {
            return {
                action:     algorithmConfig.deleteFile_userinput.action,
                message:    _("File delete - Deleting files specified by userinput, file is %1%", [params.realpath]),
                confidence: 85,
                algorithm:  'deleteFile_userinput'
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
                message:    _("File upload - Uploading a HTML/JS file with multipart/form-data protocol, filename: %1%", [params.filename]),
                confidence: 90,
                algorithm:  'fileUpload_multipart_html'
            }
        }
    }

    // 是否禁止 EXE/DLL 文件，防止被用于后门下载站点
    if (algorithmConfig.fileUpload_multipart_exe.action != 'ignore')
    {
        if (exeFileRegex.test(params.filename))
        {
            return {
                action:     algorithmConfig.fileUpload_multipart_exe.action,
                message:    _("File upload - Uploading a Executable file with multipart/form-data protocol, filename: %1%", [params.filename]),
                confidence: 90,
                algorithm:  'fileUpload_multipart_exe'
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


if (algorithmConfig.link_webshell.action != 'ignore')
{
    plugin.register('link', function (params, context) {
        // 目标文件在webroot内才认为是写后门
        if (!is_outside_webroot(context.appBasePath, params.dest, null)) {
            // 源文件是干净的文件，目标文件是脚本文件，判定为重命名方式写后门
            if (cleanFileRegex.test(params.source) && scriptFileRegex.test(params.dest))
            {
                return {
                    action:    algorithmConfig.link_webshell.action,
                    message:   _("File upload - Linking a non-script file to server-side script file, source file is %1%, link type", [
                        params.source,
                        params.type
                    ]),
                    confidence: 90,
                    algorithm:  'link_webshell'
                }
            }
        }

        return clean
    })
}

plugin.register('command', function (params, context) {
    var cmd        = params.command
    var server     = context.server
    var message    = undefined
    var raw_tokens = []


    // 算法1: 根据堆栈，检查是否为反序列化攻击。
    // 理论上，此算法不存在误报

    if (algorithmConfig.command_reflect.action != 'ignore') {
        // Java 检测逻辑
        if (server.language == 'java') {
            message = validate_stack_java(params.stack)
            if (message) {
                message = _("Reflected command execution - %1%", [message])
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
        var reason     = false
        var min_length = algorithmConfig.command_userinput.min_length
        var parameters = context.parameter || {}
        var json_parameters = context.json || {}
        var unexploitable_filter = algorithmConfig.command_userinput.java_unexploitable_filter

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
                    reason = _("Command injection - command structure altered by user input, request parameter name: %1%, value: %2%", [name, value])
                    return true
                }
            })

            return reason
        }

        // 过滤java无法利用的命令注入
        if (server.language != 'java' || !unexploitable_filter || cmdJavaExploitable.test(cmd)) {
            // 匹配 GET/POST/multipart 参数
            Object.keys(parameters).some(function (name) {
                // 覆盖场景，后者仅PHP支持
                // ?id=XXXX
                // ?data[key1][key2]=XXX
                var value_list = []
                Object.values(parameters[name]).forEach(function (value){
                    if (typeof value == 'string') {
                        value_list.push(value)
                    } else {
                        value_list = value_list.concat(Object.values(value))
                    }
                })
                reason = _run(value_list, name)
                if (reason) {
                    return true
                }
            })
            // 匹配 header 参数
            if (reason == false && context.header != null) {
                Object.keys(context.header).some(function (name) {
                    if ( name.toLowerCase() == "cookie") {
                        var cookies = get_cookies(context.header.cookie)
                        for (name in cookies) {
                            reason = _run([cookies[name]], "cookie:" + name)
                            if (reason) {
                                return true
                            }
                        }
                    }
                    else if ( headerInjection.indexOf(name.toLowerCase()) != -1) {
                        reason = _run([context.header[name]], "header:" + name)
                        if (reason) {
                            return true
                        }
                    }
                    
                })
            }

            // 匹配json参数
            if (reason == false && Object.keys(json_parameters).length > 0) {
                var jsons = [ [json_parameters, "input_json"] ]
                while (jsons.length > 0 && reason === false) {
                    var json_arr = jsons.pop()
                    var crt_json_key = json_arr[1]
                    var json_obj = json_arr[0]
                    for (item in json_obj) {
                        if (typeof json_obj[item] == "string") {
                            reason = _run([json_obj[item]], crt_json_key + "->" + item)
                            if(reason !== false) {
                                break;
                            }
                        }
                        else if (typeof json_obj[item] == "object") {
                            jsons.push([json_obj[item], crt_json_key + "->" + item])
                        }
                    }
                }
            }
        }

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
                message:    _("Webshell detected - Executing potentially dangerous command, command is %1%", [params.command]),
                confidence: 95,
                algorithm:  'command_common'
            }
        }     
    }

    // 算法4: 查找语法错误和敏感操作
    if (algorithmConfig.command_error.action != 'ignore') {
        if (raw_tokens.length == 0) {
            raw_tokens = RASP.cmd_tokenize(cmd)
        }
        var concat_char = algorithmConfig.command_error.concat_char
        var sensitive_cmd = algorithmConfig.command_error.sensitive_cmd
        var alarm_token = algorithmConfig.command_error.alarm_token

        var double_quote = 0
        var ticks = 0
        for (var i=0; i<raw_tokens.length; i++) {
            // 敏感token检测
            if (algorithmConfig.command_error.alarm_token_enable) {
                if (alarm_token.indexOf(raw_tokens[i].text) != -1) {
                    if ( !(i > 0 && i < raw_tokens.length-1 && raw_tokens[i-1].text == '"' && raw_tokens[i+1].text == '"')) {
                        return {
                            action:     algorithmConfig.command_error.action,
                            confidence: 90,
                            message:    _("Command execution - Sensitive command token detect: %1%", [raw_tokens[i].text]),
                            algorithm:  'command_error'
                        }
                    }
                }
            }

            // 敏感连接命令检测
            if (algorithmConfig.command_error.sensitive_cmd_enable) {
                if (raw_tokens[i+1] !== undefined &&
                    concat_char.indexOf(raw_tokens[i].text) != -1 &&
                    sensitive_cmd.indexOf(raw_tokens[i+1].text) != -1) {
                    return {
                        action:     algorithmConfig.command_error.action,
                        confidence: 70,
                        message:    _("Command execution - Sensitive command concat detect: %1% %2%", [raw_tokens[i].text, raw_tokens[i+1].text]),
                        algorithm:  'command_error'
                    }
                }
            }

            if (raw_tokens[i].text == "\"") {
                double_quote ++
            }
            else if (raw_tokens[i].text == "`") {
                ticks ++
            }
            else if (raw_tokens[i].text == "'" && algorithmConfig.command_error.unbalanced_quote_enable) {
                if ( !(i > 0 && i < raw_tokens.length-1 && raw_tokens[i-1].text == '"' && raw_tokens[i+1].text == '"')) {
                    return {
                        action:     algorithmConfig.command_error.action,
                        confidence: 70,
                        message:    _("Command execution - Detected unbalanced single quote!"),
                        algorithm:  'command_error'
                    }
                }
            }
        }

        // 引号不匹配检测
        if (algorithmConfig.command_error.unbalanced_quote_enable) {
            if (double_quote % 2 != 0) {
                return {
                    action:     algorithmConfig.command_error.action,
                    confidence: 70,
                    message:    _("Command execution - Detected unbalanced double quote!"),
                    algorithm:  'command_error'
                }
            }
            if (ticks % 2 != 0) {
                return {
                    action:     algorithmConfig.command_error.action,
                    confidence: 70,
                    message:    _("Command execution - Detected unbalanced backtick!"),
                    algorithm:  'command_error'
                }
            }
        }
    }

    // 算法5: 记录所有的命令执行
    if (algorithmConfig.command_other.action != 'ignore') 
    {
        return {
            action:     algorithmConfig.command_other.action,
            message:    _("Command execution - Logging all command execution by default, command is %1%", [params.command]),
            confidence: 90,
            algorithm:  'command_other'
        }
    }

    // 算法6: DNSlog检测
    if (algorithmConfig.command_dnslog.action != 'ignore') 
    {
        if (cmdDNSlogPatternCmd.test(params.command))
        {
            if (cmdDNSlogPatternDomain.test(params.command)) {
                return {
                    action:     algorithmConfig.command_dnslog.action,
                    message:    _("Command injection - Executing dnslog command, command is %1%", [params.command]),
                    confidence: 95,
                    algorithm:  'command_dnslog'
                }
            }
        }
    }

    return clean
})


// 注意: 由于libxml2无法挂钩，所以PHP暂时不支持XXE检测
plugin.register('xxe', function (params, context) {
    var server      = context.server
    var is_win      = server.os.indexOf('Windows') != -1
    var items       = params.entity.split(':')
    var parameters  = context.parameter || {}
    var header      = context.header || {}

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
        var protocol = items.shift().toLowerCase()
        var address  = items.join(":")

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
        // file:///etc/passwd?a=1#b=2 (仅Java支持)
        //
        // 相对路径容易误报, e.g
        // file://xwork.dtd
        if (algorithmConfig.xxe_file.action != 'ignore')
        {
            if (address.length > 0 && protocol === 'file')
            {
                if (address.startsWith("//")) {
                    // 去掉file://中的//，两种格式统一逻辑处理
                    // file:/etc/passwd
                    // file:///etc/passwd
                    address = address.substr(2)
                }
                var address_lc = address.toLowerCase()

                if (address_lc.indexOf("../") != -1) {
                    // 使用 ../ 
                    return {
                        action:     algorithmConfig.xxe_file.action,
                        message:    _("XXE - Accessing file %1% with ../", [address]),
                        confidence: 90,
                        algorithm:  'xxe_file'
                    }
                }

                if (address_lc.indexOf("#") !=-1 || address_lc.indexOf("?") !=-1) {
                    return {
                        action:     algorithmConfig.xxe_file.action,
                        message:    _("XXE - Using url comment in file path %1%", [address]),
                        confidence: 90,
                        algorithm:  'xxe_file'
                    }
                }

                if (is_absolute_path(address, is_win) || 
                    address_lc.startsWith("localhost") ||
                    (is_win && items.length > 2)) {
                    // 三种情况：
                    // 一般绝对路径 file:/etc/passwd
                    // localhost起始路径 file://localhost/c:/windows/win.ini
                    // 带盘符的windows绝对路径 file:c:/windows/win.ini
                    // 1.0 Rhino 引擎不支持URL对象，考虑到 1.0 用户不多，先简单处理下
                    var content_type = header["content-type"] || ""
                    if (content_type.indexOf("xml") != -1 || is_include_in_userinput(parameters, address)) {
                        // 过滤掉 xml、dtd、xsd
                        if (! address_lc.endsWith('.xml') &&
                            ! address_lc.endsWith('.xsd') &&
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
        }
    }
    return clean
})


if (algorithmConfig.eval_regex.action != 'ignore')
{
	// 算法1: 正则表达式
    plugin.register('eval', function(params, context) {
        var regex_filter = new RegExp(algorithmConfig.eval_regex.regex, 'i')
            
        if (regex_filter.test(params.code)) {

        	// 避免 message 太长
        	var code = params.code.substr(0, 50)
        	if (params.code.length > 50)
        	{
        		code = code + ' ...'
        	}

            return {
                action:     algorithmConfig.eval_regex.action,
                confidence: 60,
                message:    _("Code Execution - Running %1% with %2%() function", [code, params.function]),
                algorithm:  'eval_regex'
            }
        }
    })
}

plugin.register('loadLibrary', function(params, context) {

    if (algorithmConfig.loadLibrary_unc.action != 'ignore') {

        // 仅 windows 需要检查 UNC
        var is_windows = context.server.os.indexOf('Windows') != -1
        if (is_windows) {
            if (params.path.startsWith('\\\\') || params.path.startsWith('//')) {
                return {
                    action:     algorithmConfig.loadLibrary_unc.action,
                    confidence: 60,
                    message:    _("Load library in UNC path - loading %1% with %2%() function", [params.path, params.function]),
                    algorithm:  'loadLibrary_unc'
                }
            }    
        }
        
    }

    // if (algorithmConfig.loadLibrary_other.action != 'ignore') {
    //     return {
    //         action:     algorithmConfig.loadLibrary_other.action,
    //         confidence: 60,
    //         message:    _("Load library - logging all by default, library path is %1%", [params.path]),
    //         algorithm:  'loadLibrary_other'
    //     }     
    // }

    return clean
})

// 默认情况下，当OGNL表达式长度超过30才会进入检测点，此长度可配置
plugin.register('ognl', function (params, context) {

    // 算法1: OGNL语句黑名单
    if (algorithmConfig.ognl_blacklist.action != 'ignore')
    {
        var ognlExpression = params.expression
        for (var index in algorithmConfig.ognl_blacklist.expression)
        {
            if (ognlExpression.indexOf(algorithmConfig.ognl_blacklist.expression[index]) > -1)
            {
                return {
                    action:     algorithmConfig.ognl_blacklist.action,
                    message:    _("OGNL exec - Trying to exploit a OGNL expression vulnerability"),
                    confidence: 100,
                    algorithm:  'ognl_blacklist'
                }
            }

        }
    }

    // 算法2: OGNL表达式长度限制
    if (algorithmConfig.ognl_length_limit.action != 'ignore')
    {
        var ognlLength = params.expression.length
        if (ognlLength > algorithmConfig.ognl_length_limit.max_length)
        {
            return {
                action:     algorithmConfig.ognl_length_limit.action,
                message:    _("OGNL exec - Trying to execute a OGNL expression of unusual length: " + ognlLength),
                confidence: 100,
                algorithm:  'ognl_length_limit'
            }
        }
    }

    return clean
})

if (algorithmConfig.jndi_disable_all.action != 'ignore') 
{
    plugin.register('jndi', function (params, context) {
        let name = params.name
        return {
            action:     algorithmConfig.jndi_disable_all.action,
            message:    _("JNDI blacklist - blocked jndi lookup: " + name),
            confidence: 100,
            algorithm:  'jndi_disable_all'
        }
    })
}

if (algorithmConfig.dns_blacklist.action != 'ignore') 
{
    plugin.register('dns', function (params, context) {
        let host = params.host
        if (is_hostname_dnslog(host)) {
            return {
                action:     algorithmConfig.dns_blacklist.action,
                message:    _("DNS blacklist - blocked dnslog domain: " + host),
                confidence: 100,
                algorithm:  'dns_blacklist'
            }
        }
    })
}


if (algorithmConfig.deserialization_blacklist.action != 'ignore') 
{
    plugin.register('deserialization', function (params, context) {
        var clazz = params.clazz
        for (var index in algorithmConfig.deserialization_blacklist.clazz) {
            if (clazz === algorithmConfig.deserialization_blacklist.clazz[index]) {
                return {
                    action:     algorithmConfig.deserialization_blacklist.action,
                    message:    _("Deserialization blacklist - blocked " + clazz + " in resolveClass"),
                    confidence: 100,
                    algorithm:  'deserialization_blacklist'
                }
            }
        }
        return clean
    })
}


// 匹配身份证
function findFirstIdentityCard(data) {
    const regexChineseId = /(?<!\d)\d{10}(?:[01]\d)(?:[0123]\d)\d{3}(?:\d|x|X)(?!\d)/;
    const W = [7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2];
    const m = regexChineseId.exec(data)
    if (m) {
        const id = m[0]

        // FIXME: 简单处理 springboot 接口误报问题
        if (id[0] == 0) {
            return
        }

        let sum = 0;
        for (let i = 0; i < W.length; i++) {
            sum += (id[i] - '0') * W[i];
        }
        if (id[17] == 'X' || id[17] == 'x') {
            sum += 10;
        } else {
            sum += id[17] - '0';
        }
        if (sum % 11 == 1) {
            return {
                type:  'Identity Card',
                match: m[0],
                parts: data.slice(Math.max(m.index - 40, 0), m.index + m[0].length + 40)
            }
        }
    }
}

// 匹配手机号
function findFirstMobileNumber(data) {
    const regexChinesePhone = /(?<!\w)(?:(?:00|\+)?86 ?)?(1\d{2})(?:[ -]?\d){8}(?!\w)/;
    const prefixs = new Set([133, 149, 153, 173, 174, 177, 180,
        181, 189, 199, 130, 131, 132, 145, 146, 155, 156, 166, 175, 176, 185, 186, 134, 135, 136, 137, 138, 139,
        147, 148, 150, 151, 152, 157, 158, 159, 165, 178, 182, 183, 184, 187, 188, 198, 170
    ]);
    let m = regexChinesePhone.exec(data)
    if (m) {
        if (prefixs.has(parseInt(m[1]))) {
            return {
                type:  'Mobile Number',
                match: m[0],
                parts: data.slice(Math.max(m.index - 40, 0), m.index + m[0].length + 40)
            }
        }
    }
}

// 匹配银行卡、信用卡
function findFirstBankCard(data) {
    const regexBankCard = /(?<!\d)(?:62|3|5[1-5]|4\d)\d{2}(?:[ -]?\d{4}){3}(?!\d)/;
    let m = regexBankCard.exec(data)
    if (m) {
        let card = m[0].replace(/ |-/g, "");
        let len = card.length;
        let sum = 0;
        for (let i = len; i >= 1; i--) {
            let t = card[len - i] - '0';
            if (i % 2 == 0) {
                t *= 2;
            }
            sum = sum + Math.floor(t / 10) + t % 10;
        }
        if (sum % 10 == 0) {
            return {
                type:  'Bank Card',
                match: m[0],
                parts: data.slice(Math.max(m.index - 40, 0), m.index + m[0].length + 40)
            }
        }
    }
}

if (algorithmConfig.response_dataLeak.action != 'ignore') {

    // response 所有检测点都会抽样
    plugin.register('response', function (params, context) {
        const content_type = params.content_type
        const content      = params.content
        const kind         = algorithmConfig.response_dataLeak.kind
        const header       = context.header || {}

        var items = [], parts = []

        // content-type 过滤
        if ( ! content_type || ! dataLeakContentType.test(content_type)) {
            return clean
        }

        // 是否检查身份证泄露
        if (kind.identity_card) {
            const data = findFirstIdentityCard(content)
            if (data) {
                items.push(data.match + '(' + data.type + ')')
                parts.push(data)
            }
        }

        // 是否检查手机号泄露
        if (kind.phone) {
            const data = findFirstMobileNumber(content)
            if (data) {
                items.push(data.match + '(' + data.type + ')')
                parts.push(data)
            }
        }

        // 是否检查银行卡泄露
        if (kind.bank_card) {
            const data = findFirstBankCard(content)
            if (data) {
                items.push(data.match + '(' + data.type + ')')
                parts.push(data)
            }
        }

        if (items.length) {
            return {
                action:     algorithmConfig.response_dataLeak.action,
                message:    'PII leak detected: ' + items.join('、 '),
                confidence: 80,
                algorithm:  'response_dataLeak',
                params: {
                    parts
                }
            }
        }
    })
}

plugin.log('OpenRASP official plugin: Initialized, version', plugin_version)

