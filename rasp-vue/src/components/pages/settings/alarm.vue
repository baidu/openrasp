<template>
  <div>
    <!-- begin alarm methods -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          报警类型配置
        </h3>
        <div class="card-options">
          <!-- <label class="custom-switch m-0">
            <input type="checkbox" value="1" class="custom-switch-input">
            <span class="custom-switch-indicator"></span>
          </label> -->
        </div>
      </div>
      <div class="card-body">
        <div v-for="(descr, name) in attack_types" :key="name">
          <div class="row" style="margin-top: 3px">
            <div class="col">
              <label style="min-width: 220px;">{{ descr }}</label>
            </div>
            <div class="col">
              <label class="custom-switch m-0">
                <input type="checkbox" value="1" class="custom-switch-input" v-model="sendMethods[name].email">
                <span class="custom-switch-indicator"></span>
                <span class="custom-switch-description">邮件报警</span>
              </label>
            </div>
            <div class="col">
              <label class="custom-switch m-0">
                <input type="checkbox" value="1" class="custom-switch-input" v-model="sendMethods[name].ding">
                <span class="custom-switch-indicator"></span>
                <span class="custom-switch-description">钉钉报警</span>
              </label>
            </div>
            <div class="col">
              <label class="custom-switch m-0">
                <input type="checkbox" value="1" class="custom-switch-input" v-model="sendMethods[name].http">
                <span class="custom-switch-indicator"></span>
                <span class="custom-switch-description">HTTP 推送</span>
              </label>
            </div>
          </div>
        </div>
      </div>
      
      <div v-bind:class="{'card-footer': true, 'sticky-card-footer': sticky}">
        <button type="submit" class="btn btn-primary" @click="saveAlarmMethods()">
          保存
        </button>

        <button type="submit" class="btn btn-info pull-right" @click="resetAlarmMethods(true, true);">
          重置
        </button>
      </div>
    </div>
    <!-- end alarm methods -->

    <!-- begin alarm settings -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          邮件报警
        </h3>
        <div class="card-options">
          <!-- <label class="custom-switch m-0">
            <input type="checkbox" value="1" class="custom-switch-input">
            <span class="custom-switch-indicator"></span>
          </label> -->
        </div>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="form-label">
            推送邮箱地址 - 逗号或者分号分隔
          </label>
          <input v-model.trim="data.email_alarm_conf.recv_addr" type="text" class="form-control" placeholder="user1@example.com; user2@example.com">
        </div>
        <div class="form-group">
          <label class="form-label">
            报警标题
          </label>
          <input v-model.trim="data.email_alarm_conf.subject" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义 From 头信息
          </label>
          <input v-model.trim="data.email_alarm_conf.from" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            邮件服务器地址
          </label>
          <input v-model.trim="data.email_alarm_conf.server_addr" type="text" class="form-control" placeholder="smtp.163.com:25" autocomplete="off">
        </div>
        <div class="form-group">
          <label class="form-label">
            邮箱账号
          </label>
          <input v-model="data.email_alarm_conf.username" type="email" class="form-control" placeholder="hello@163.com" autocomplete="off">
        </div>
        <div class="form-group">
          <label class="form-label">
            邮箱密码
          </label>
          <input v-model="data.email_alarm_conf.password" type="password" class="form-control" autocomplete="off">
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input v-model="data.email_alarm_conf.enable" type="checkbox" checked="data.email_alarm_conf.enable" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启邮件报警
            </span>
          </label>
        </div>

        <div class="form-group">
          <label class="custom-switch">
            <input v-model="data.email_alarm_conf.tls_enable" type="checkbox" checked="data.email_alarm_conf.enable" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              强制开启 TLS <a target="_blank" href="https://rasp.baidu.com/doc/setup/panel.html#email-alarm">[帮助文档]</a>
            </span>
          </label>
        </div>
      </div>
      <div class="card-footer">
        <button type="submit" class="btn btn-primary" @click="saveSettings('email')">
          保存
        </button>
        <button type="submit" class="btn btn-info pull-right" @click="testSettings('email')">
          发送测试数据
        </button>
      </div>
    </div>
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          HTTP 推送
        </h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="form-label">
            HTTP/HTTPS URL
          </label>
          <input v-model.trim="data.http_alarm_conf.recv_addr" type="text" class="form-control" placeholder="http://myserver/myurl">
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input v-model="data.http_alarm_conf.enable" type="checkbox" checked="data.http_alarm_conf.enable" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启报警推送
            </span>
          </label>
        </div>
      </div>
      <div class="card-footer">
        <button type="submit" class="btn btn-primary" @click="saveSettings('http')">
          保存
        </button>
        <button type="submit" class="btn btn-info pull-right" @click="testSettings('http')">
          发送测试数据
        </button>
      </div>
    </div>
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          钉钉集成
        </h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="form-label">
            推送用户列表 - 逗号或者分号分隔
            <a target="_blank" href="https://rasp.baidu.com/doc/setup/panel.html#dingding-alarm">
              [帮助文档]
            </a>
          </label>
          <input v-model.trim="data.ding_alarm_conf.recv_user" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            推送部门列表
          </label>
          <input v-model.trim="data.ding_alarm_conf.recv_party" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            Corp ID
          </label>
          <input v-model.trim="data.ding_alarm_conf.corp_id" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            Corp Secret
          </label>
          <input v-model.trim="data.ding_alarm_conf.corp_secret" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            Agent ID
          </label>
          <input v-model.trim="data.ding_alarm_conf.agent_id" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input v-model="data.ding_alarm_conf.enable" type="checkbox" checked="data.ding_alarm_conf.enable" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启钉钉推送
            </span>
          </label>
        </div>
      </div>
      <div class="card-footer">
        <button type="submit" class="btn btn-primary" @click="saveSettings('ding')">
          保存
        </button>
        <button type="submit" class="btn btn-info pull-right" @click="testSettings('ding')">
          发送测试数据
        </button>
      </div>
    </div>

    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          Syslog 报警配置
        </h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="form-label">
            服务器地址
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-syslog" target="_blank">
              [帮助文档]
            </a>
          </label>
          <input v-model.trim="data.general_config['syslog.url']" type="text" class="form-control" placeholder="tcp://1.1.1.1:6666">
        </div>
        <div class="form-group">
          <label class="form-label">
            Facility
          </label>
          <b-form-input v-model="data.general_config['syslog.facility']" type="number" class="form-control" />
        </div>
        <div class="form-group">
          <label class="form-label">
            Tag
          </label>
          <input v-model.trim="data.general_config['syslog.tag']" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input v-model="data.general_config['syslog.enable']" type="checkbox" checked="data.general_config['syslog.enable']" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启 syslog 日志
            </span>
          </label>
        </div>
      </div>
      <div class="card-footer">
        <button type="submit" class="btn btn-primary" @click="saveSettings('syslog')">
          保存
        </button>
      </div>
    </div>
    <!-- end alarm settings -->
  </div>
