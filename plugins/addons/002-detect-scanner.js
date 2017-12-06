var plugin = new RASP('offical')
var clean  = {
  action: 'ignore',
  message: '无风险',
  confidence: 0
}

plugin.register('request', function(params, context) {
  // 已知的扫描器识别
  var foundScanner = false
  var scannerUA    = [
    "attack", "scan", "vulnerability", "injection", "xss",
    "exploit", "grabber", "cgichk", "bsqlbf", "sqlmap", 
    "nessus", "arachni", "metis", "sql power injector", 
    "bilbo", "absinthe", "black widow", "n-stealth", "brutus", 
    "webtrends security analyzer", "netsparker", "jaascois", "pmafind", 
    ".nasl", "nsauditor", "paros", "dirbuster", "pangolin", "nmap nse", 
    "sqlninja", "nikto", "webinspect", "blackwidow", "grendel-scan", 
    "havij", "w3af", "hydra"]
  var headers      = context.header

  if (headers['acunetix-product'] || headers['x-wipp']) {
    foundScanner = true
  } else {
    var ua = headers['user-agent']
    if (ua) {
      for (var i = 0; i < scannerUA.length; i++) {
        if (ua.indexOf(scannerUA[i].toLowerCase()) != -1) {
          foundScanner = true
          break
        }
      }
    }
  }
  // 扫描器识别 DEMO //    
  if (foundScanner) {
    return {
      action: 'block',
      message: '已知的扫描器探测行为: ' + scannerUA[i],
      confidence: 90
    }
  }
  return clean
})

plugin.log('002-detect-scanner 加载完成')

