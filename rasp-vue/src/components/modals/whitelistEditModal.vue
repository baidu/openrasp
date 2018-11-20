<template>
  <div class="modal no-fade" id="whitelistEditModal" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">添加/编辑 白名单</h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close"></button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>URL - 不区分 http/https</label>
            <input type="text" class="form-control" v-model="data.url" maxlen="200">
          </div>
          <div class="form-group">
            <label>检测点</label>
            <div class="row">
              <div class="col-12">
                <label class="custom-switch">
                  <input type="checkbox" v-model="data.hook.all" checked="data.hook.all" class="custom-switch-input"/>
                  <span class="custom-switch-indicator">
                  </span>
                  <span class="custom-switch-description">
                    关闭所有检测点
                  </span>
                </label>
              </div>
            </div>
            <div class="row" v-if="! data.hook.all">
              <div class="col-6" v-for="(item, key) in attack_types" :key="key">
                <label class="custom-switch">
                  <input type="checkbox" v-model="data.hook[key]" checked="data.hook[key]" class="custom-switch-input"/>
                  <span class="custom-switch-indicator">
                  </span>
                  <span class="custom-switch-description">
                    {{ item }}
                  </span>
                </label>
              </div>
            </div>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-primary" data-dismiss="modal" @click="saveApp()">保存</button>
          <button class="btn btn-default" data-dismiss="modal">关闭</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { attack_types } from '../../util/'

export default {
  name: 'whitelistEditModal',
  data: function () {
    return {
      data: {
        hook: {}
      },
      index: false,
      attack_types: attack_types
    }
  },
  mounted: function () {

  },
  methods: {
    showModal(newData, index) {
      this.data  = JSON.parse(JSON.stringify(newData))
      this.index = index

      $('#whitelistEditModal').modal()
    },
    saveApp: function () {
      this.$emit('save', {
        data: this.data,
        index: this.index
      })
    }
  }
}

</script>
