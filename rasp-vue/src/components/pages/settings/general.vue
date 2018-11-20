<template>
  <div id="settings-general" class="tab-pane fade show active">
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
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <input type="text" class="form-control" name="example-text-input" v-model="data['clientip.header']" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义拦截状态码
          </label>
          <select class="custom-select" v-model="data['block.status_code']">
            <option value="302">
              302
            </option>
            <option value="403">
              403
            </option>
            <option value="404">
              404
            </option>
            <option value="500">
              500
            </option>
          </select>
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义拦截跳转页面
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <input type="text" class="form-control" name="example-text-input" v-model="data['block.redirect_url']" />
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义HTML响应内容
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <textarea type="text" class="form-control" v-model="data['block.content_html']">
          </textarea>
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义XML响应内容
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <textarea type="text" class="form-control" v-model="data['block.content_xml']">
          </textarea>
        </div>
        <div class="form-group">
          <label class="form-label">
            自定义JSON响应内容
            <a href="javascript:">
              [帮助文档]
            </a>
          </label>
          <textarea type="text" class="form-control" v-model="data['block.content_json']">
          </textarea>
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
  data: function () {
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
    setData: function (data) {
      this.data = data
    },
    doSave: function () {
      var self = this
      var body = {
        app_id: this.current_app.id,
        config: self.data
      }

      this.api_request('v1/api/app/general/config', body, function (data) {
        alert ('保存成功')
      })
    }
  }
}
</script>
