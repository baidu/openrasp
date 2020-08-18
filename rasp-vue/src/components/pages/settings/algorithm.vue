<template>
  <div>
    <!-- begin algorithm settings -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          {{ data.iast ? '扫描设置' : '防护设置' }}
        </h3>
      </div>
      <div class="card-body" v-if="! current_app.selected_plugin_id || ! current_app.selected_plugin_id.length">
        <p>
          你还没有选择插件，请在「插件管理」中进行设置
        </p>
      </div>
      <div class="card-body" v-else>
        <!-- IAST设置 -->
        <div v-if="data.iast">
          <div class="form-group">
            <label for="">Fuzz 服务器地址</label>
            <input type="text" class="form-control" v-model="data.iast.fuzz_server">
          </div>

          <div class="form-group">
            <label for="">Fuzz 服务器连接超时（毫秒）</label>
            <input type="number" class="form-control" v-model="data.iast.request_timeout">
          </div>

          <div v-bind:class="{'form-group': true, 'has-error': byhost_regex_error}">
            <label for="">使用 HOST 直接访问的服务（正则）<a target="_blank" href="https://rasp.baidu.com/doc/install/iast.html#faq-no-task">[帮助文档]</a></label>
            <input type="text" class="form-control" v-model="data.iast.byhost_regex">
            <span class="text-danger" style="margin-top: 5px; display: block" v-if="byhost_regex_error">{{byhost_regex_error }}</span>
          </div>
        </div>
        <!-- 结束 IAST设置 -->

        <!-- 快速设置 -->
        <div class="form-group" v-if="data.meta && ! data.iast">
          <div class="form-label">
            快速设置
          </div>
          <label class="custom-switch">
            <input
              v-model="data.meta.all_log"
              type="checkbox"
              name="custom-switch-checkbox"
              class="custom-switch-input"
            >
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              将所有算法设置为「记录日志」模式（"XXE 禁止外部实体加载" 算法除外）
            </span>
          </label>
          <br>
          <label class="custom-switch">
            <input
              v-model="data.meta.is_dev"
              type="checkbox"
              name="custom-switch-checkbox"
              class="custom-switch-input"
            >
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              启动「研发模式」，开启一些消耗性能的检测算法
            </span>
          </label>
          <br>
          <label class="custom-switch" v-if="data.meta.log_event != undefined">
            <input
              v-model="data.meta.log_event"
              type="checkbox"
              name="custom-switch-checkbox"
              class="custom-switch-input"
            >
            <span class="custom-switch-indicator" />
            <span class="custom-switch-description">
              打印「行为日志」，仅用于调试，请勿在线上开启
            </span>
          </label>
        </div>
        <!-- 结束 快速设置 -->

        <div
          v-for="row in items"
          :key="row.name"
          class="form-group"
        >
          <div class="form-label">
            {{ attack_type2name(row.name) }}
          </div>
          <div
            v-for="item in row.items"
            :key="item.key"
          >
            <form disabled="true">
              <div class="selectgroup">
                <label class="selectgroup-item">
                  <input
                    v-model="data[item.key].action"
                    type="radio"
                    name="value"
                    value="block"
                    class="selectgroup-input"
                  >
                  <span class="selectgroup-button">
                    拦截攻击
                  </span>
                </label>
                <label class="selectgroup-item">
                  <input
                    v-model="data[item.key].action"
                    type="radio"
                    name="value"
                    value="log"
                    class="selectgroup-input"
                  >
                  <span class="selectgroup-button">
                    记录日志
                  </span>
                </label>
                <label class="selectgroup-item">
                  <input
                    v-model="data[item.key].action"
                    type="radio"
                    name="value"
                    value="ignore"
                    class="selectgroup-input"
                  >
                  <span class="selectgroup-button">
                    完全忽略
                  </span>
                </label>
              </div>
              <p style="display: inline; margin-left: 10px; ">
                {{ item.name }}
                <a
                  v-if="data[item.key].reference"
                  target="_blank"
                  :href="data[item.key].reference"
                >
                  [帮助文档]
                </a>

                <a
                  style="color: #B22222"
                  v-if="hasAdvancedConfig[item.key]"
                  href="javascript:"
                  @click="showAdvancedConfig(item.key, data[item.key])"
                >
                  [高级选项]
                </a>
              </p>
            </form>

            <!--
            <label class="custom-switch">
              <input type="checkbox" name="custom-switch-checkbox" checked="data[key].action == 'block'" class="custom-switch-input" />
              <span class="custom-switch-indicator">
              </span>
              <span class="custom-switch-description">
                {{ item.name }}
              </span>
            </label>
            <br />
            -->
          </div>
        </div>
      </div>
      <div
        v-if="current_app.selected_plugin_id && current_app.selected_plugin_id.length"
        v-bind:class="{'card-footer': true, 'sticky-card-footer': sticky}"
      >
        <button
          type="submit"
          class="btn btn-primary"
          @click="saveConfig()"
          :disabled="byhost_regex_error"
        >
          保存
        </button>
        <button
          type="submit"
          class="btn btn-info pull-right"
          @click="resetConfig()"
        >
          重置
        </button>
      </div>
    </div>
    <!-- end algorithm settings -->

    <AlgorithmConfigModal ref="algorithmConfigModal" @save="applyAdvancedConfig"></AlgorithmConfigModal>
  </div>
