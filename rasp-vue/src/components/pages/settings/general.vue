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
          <input v-model.trim="data['clientip.header']" type="text" class="form-control">
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
            调试开关 [0表示关闭，1以上的值表示开启]
          </label>
          <input v-model.number="data['debug.level']" type="number" min="0" class="form-control" placeholder="0">
        </div>    
        <div class="form-group">
          <label class="form-label">
            [插件] 单个hook点最大执行时间（ms）
          </label>
          <input v-model.number="data['plugin.timeout.millis']" min="0" type="number" class="form-control" placeholder="100">
        </div>
        <div class="form-group">
          <label class="form-label">
            [日志] 报警日志记录的最大堆栈深度
          </label>
          <input v-model.number="data['log.maxstack']" type="number" min="0" class="form-control" placeholder="50">
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
          <label class="custom-switch">
            <input v-model="data['decompile.enable']" type="checkbox" checked="data['decompile.enable']" class="custom-switch-input">
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              开启反汇编功能
              <a href="https://rasp.baidu.com/doc/setup/panel.html#decompiler" target="_blank">
                [帮助文档]
              </a>
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
    ...mapGetters(['current_app', 'sticky'])
  },
  methods: {
    ...mapMutations(["setCurrentApp"]),
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
