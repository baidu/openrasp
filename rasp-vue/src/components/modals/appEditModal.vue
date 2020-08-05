<template>
  <div id="appEditModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            添加/编辑 应用
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>应用名称</label>
            <input ref="name" v-model.trim="data.name" type="text" class="form-control">
          </div>
          <div class="form-group">
            <label>应用备注</label>
            <input v-model.trim="data.description" type="text" class="form-control">
          </div>
          <div class="form-group">
            <label>应用语言</label>
            <select v-model="data.language" class="form-control">
              <option value="java">
                java
              </option>
              <option value="php">
                php
              </option>
            </select>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-primary" data-dismiss="modal" @click="saveApp()">
            保存
          </button>
          <button class="btn btn-info" data-dismiss="modal">
            关闭
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'AppEditModal',
  data: function() {
    return {
      data: {},
      is_edit: false
    }
  },
  methods: {
    showModal(data, is_edit) {
      this.data = JSON.parse(JSON.stringify(data))
      this.is_edit = is_edit
      $('#appEditModal').modal()
      this.$refs.name.focus()
    },
    saveApp: function() {
      var body = {
        data: this.data,
        is_edit: this.is_edit
      }

      this.$emit('save', body)
    }
  }
}

</script>
