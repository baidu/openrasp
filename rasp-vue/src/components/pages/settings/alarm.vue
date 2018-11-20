<template>
  <div id="settings-alarm" class="tab-pane fade">
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
            推送邮箱地址
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <input type="text" class="form-control" v-model="data.email_alarm_conf.recv_addr" />
        </div>
        <div class="form-group">
          <label class="form-label">
            报警标题
          </label>
          <input type="text" class="form-control" v-model="data.email_alarm_conf.subject" />
        </div>
        <div class="form-group">
          <label class="form-label">
            邮件服务器地址
          </label>
          <input type="text" class="form-control" v-model="data.email_alarm_conf.server_addr" />
        </div>
        <div class="form-group">
          <label class="form-label">
            邮箱账号
          </label>
          <input type="email" class="form-control" v-model="data.email_alarm_conf.username" />
        </div>
        <div class="form-group">
          <label class="form-label">
            邮箱密码
          </label>
          <input type="text" class="form-control" v-model="data.email_alarm_conf.password" />
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input type="checkbox" v-model="data.email_alarm_conf.enable" checked="data.email_alarm_conf.enable" class="custom-switch-input" />
            <span class="custom-switch-indicator">
            </span>
            <span class="custom-switch-description">
              开启邮件报警
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
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <input type="text" class="form-control" v-model="data.http_alarm_conf.recv_addr" />
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input type="checkbox" v-model="data.http_alarm_conf.enable" checked="data.http_alarm_conf.enable" class="custom-switch-input" />
            <span class="custom-switch-indicator">
            </span>
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
            推送用户列表
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <input type="text" class="form-control" v-model="data.ding_alarm_conf.recv_user" />
        </div>
        <div class="form-group">
          <label class="form-label">
            推送部门列表
          </label>
          <input type="text" class="form-control" v-model="data.ding_alarm_conf.recv_party" />
        </div>
        <div class="form-group">
          <label class="form-label">
            Corp ID
          </label>
          <input type="text" class="form-control" v-model="data.ding_alarm_conf.corp_id" />
        </div>
        <div class="form-group">
          <label class="form-label">
            Corp Secret
          </label>
          <input type="text" class="form-control" v-model="data.ding_alarm_conf.corp_secret" />
        </div>
        <div class="form-group">
          <label class="form-label">
            Agent ID
          </label>
          <input type="text" class="form-control" v-model="data.ding_alarm_conf.agent_id" />
        </div>
        <div class="form-group">
          <label class="custom-switch">
            <input type="checkbox" v-model="data.ding_alarm_conf.enable" checked="data.ding_alarm_conf.enable" class="custom-switch-input" />
            <span class="custom-switch-indicator">
            </span>
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
    <!-- end alarm settings -->
  </div>
</template>


<script>
import { mapGetters } from 'vuex'

export default {
  name: "AlarmSettings",
  data: function () {
    return {
      data: {
        email_alarm_conf: {
          recv_addr: []
        },
        ding_alarm_conf: {},
        http_alarm_conf: {}
      }
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  methods: {
    setData: function (data) {
      this.data = data
    },
    saveSettings: function (type) {
      var self = this
      var body = {
        app_id: this.current_app.id
      }
      var msg = '报警设置保存成功'

      switch (type) {
        case 'email':
          body['email_alarm_conf'] = self.data.email_alarm_conf
          if (typeof body.email_alarm_conf.recv_addr == 'string') {
            body.email_alarm_conf.recv_addr = body.email_alarm_conf.recv_addr.split(/\s*[,;]\s*/)
          }

          msg = '邮件报警设置保存成功'
          break
        case 'ding':
          body['ding_alarm_conf'] = self.data.ding_alarm_conf
          if (typeof body.ding_alarm_conf.recv_user == 'string') {
            body.ding_alarm_conf.recv_user = body.ding_alarm_conf.recv_user.split(/\s*[,;]\s*/)
          }
          if (typeof body.ding_alarm_conf.recv_party == 'string') {
            body.ding_alarm_conf.recv_party = body.ding_alarm_conf.recv_party.split(/\s*[,;]\s*/)
          }          

          msg = '钉钉报警设置保存成功'
          break
        case 'http':
          body['http_alarm_conf'] = self.data.http_alarm_conf
          if (typeof body.http_alarm_conf.recv_addr == 'string') {
            body.http_alarm_conf.recv_addr = [body.http_alarm_conf.recv_addr]
          }

          msg = 'HTTP 推送设置保存成功'
          break
      }
      // console.log (type, body)

      this.api_request('v1/api/app/alarm/config', body, function (data) {
        alert(msg)
      })
    },
    testSettings: function (type) {
      var self = this
      var body = {
        app_id: this.current_app.id
      }
      var url = 'v1/api/app/' + type + '/test'

      this.api_request(url, body, function (data) {
        alert('发送成功')
      })
    }
  }
};
</script>
