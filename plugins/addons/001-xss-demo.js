var plugin = new RASP('offical')
var clean  = {
    action: 'ignore',
    message: '无风险',
    confidence: 0
}

// [[ 近期调整~ ]]
plugin.register('request', function(params, context) {

    // XSS 检测 DEMO
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
            action: 'block',
            message: message,
            confidence: 90
        }
    }

    return clean    
})

plugin.log('001-xss-demo 加载完成')

