const plugin_version = '2018-1000-1000'
const plugin_name    = '001-xss-demo'

// Java 版本需要设置 request.param_encoding 之后才能使用
// PHP  版本不支持 request hook 点，所以没有这个检测
//  
//  https://rasp.baidu.com/doc/setup/others.html

var plugin = new RASP(plugin_name)
var clean  = {
    action: 'ignore',
    message: '无风险',
    confidence: 0
}

// BEGIN ALGORITHM CONFIG //

var algorithmConfig = {}

// END ALGORITHM CONFIG //

plugin.register('request', function(params, context) {

    // XSS 检测 DEMO
    // 在 request hook 点简单匹配用户输入参数

    function detectXSS(params, context) {
        var xssRegex   = /<script|script>|<iframe|iframe>|javascript:(?!(?:history\.(?:go|back)|void\(0\)))/i
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

        return message
    }

    // XSS 检测 DEMO //
    var message = detectXSS(params, context)
    if (message.length) {
        return {
            action:     'block',
            message:    message,
            confidence: 90
        }
    }

    return clean    
})

plugin.log('001-xss-demo 加载完成')

