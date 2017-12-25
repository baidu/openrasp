// SSRF 检测算法已经移动到Java层面
// 这个插件默认已经不需要了

'use strict'
var plugin  = new RASP('offical')

const clean = {
    action:     'ignore',
    message:    '无风险',
    confidence: 0
}

plugin.register('ssrf', function (params, context) {
    var hostname = params.hostname
    var reason   = false

    // 检查常见探测域名
    if (hostname.endsWith('.xip.io') || hostname.endsWith('.burpcollaborator.net')) {
        reason = '访问已知的内网探测域名'    
    } 
    // 检测AWS私有地址，如有误报可注释掉
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