</template>

<script>
import { mapGetters, mapActions, mapMutations } from "vuex";
import { getDefaultConfig, attack_types } from '@/util'

export default {
  name: 'AlarmSettings',
  props: {
    data: {
      type: Object,
      default() {
        return getDefaultConfig()
      }
    }
  },
  data: function() {
    return {
      attack_types: attack_types,
      sendMethods: {}
    }
  },
  computed: {
    ...mapGetters(['current_app', 'sticky'])
  },
  watch: {
    data: function(newVal, oldVal) {
      this.loadAlarmMethod()
    }
  },
  beforeMount: function () {
    this.resetAlarmMethods(false)
  },
  methods: {
    loadAlarmMethod: function() {
      var self = this
      var conf = self.data.attack_type_alarm_conf

      // 没配置等于全开
      if (! conf) {
        this.resetAlarmMethods(true)
        return
      }

      // 转换
      Object.keys(conf).forEach(function (name) {
        self.sendMethods[name] = {
          ding: false,
          http: false,
          email: false
        }

        conf[name].forEach(function (method) {
          self.sendMethods[name][method] = true
        })
      })
    },
    resetAlarmMethods: function(value, save) {
      var self = this
      Object.keys(this.attack_types).forEach(function (name) {
        if (! self.sendMethods[name]) {
          self.sendMethods[name] = {}
        }
        
        ['email', 'ding', 'http'].forEach(function (key) {
          self.sendMethods[name][key] = value
        })
      })

      self.sendMethods = Object.assign({}, self.sendMethods)

      // 重置时保存
      if (save) {
        self.saveAlarmMethods()
      }
    },
    saveAlarmMethods: function(data) {
      var self = this
      var body = { 
        app_id: self.current_app.id,
        attack_type_alarm_conf: {} 
      }

      Object.keys(self.sendMethods).forEach(function (name) {
        var tmp = self.sendMethods[name]
        body['attack_type_alarm_conf'][name] = []

        Object.keys(tmp).forEach(function (method) {
          if (tmp[method]) {
            body['attack_type_alarm_conf'][name].push(method)
          }
        })
      })

      this.request.post('v1/api/app/alarm/config', body)
        .then(() => {
          alert('报警方式保存成功')
        })
    },    
    saveSettings: function(type) {
      if (type === 'syslog') {
        try {
          this.data.general_config['syslog.facility'] = parseInt(this.data.general_config['syslog.facility'])
        } catch (err) {
          this.data.general_config['syslog.facility'] = null
        }
        msg = 'Syslog 报警设置保存成功'
        return this.request.post('v1/api/app/general/config', {
          app_id: this.current_app.id,
          config: this.data.general_config
        }).then(() => {
          alert(msg)
        })
      }
      var body = {
        app_id: this.current_app.id
      }
      var msg = '报警设置保存成功'

      switch (type) {
        case 'email':
          body['email_alarm_conf'] = this.data.email_alarm_conf
          if (typeof body.email_alarm_conf.recv_addr === 'string') {
            body.email_alarm_conf.recv_addr = body.email_alarm_conf.recv_addr.split(/\s*[,;]\s*/).filter(item => !!item)
          }

          msg = '邮件报警设置保存成功'
          break
        case 'ding':
          body['ding_alarm_conf'] = this.data.ding_alarm_conf
          if (typeof body.ding_alarm_conf.recv_user === 'string') {
            body.ding_alarm_conf.recv_user = body.ding_alarm_conf.recv_user.split(/\s*[,;]\s*/).filter(item => !!item)
          }
          if (typeof body.ding_alarm_conf.recv_party === 'string') {
            body.ding_alarm_conf.recv_party = body.ding_alarm_conf.recv_party.split(/\s*[,;]\s*/).filter(item => !!item)
          }

          msg = '钉钉报警设置保存成功'
          break
        case 'http':
          body['http_alarm_conf'] = this.data.http_alarm_conf
          if (typeof body.http_alarm_conf.recv_addr === 'string') {
            body.http_alarm_conf.recv_addr = [body.http_alarm_conf.recv_addr]
          }

          msg = 'HTTP 推送设置保存成功'
          break
      }

      return this.request.post('v1/api/app/alarm/config', body)
        .then(() => {
          alert(msg)
        })
    },
    testSettings: function(type) {
      this.saveSettings(type)
        .then(() => this.request.post('v1/api/app/' + type + '/test', { app_id: this.current_app.id }))
        .then(() => alert('发送成功'))
    }
  }
}
</script>
