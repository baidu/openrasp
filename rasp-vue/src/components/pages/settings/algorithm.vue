<template>
  <div id="settings-algorithm" class="tab-pane fade">
    <!-- begin algorithm settings -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          防护设置
        </h3>
      </div>
      <div class="card-body">
        <p v-if="! current_app.selected_plugin_id.length">你还没有选择插件，请在「插件管理」中进行设置</p>
        <div class="form-group" v-if="current_app.selected_plugin_id.length">
          <div class="form-label">快速设置</div>
          <label class="custom-switch">
            <input type="checkbox" name="custom-switch-checkbox" v-model="data.meta.all_log" class="custom-switch-input" />
            <span class="custom-switch-indicator">
            </span>
            <span class="custom-switch-description">
              将所有算法设置为「记录日志」模式
            </span>
          </label>
        </div>
        <div class="form-group" v-for="row in items" :key="row.name" v-if="current_app.selected_plugin_id.length">
          <div class="form-label">
            {{ attack_type2name(row.name) }}
          </div>
          <div v-for="item in row.items" :key="item.key">
            <form disabled="true">
              <div class="selectgroup">
                <label class="selectgroup-item">
                  <input type="radio" name="value" value="block" class="selectgroup-input" v-model="data[item.key].action">
                  <span class="selectgroup-button">拦截攻击</span>
                </label>
                <label class="selectgroup-item">
                  <input type="radio" name="value" value="log" class="selectgroup-input" v-model="data[item.key].action">
                  <span class="selectgroup-button">记录日志</span>
                </label>
                <label class="selectgroup-item">
                  <input type="radio" name="value" value="ignore" class="selectgroup-input" v-model="data[item.key].action">
                  <span class="selectgroup-button">完全忽略</span>
                </label>
              </div>
              <p style="display: inline; margin-left: 10px; ">
                {{ item.name }} 
                <a v-if="data[item.key].reference" target="_blank" v-bind:href="data[item.key].reference">[帮助文档]</a>
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
      <div class="card-footer text-right" v-if="current_app.selected_plugin_id.length">
        <div class="d-flex">
          <button type="submit" class="btn btn-primary" @click="saveConfig()">
            保存
          </button>
        </div>
      </div>
    </div>
    <!-- end algorithm settings -->
  </div>
</template>

<script>
import { attack_type2name, block_status2name } from '../../../util'
import { mapGetters } from 'vuex'

export default {
  name: 'AlgorithmSettings',
  data: function () {
    return {
      items: {},
      data: {
        meta: {

        }
      }
    }
  },
  watch: {
    current_app() {
      this.loadConfig();
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  activated: function () {
    if (!this.data.length && this.current_app.selected_plugin_id) {
      this.loadConfig()
    }
  },
  methods: {
    attack_type2name: attack_type2name,
    loadConfig: function () {
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

      this.api_request('v1/api/plugin/get', body, function (data) {
        var tmp = data.algorithm_config, hooks = {}
        self.data = data.algorithm_config

        // 格式换砖
        Object.keys(tmp).forEach(function (key) {
          if (key.indexOf('_') == -1)
            return

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

        Object.keys(hooks).forEach(function (key) {
          hooks[key]['items'].sort(compare)
        })

        self.items = Object.values(hooks)
      })
    },
    saveConfig: function () {
      var self = this
      var body = {
        plugin_id: this.current_app.selected_plugin_id,
        config: this.data
      }

      this.api_request('v1/api/app/algorithm/config', body, function (data) {
        alert('保存成功')
      })
    }
  }
}
</script>
