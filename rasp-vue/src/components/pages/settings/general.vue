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
          <input v-model="data['clientip.header']" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义拦截状态码
          </label>
          <b-form-select v-model="data['block.status_code']" :options="[302, 403, 404, 500]" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义拦截跳转页面
          </label>
          <input v-model="data['block.redirect_url']" type="text" class="form-control">
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义HTML响应内容
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-block" target="_blank">
              [帮助文档]
            </a>
          </label>
          <textarea v-model="data['block.content_html']" type="text" class="form-control" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义XML响应内容
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-block" target="_blank">
              [帮助文档]
            </a>
          </label>
          <textarea v-model="data['block.content_xml']" type="text" class="form-control" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义JSON响应内容
            <a href="https://rasp.baidu.com/doc/setup/others.html#common-block" target="_blank">
              [帮助文档]
            </a>
          </label>
          <textarea v-model="data['block.content_json']" type="text" class="form-control" />
        </div>
      </div>
      <div class="card-footer text-right">
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
import { mapGetters } from 'vuex'

export default {
  name: 'GeneralSettings',
  data: function() {
    return {
      data: {
        rasp_config: {}
      }
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  methods: {
    setData: function(data) {
      this.data = data
    },
    doSave: function() {
      var self = this
      var body = {
        app_id: this.current_app.id,
        config: self.data
      }

      this.api_request('v1/api/app/general/config', body, function(data) {
        alert('保存成功')
      })
    }
  }
}
</script>
