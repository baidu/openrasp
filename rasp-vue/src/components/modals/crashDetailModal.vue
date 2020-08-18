<template>
  <div id="showCrashDetailModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog modal-lg" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            崩溃详情
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body" style="padding-top: 20px">
          <div id="myTabContent" class="tab-content">
            <div class="h6">
              应用堆栈
            </div>
            <pre style="max-height: 400px; overflow-y: scroll">{{ data.stack_trace }}</pre>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-primary mr-auto" v-clipboard:copy="data.stack_trace">
            复制
          </button>

          <button class="btn btn-primary" data-dismiss="modal">
            关闭
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>

export default {
  name: 'CrashDetailModal',
  data: function() {
    return {
      tabIndex: 0,
      data: {
        url: '',
        stack_md5: '',
        stack_trace: '',
      }
    }
  },
  methods: {
    showModal(data) {
      this.tabIndex = 0
      this.data = data
      this.data.stack_trace = data.crash_log.replace(/(^\s*)|(\s*$)/g, "");
      $('#showCrashDetailModal').modal()
    }
  }
}
</script>
