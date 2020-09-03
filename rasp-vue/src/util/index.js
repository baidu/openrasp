import axios from 'axios'
import Cookie from 'js-cookie'
import router from '@/router'

export var rasp_version = '1.3.5'

// 起始 type_id: 1001
export var audit_types = {
  1002: 'Agent 注册',
  1003: 'Agent 删除',
  1004: '重置 AppSecret',
  1005: '下发通用配置',
  1006: '下发白名单配置',
  1007: '下发算法配置',
  1008: '下发报警配置',
  1009: '下发检测插件',
  1010: '上传插件',
  1011: '删除插件',
  1012: '创建应用',
  1013: '删除应用',
  1014: '更新应用信息',
  1015: '重置插件配置'
}

export var browser_headers = [{
  name: 'X-Frame-Options',
  descr: '点击劫持防护',
  options: [{
    name: '不开启',
    value: undefined
  },
  {
    name: '拒绝 (deny)',
    value: 'deny'
  },
  {
    name: '只允许同源 (sameorigin)',
    value: 'sameorigin'
  }
  ]
},
{
  name: 'X-Content-Type-Options',
  descr: 'MIME 嗅探防护',
  options: [{
    name: '不开启',
    value: undefined
  },
  {
    name: '开启',
    value: 'nosniff'
  }
  ]
},
{
  name: 'X-XSS-Protection',
  descr: 'XSS Auditor 防护',
  options: [{
    name: '不开启',
    value: undefined
  },
  {
    name: '拦截模式',
    value: '1; mode=block'
  }
  ]
},
// {
//   name: "X-Referrer-Policy",
//   descr: "Referrer 保护",
//   options: [
//     "no-referrer",
//     "no-referrer-when-downgrade",
//     "same-origin",
//     "origin",
//     "strict-origin",
//     "origin-when-cross-origin",
//     "strict-origin-when-cross-origin",
//     "unsafe-url"
//   ]
// },
{
  name: 'X-Download-Options',
  descr: '文件下载防护',
  options: [{
    name: '不开启',
    value: undefined
  },
  {
    name: '关闭自动运行 (noopen)',
    value: 'noopen'
  }
  ]
}
]

export var baseline_types = {
  3001: 'Cookie httpOnly 检查',
  3002: '进程启动账号检查',
  3003: '后台弱口令检查',
  3004: '不安全的默认应用检查',
  3005: '开放目录检查',
  3006: '数据库连接账号审计',
  3007: 'JBoss 后台无认证检查',
  // 3008: '日志泄露敏感信息',
  3009: 'Web 根目录敏感文件泄露',

  4001: 'allow_url_include 配置审计',
  4002: 'expose_php 配置审计',
  4003: 'display_errors 配置审计',
  4004: 'yaml.decode_php 配置审计'
}

export var attack_types = {
  sql: 'SQL 注入',
  sql_exception: 'SQL 语句异常',
  eval: 'EVAL 代码执行',
  loadLibrary: '类库加载',
  command: '命令执行',
  xxe: 'XXE 外部实体加载',
  directory: '目录遍历',
  rename: '文件重命名',
  readFile: '任意文件读取',
  deleteFile: '任意文件删除',
  include: '任意文件包含',
  writeFile: '任意文件写入',
  ssrf: 'SSRF 请求伪造',
  ssrfRedirect: 'SSRF 请求伪造(重定向后)',
  ognl: 'OGNL 代码执行',
  webdav: '任意文件上传 (PUT)',
  fileUpload: '任意文件上传',
  deserialization: 'Transformer 反序列化',
  xss_echo: 'Echo XSS 跨站脚本攻击',
  xss_userinput: 'BODY XSS 跨站脚本攻击',
  webshell_callable: 'WebShell - 变形后门',
  webshell_eval: 'WebShell - 中国菜刀',
  webshell_command: 'WebShell - 命令执行',
  webshell_file_put_contents: 'WebShell - 后门上传',
  webshell_ld_preload: 'WebShell - LD_PRELOAD 后门',
  response: 'HTTP 响应采样检测',
  request: '请求开始',
  link: '文件链接'
}

export var status_types = {
  block: '拦截请求',
  log: '记录日志'
  // ignore: '忽略放行'
}

// 去除空格、换行，按照指定的分隔符分隔，最后删掉 null/undefined/空字符串
export function trimSplit(data, sep) {
  var tmp = data.replace(/\s/g, '').split(sep)
  return tmp.filter(a => a)
}

export function convertToInt(data) {
  var tmp = []
  data.forEach (function (row) {
    tmp.push(parseInt(row))
  })

  return tmp.filter(a => a)
}

export function getDefaultConfig() {
  return {
    general_config: {
      'inject.custom_headers': {}
    },
    whitelist_config: [],
    email_alarm_conf: {
      recv_addr: []
    },
    ding_alarm_conf: {
      recv_user: [],
      recv_party: []
    },
    http_alarm_conf: {
      recv_addr: []
    },
    kafka_alarm_conf: {
      
    },
    general_alarm_conf: {

    }
  }
}

export function validateRegex(value) {
  var error = false
  try {
    new RegExp(value)
  } catch (e) {
    error = '正则表达式错误:' + e.toString()
  }

  return error
}

export function block_status2name(status) {
  return status_types[status] || status
}

export function attack_type2name(id) {
  if (id == 'webshell') {
    return 'WebShell 网站后门'
  }

  if (id == 'xss') {
    return 'XSS 跨站脚本攻击'
  }

  return attack_types[id] || id
}

export const request = axios.create({
  baseURL:
    process.env.NODE_ENV === 'production'
      ? '/'
      : 'http://10.14.117.55:8080/',
  timeout: 8000
})
request.interceptors.request.use(
  config => {
    if (process.env.NODE_ENV !== 'production') {
      config.headers['X-OpenRASP-Token'] =
        '9256a3555fbd4f24f7a2ba915a32261ab4c720fc'
    }
    return config
  },
  error => {
    console.error(error)
    Promise.reject(error)
  }
)
request.interceptors.response.use(
  response => {
    const res = response.data
    if (res.status !== 0) {
      if (res.status === 401) {
        Cookie.set('RASP_AUTH_ID', null)
        if (router.currentRoute.name !== 'login') {
          router.push({
            name: 'login',
            query: {
              redirect: location.href
            }
          })
        }
      } else {
        alert(response.config.url + ' 接口出错: ' + res.status + ' - ' + res.description)
      }
      return Promise.reject(res)
    } else {
      return res.data
    }
  },
  error => {
    alert('HTTP 请求出错: 响应码 ' + error.response.status)
    console.error(error)
    return Promise.reject(error)
  }
)
