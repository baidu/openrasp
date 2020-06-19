<template>
  <div>
    <!-- begin general settings -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          通用设置
        </h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="form-label">
            真实 IP header
            <a target="_blank" href="https://rasp.baidu.com/doc/setup/panel.html#reverse-proxy">
              [帮助文档]
            </a>
          </label>
          <input v-model.trim="data['clientip.header']" type="text" class="form-control" maxlength="100">
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义拦截状态码
          </label>
          <b-form-select v-model.number="data['block.status_code']" :options="[200, 302, 403, 404, 500]" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义拦截跳转页面 [仅自定义拦截状态码为302生效]
          </label>
          <input v-model.trim="data['block.redirect_url']" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义HTML响应内容
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-block" target="_blank">
              [帮助文档]
            </a>
          </label>
          <textarea v-model.trim="data['block.content_html']" type="text" class="form-control" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义XML响应内容
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-block" target="_blank">
              [帮助文档]
            </a>
          </label>
          <textarea v-model.trim="data['block.content_xml']" type="text" class="form-control" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义JSON响应内容
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-block" target="_blank">
              [帮助文档]
            </a>
          </label>
          <textarea v-model.trim="data['block.content_json']" type="text" class="form-control" />
        </div>
        <div class="form-group">
          <label class="form-label">
            最多读取 body 多少字节
          </label>
          <input v-model.number="data['body.maxbytes']" type="number" min="0" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            设置请求编码
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-others" target="_blank">
              [帮助文档]
            </a>
          </label>
          <input type="text" v-model="data['request.param_encoding']" class="form-control" placeholder="">
        </div>
        <div class="form-group">
          <label class="form-label">
            调试开关 [0表示关闭，1以上的值表示开启]
          </label>
          <b-form-select v-model.number="data['debug.level']" :options="[0, 1]" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自动清理离线超过N天的主机 [0表示关闭，1以上的值表示开启；每天零点执行清理任务]
          </label>
          <input v-model.number="data['offline_hosts.cleanup.interval']" type="number" min="0" class="form-control" placeholder="0">
        </div>
        <div class="form-group">
          <label class="form-label">
            LRU 大小 [默认1000，若要关闭写 0]
          </label>
          <input v-model.number="data['lru.max_size']" type="number" min="0" class="form-control" placeholder="1000">
        </div>
        <div class="form-group">
          <label class="form-label">
            LRU 原始字符串上限 [若不开启只存储哈希；若开启同时存储原始字符串，若字符串超过上限直接进入插件检测；范围 1-102400]
          </label>
          <input v-model.number="data['lru.compare_limit']" type="number" min="0" class="form-control" placeholder="10240">
        </div>
        <div class="form-group">
          <label class="form-label">
            [基线] 弱口令列表，适用于数据库、Tomcat管理后台等等；上限200个，单个最长16，逗号分隔
          </label>
          <textarea v-model.trim="weak_password_list" type="text" class="form-control" placeholder="111111,123,123123,123456,123456a,a123456,admin,both,manager,mysql,root,rootweblogic,tomcat,user,weblogic1,weblogic123,welcome1" />
        </div>
        <div class="form-group">
          <label class="form-label">
            [基线] [Web根目录敏感文件检查] 文件名正则，仅PHP支持，最长100字符
          </label>
          <textarea v-model.trim="data['fileleak_scan.name']" type="text" class="form-control" maxlen="100" placeholder="\.(git|svn|tar|gz|rar|zip|sql|log)$" />
        </div>
        <div class="form-group">
          <label class="form-label">
            [基线] [Web根目录敏感文件检查] 扫描间隔（秒），范围 60-86400
          </label>
          <input v-model.number="data['fileleak_scan.interval']" type="number" class="form-control" placeholder="21600" min="60" />
        </div>
        <div class="form-group">
          <label class="form-label">
            [基线] [Web根目录敏感文件检查] 最多检测多少个文件，写 0 关闭这个功能
          </label>
          <input v-model.number="data['fileleak_scan.limit']" type="number" class="form-control" placeholder="100" min="0" />
        </div>
        <div class="form-group">
          <label class="form-label">
            [插件] 单个hook点最大执行时间（ms）
          </label>
          <input v-model.number="data['plugin.timeout.millis']" min="0" type="number" class="form-control" placeholder="100">
        </div>
        <div class="form-group">
          <label class="form-label">
            [插件] 传给插件的最大堆栈深度
          </label>
          <input v-model.number="data['plugin.maxstack']" type="number" min="0" class="form-control" placeholder="100">
        </div>
        <div class="form-group">
          <label class="form-label">
            [日志] 每个进程/线程每秒钟最大日志条数
          </label>
          <input v-model.number="data['log.maxburst']" type="number" min="0" class="form-control" placeholder="100">
        </div>
        <div class="form-group">
          <label class="form-label">
            [日志] 最大备份天数
          </label>
          <input v-model.number="data['log.maxbackup']" type="number" min="0" class="form-control" placeholder="30">
        </div>

        <div class="form-group">
          <label class="form-label">
            [熔断] 单核CPU占用率采集间隔（秒），范围 1-1800
          </label>
          <input v-model.number="data['cpu.usage.interval']" type="number" min="1" max="1800" class="form-control" placeholder="5">
        </div>
        <div class="form-group">
          <label class="form-label">
            [熔断] 单核CPU占用率阈值（百分比），范围 30-100
          </label>
          <input v-model.number="data['cpu.usage.percent']" type="number" min="30" max="100" class="form-control" placeholder="90">
        </div>
        <div class="form-group">
          <label class="form-label">
            [类库信息] 采集任务间隔（秒），范围 60-86400
          </label>
          <input v-model.number="data['dependency_check.interval']" type="number" min="60" max="86400" class="form-control" placeholder="43200">
        </div>
        <div class="form-group">
          <label class="form-label">
            [响应检测] 采样周期（秒），设置为 0 关闭，最低 60
          </label>
          <input v-model.number="data['response.sampler_interval']" type="number" min="60" class="form-control" placeholder="60">
        </div>
        <div class="form-group">
          <label class="form-label">
            [响应检测] 在采样周期里，最多检测多少次，设置为 0 关闭
          </label>
          <input v-model.number="data['response.sampler_burst']" type="number" min="0" class="form-control" placeholder="5">
        </div>

        <div class="form-group">
          <label class="custom-switch">
            <input v-model="data['cpu.usage.enable']" type="checkbox" checked="data['cpu.usage.enable']" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启熔断保护:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 当CPU占用持续超过某个值，关闭所有防护（仅 Java >= 1.2.1 支持）
            </span>
          </label>
          <br>

          <label class="custom-switch">
            <input v-model="data['plugin.filter']" type="checkbox" checked="data['plugin.filter']" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启文件过滤器: 当文件不存在时不调用检测插件
            </span>
          </label>
          <br>
          <label class="custom-switch">
            <input v-model="data['decompile.enable']" type="checkbox" checked="data['decompile.enable']" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启反汇编功能: 自动提取应用源代码
              <a href="https://rasp.baidu.com/doc/setup/panel.html#decompiler" target="_blank">
                [帮助文档]
              </a>
            </span>
          </label>
          <br>
          <label class="custom-switch">
            <input v-model="data['lru.compare_enable']" type="checkbox" checked="data['lru.compare_enable']" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启 LRU 原始字符串比较（仅 Java >= 1.2.2 支持）
            </span>
          </label>
        </div>
      </div>
      <div v-bind:class="{'card-footer': true, 'sticky-card-footer': sticky}">
        <div class="d-flex">
          <button type="submit" class="btn btn-primary" @click="doSave()">
            保存
          </button>
        </div>
      </div>
    </div>
    <!-- end general settings -->
  </div>
