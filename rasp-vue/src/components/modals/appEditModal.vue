<template>
  <div class="modal no-fade" id="appEditModal" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">添加/编辑 应用</h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close"></button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>应用名称</label>
            <input type="text" class="form-control" v-model="data.name">
          </div>
          <div class="form-group">
            <label>应用备注</label>
            <input type="text" class="form-control" v-model="data.description">
          </div>
          <div class="form-group">
            <label>应用语言</label>
            <select v-model="data.language" class="form-control">
              <option value="java">java</option>
              <option value="php">php</option>
            </select>
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
export default {
  name: 'appEditModal',
  data: function () {
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
    },
    saveApp: function () {
      var body = {
        data: this.data,
        is_edit: this.is_edit
      }
      
      this.$emit('save', body)
    }
  }
}

</script>
