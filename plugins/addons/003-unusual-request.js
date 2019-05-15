const plugin_version = '2018-1000-1000'
const plugin_name    = '003-unusual-request'

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
  var header = context.header
  var method = context.method
  var reason = false

  if (! header['accept']) {
    reason = '缺少 Accept 请求头'
  } else if (method != 'get' && method != 'post' && method != 'head') {
    reason = method.toUpperCase()
  }

  if (reason) {
    return {
      action:     'block',
      message:    '不常见的请求方式: ' + reason,
      confidence: 90
    }
  }

  return clean
})

plugin.log('003-unusual-request 加载完成')

