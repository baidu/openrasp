<template>
  <div id="appEditModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            高级选项
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body">

          <div v-if="key == 'sql_userinput'">
            <label class="custom-switch m-0">
              <input type="checkbox" v-model="data.pre_enable" class="custom-switch-input">
              <span class="custom-switch-indicator" @click="data.pre_enable = !data.pre_enable"></span>
              <span class="custom-switch-description">开启关键词过滤: 有漏洞但是没有攻击时不报警</span>              
            </label>

            <label class="custom-switch m-0">
              <input type="checkbox" v-model="data.allow_full" class="custom-switch-input">
              <span class="custom-switch-indicator" @click="data.allow_full = !data.allow_full"></span>
              <span class="custom-switch-description">允许数据库查询: 通过接口传递完整SQL语句并执行</span>              
            </label>
          </div>

          <div v-if="key == 'sql_policy'">
            <label class="custom-switch m-0">
              <input type="checkbox" v-model="data.feature.information_schema" class="custom-switch-input">
              <span class="custom-switch-indicator" @click="data.feature.information_schema = !data.feature.information_schema"></span>
              <span class="custom-switch-description">拦截 information_schema 相关操作</span>              
            </label>
          </div>

          <div v-if="key == 'sql_regex'">
            <label>SQL语句正则表达式</label>
            <div v-bind:class="{'form-group': true, 'has-error': sql_regex_error}">
              <input type="text" v-model="data.regex" class="form-control">
            </div>
            <span class="text-danger" v-if="sql_regex_error">{{sql_regex_error }}</span>
          </div>

        </div>
        <div class="modal-footer">
          <button class="btn btn-primary mr-auto" data-dismiss="modal" @click="saveConfig()" :disabled="sql_regex_error">
            保存
          </button>
          <button class="btn btn-info" data-dismiss="modal" @click="saveConfig()">
            关闭
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'AlgorithmConfigModal',
  data: function() {
    return {
      key: '',
      data: {},
      sql_regex_error: false
    }
  },
  watch: {
    'data.regex': function(newval, oldval) {
      this.validateRegex(newval)
    }
  },
  methods: {
    validateRegex: function(value) {
      var error = false
      try {
        new RegExp(value)
      } catch (e) {
        error = '正则表达式错误:' + e.toString()
      }

      this.sql_regex_error = error
    },
    showModal(key, data) {
      this.key  = key
      this.data = JSON.parse(JSON.stringify(data))
      $('#appEditModal').modal({
        backdrop: 'static',
        keyboard: false
      })
    },
    saveConfig: function() {
      var body = {
        key:  this.key,
        data: this.data,
      }

      this.$emit('save', body)
    }
  }
}

</script>
