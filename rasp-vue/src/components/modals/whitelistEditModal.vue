<template>
  <div id="whitelistEditModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            添加/编辑 白名单
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>URL - 不区分 http/https，格式如 rasp.baidu.com/phpmyadmin/</label>
            <input v-model="data.url" type="text" class="form-control" maxlen="200">
          </div>
          <div class="form-group">
            <label>检测点</label>
            <div class="row">
              <div class="col-12">
                <label class="custom-switch">
                  <input v-model="data.hook.all" type="checkbox" checked="data.hook.all" class="custom-switch-input">
                  <span class="custom-switch-indicator" />
                  <span class="custom-switch-description">
                    关闭所有检测点
                  </span>
                </label>
              </div>
            </div>
            <div v-if="! data.hook.all" class="row">
              <div v-for="(item, key) in attack_types" :key="key" class="col-6">
                <label class="custom-switch">
                  <input v-model="data.hook[key]" type="checkbox" checked="data.hook[key]" class="custom-switch-input">
                  <span class="custom-switch-indicator" />
                  <span class="custom-switch-description">
                    {{ item }}
                  </span>
                </label>
              </div>
            </div>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-primary" @click="saveApp()">
            保存
          </button>
          <button class="btn btn-default" data-dismiss="modal">
            关闭
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { attack_types } from '../../util/'

export default {
  name: 'WhitelistEditModal',
  data: function() {
    return {
      data: {
        hook: {}
      },
      index: false,
      attack_types: attack_types
    }
  },
  mounted: function() {

  },
  methods: {
    showModal(newData, index) {
      this.data = JSON.parse(JSON.stringify(newData))
      this.index = index

      $('#whitelistEditModal').modal()
    },
    saveApp() {
      if (! this.data.url) {
        return
      }

      if (this.data.url.startsWith('http://') || this.data.url.startsWith('https://')) {
        alert('URL 无需以 http/https 开头，请删除')
        return
      }
      
      $('#whitelistEditModal').modal('hide')
      this.$emit('save', {
        data: this.data,
        index: this.index
      })
    }
  }
}

</script>