</template>

<script>
import { mapGetters, mapActions, mapMutations } from "vuex";
import { validateRegex, trimSplit, convertToInt } from "@/util"

export default {
  name: 'GeneralSettings',
  data: function() {
    return {
      weak_password_list: '',
      dependency_check: {
        min_interval: 60,
        max_interval: 86400,
      },
      cpu_usage: {
        min_percent: 30,
        max_percent: 100,
        min_interval: 1,
        max_interval: 1800,
      },
      sampler_interval: {
        close: 0,
        min_period: 60,
      },
      data: {
        rasp_config: {},
      }
    }
  },
  computed: {
    ...mapGetters(['current_app', 'sticky'])
  },
  methods: {
    ...mapMutations(["setCurrentApp"]),
    setData: function(data) {      
      this.data = data
      if (this.data['security.weak_passwords']) {
          this.weak_password_list = this.data['security.weak_passwords'].join(',')
      }
    },
    doSave: function() {
      // v1.2 之后，agent 删除 log.maxstack 配置
      // 为了让 v1.2 之后的后台兼容 v1.2 之前的 agent，前端来同步两个配置
      this.data['log.maxstack'] = this.data['plugin.maxstack']
      this.data['security.weak_passwords'] = trimSplit(this.weak_password_list, ',')
      // 空密码也报警
      this.data['security.weak_passwords'].push('')

      if (this.data['security.weak_passwords'].length > 200) {
        alert('为了降低内存占用，弱口令最多允许200条，当前数量 ' + this.data['security.weak_passwords'].length + '条')
        return
      }
      delete this.data['security.weak_password']

      if (this.data['dependency_check.interval'] < this.dependency_check.min_interval ||
          this.data['dependency_check.interval'] > this.dependency_check.max_interval) {
          alert('采集任务间隔范围：' + this.dependency_check.min_interval + '~' + this.dependency_check.max_interval)
          return
      }

      if (this.data['cpu.usage.percent'] < this.cpu_usage.min_percent ||
          this.data['cpu.usage.percent'] > this.cpu_usage.max_percent) {
          alert('单核CPU占用率阈值：' + this.cpu_usage.min_percent + '~' + this.cpu_usage.max_percent)
          return
      }

      if (this.data['cpu.usage.interval'] < this.cpu_usage.min_interval ||
          this.data['cpu.usage.interval'] > this.cpu_usage.max_interval) {
          alert('单核CPU占用率采集间隔：' + this.cpu_usage.min_interval + '~' + this.cpu_usage.max_interval)
          return
      }

      if (this.data['response.sampler_interval'] < this.sampler_interval.min_period &&
          this.data['response.sampler_interval'] > this.sampler_interval.close) {
          alert('采样周期最低60s')
          return
      }

      if (this.data['offline_hosts.cleanup.interval'] < 0) {
          alert('离线主机清理时间必须大于0')
          return
      }

      var body = {
        app_id: this.current_app.id,
        config: this.data
      }

      this.request.post('v1/api/app/general/config', body).then(() => {
        alert('保存成功')
      })
    }
  }
}
</script>
