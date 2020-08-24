<template>
  <div id="showHostDetailModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog modal-lg" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            主机详情
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body" style="padding-top: 0">
          <ul id="myTab" class="nav nav-tabs" role="tablist">
            <li class="nav-item">
              <a id="home-tab" class="nav-link active" data-toggle="tab" href="#basic">
                基础信息
              </a>
            </li>
            <li class="nav-item">
              <a class="nav-link" id="profile-tab" data-toggle="tab" href="#profile" role="tab">
                RASP 信息
              </a>
            </li>
          </ul>
          <br>
          <div id="myTabContent" class="tab-content">
            <div id="basic" class="tab-pane fade show active" role="tabpanel" aria-labelledby="home-tab">
              <div class="h6">
                主机名称
              </div>
              <p>{{ data.hostname }}</p>
              
              <div v-if="data.host_type">
                <div class="h6">
                  容器类型
                </div>
                <p>{{ data.host_type }}</p>
              </div>

              <div class="h6">
                注册时间
              </div>
              <p>{{ moment(data.register_time * 1000).format('YYYY-MM-DD HH:mm:ss') }}</p>              

              <div class="h6">
                注册 IP
              </div>
              <p style="word-break: break-all; ">
                {{ data.register_ip }}
              </p>
              <div v-if="data.environ && Object.keys(data.environ).length != 0">
                <div class="h6">
                  环境变量
                </div>
                <pre>{{ env2str(data.environ) }}</pre>
              </div>
            </div>

            <div class="tab-pane fade" id="profile" role="tabpanel" aria-labelledby="profile-tab">
              <div class="h6">
                服务器版本
              </div>
              <p>{{ data.language }}/{{ data.language_version }}</p>

              <div class="h6">
                Agent 版本
              </div>
              <p>{{ data.version }}</p>

              <div class="h6">
                插件版本
              </div>
              <p>{{ data.plugin_version }}</p>

              <div class="h6">
                RASP ID
              </div>
              <p>{{ data.id }}</p>
            </div>
          </div>
        </div>
        <div class="modal-footer">
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
  name: 'HostDetailModal',
  data: function() {
    return {
      data: {
      }
    }
  },
  methods: {
    showModal(data) {
      this.data = data
      $('#showHostDetailModal').modal()
    },
    env2str(environ) {
      var result = ''
      for (let name in environ) {
        result = result + name + '=' + environ[name] + "\n"
      }

      return result
    }
  }
}
</script>
