// SQLi 检测算法已经移动到Java层面
// 这个插件默认已经不需要了

'use strict'
var plugin  = new RASP('offical')

const clean = {
    action:     'ignore',
    message:    '无风险',
    confidence: 0
}

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
            'load_file': true,
            'benchmark': true,
            'sleep':     true,
            'pg_sleep':  true
        }
        var tokens_lc = tokens.map(v => v.toLowerCase())

        for (var i = 0; i < tokens_lc.length; i ++) {
            if (tokens_lc[i] == ';') {
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
                        
                var op1    = tokens_lc[i - 1]
                var op2    = tokens_lc[i + 1]

                if (Number.isInteger(op1) && Number.isInteger(op2)) {
                    reason = '禁止常量比较操作'
                    break
                }                    
            } else if (i > 0 && tokens_lc[i].indexOf('(') == 0) {
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
                message:    '数据库语句异常: ' + reason + '（算法3）',
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