</template>

<script>
import {
  attack_type2name,
  block_status2name,
  browser_headers,
  validateRegex
} from '@/util'
import { mapGetters, mapActions, mapMutations } from "vuex";
import AlgorithmConfigModal from "@/components/modals/algorithmConfig"

export default {
  name: 'AlgorithmSettings',
  data: function() {
    return {
      items: {},
      data: {
        meta: {},
      },
      byhost_regex_error: false,
      hasAdvancedConfig: {
        'command_common': true,
        'sql_userinput': true,
        'sql_policy': true,
        'sql_exception': true,
        'sql_regex': true,
        'eval_regex': true,
        'include_protocol': true,
        'xxe_protocol': true,
        'ssrf_protocol': true,
        'response_dataLeak': true,
        'command_error': true
      },
      browser_headers: browser_headers
    }
  },
  components: {
    AlgorithmConfigModal
  },
  computed: {
    ...mapGetters(['current_app', 'sticky'])
  },
  watch: {
    current_app() {
      this.loadConfig()
    },
    'data.iast.byhost_regex': function(newVal, oldVal) {
      this.byhost_regex_error = this.validateRegex(newVal)
    }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.loadConfig()    
  },
  methods: {
    ...mapActions(["loadAppList"]),
    ...mapMutations(["setCurrentApp"]),
    attack_type2name,
    validateRegex,
    showAdvancedConfig: function(key, value) {
      this.$refs.algorithmConfigModal.showModal(key, value)
    },
    applyAdvancedConfig: function(data) {
      if (! data) {
        return
      }

      var key  = data.key
      var data = data.data

      this.data[key] = data
    },
    loadConfig: function() {
      if (!this.current_app.selected_plugin_id.length) {
        return
      }

      var self = this
      var body = {
        id: this.current_app.selected_plugin_id
      }

      function compare(a, b) {
        return a.name.localeCompare(b.name)
      }

      this.request.post('v1/api/plugin/get', body).then(data => {
        var tmp = data.algorithm_config
        var hooks = {}
        self.data = data.algorithm_config

        // 格式转换
        Object.keys(tmp).forEach(function(key) {
          if (key.indexOf('_') == -1) {
            return
          }

          var hook = key.split('_')[0]
          if (!hooks[hook]) {
            hooks[hook] = {
              name: hook,
              items: []
            }
          }

          hooks[hook]['items'].push({
            name: tmp[key]['name'],
            key: key
          })
        })

        Object.keys(hooks).forEach(function(key) {
          hooks[key]['items'].sort(compare)
        })

        self.items = Object.values(hooks)

        // 老版本的官方插件，sql_exception.X.error_code 字段不存在，不要展示高级配置
        if (! data.algorithm_config.sql_exception || ! data.algorithm_config.sql_exception.mysql) {
          self.hasAdvancedConfig['sql_exception'] = false
        }
      })
    },
    saveConfig: function() {
      var body = {
        id: this.current_app.selected_plugin_id,
        config: this.data
      }

      this.request.post('v1/api/plugin/algorithm/config', body).then(() => {
        this.loadAppList(this.current_app.id)
        alert('保存成功，请等待一个心跳周期生效（3分钟以内，取决于客户端配置）')
      })
    },
    resetConfig: function() {
      if (!confirm('还原默认配置？')) {
        return
      }

      var body = {
        id: this.current_app.selected_plugin_id
      }

      this.request.post('v1/api/plugin/algorithm/restore', body).then(() => {
        this.loadAppList(this.current_app.id)
        this.loadConfig()
        alert('恢复成功')
      })
    }
  }
}
</script>
