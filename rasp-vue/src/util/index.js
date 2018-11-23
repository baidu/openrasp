import axios from 'axios'

export var attack_types = {
  'sql':                        'SQL 注入',
  'command':                    '命令执行',
  'xxe':                        'XXE 外部实体加载',
  'directory':                  '目录遍历',
  'rename':                     '文件重命名',
  'readFile':                   '任意文件下载',
  'include':                    '任意文件包含',
  'writeFile':                  '任意文件写入',
  'ssrf':                       'SSRF 服务端请求伪造',
  'ognl':                       'OGNL 代码执行',
  'webdav':                     '任意文件上传 (PUT)',
  'fileUpload':                 '任意文件上传',
  'deserialization':            'Transformer 反序列化',
  'callable':                   'WebShell - 变形后门',
  'webshell_eval':              'WebShell - 中国菜刀',
  'webshell_command':           'WebShell - 命令执行',
  'webshell_file_put_contents': 'WebShell - 后门上传'
}

export var status_types = {
  'block':  '拦截请求',
  'log':    '记录日志',
  'ignore': '忽略放行'
}

export function block_status2name(status) {
  if (status_types[status]) {
    return status_types[status]
  }

  return status
}

export function attack_type2name(id) {
  if (attack_types[id]) {
    return attack_types[id]
  }

  return id
}

export function api_request(url, data, cb, err_cb) {
  var prefix = "/"

  // 本地开发
  if (location.host == '127.0.0.1:8080') {
    prefix = "http://scloud.baidu.com:8090/"

    axios.defaults.headers['X-OpenRASP-Token'] = '9256a3555fbd4f24f7a2ba915a32261ab4c720fc'
  }

  axios
    .post(prefix + url, data)
    .then(function(response) {
      if (response.status != 200) {
        alert("HTTP 请求出错: 响应码 " + response.status)
      } else if (response.data.status != 0) {
        if (err_cb) {
          err_cb(response.data.status, response.data.description)
        } else {
          alert("API 接口出错: " + response.data.status + " - " + response.data.description)
        }
      } else {
        console.log (url, response.data.data)
        cb(response.data.data)
      }
    })
    .catch(function(error) {
      console.log("axios 错误: ", url, error)
    })
}

