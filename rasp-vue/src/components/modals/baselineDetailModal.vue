<template>
  <div class="modal no-fade" id="showBaselineDetailModal" tabindex="-1" role="dialog">
    <div class="modal-dialog modal-lg" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">基线详情</h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close"></button>
        </div>
        <div class="modal-body" style="padding-top: 0">
          <ul class="nav nav-tabs" id="myTab" role="tablist">
            <li class="nav-item">
              <a class="nav-link active" id="home-tab" data-toggle="tab" href="#vuln">报警信息</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" id="profile-tab" data-toggle="tab" href="#profile" role="tab">资产信息</a>
            </li>
            <!-- <li class="nav-item">
              <a class="nav-link" id="contact-tab" data-toggle="tab" href="#contact" role="tab">修复建议</a>
            </li> -->
          </ul>
          <br>
          <div class="tab-content" id="myTabContent">
            <div class="tab-pane fade show active" id="vuln" role="tabpanel" aria-labelledby="home-tab">
              <div class="h6">
                报警时间
              </div>
              <p>{{ moment(data.event_time).format('YYYY-MM-DD HH:mm:ss') }}</p>
              <div class="h6">
                报警消息
              </div>
              <p>
                [{{ data.policy_id }}] {{ data.message }}
              </p>
              <baseline_params ref="baseline_params"></baseline_params>
            </div>
            <div class="tab-pane fade" id="profile" role="tabpanel" aria-labelledby="profile-tab">
              <div class="h6">
                主机名称
              </div>
              <p>{{ data.server_hostname }}</p>
              <div class="h6">
                服务器 IP
              </div>
              <ul>
                <li v-for="nic in data.server_nic" :key="nic.name">{{ nic.name }}: {{ nic.ip }}</li>
              </ul>
              <div class="h6">
                应用版本
              </div>
              <p>{{ data.server_type }}/{{ data.server_version }}</p>
            </div>
            <!-- <div class="tab-pane fade" id="contact" role="tabpanel" aria-labelledby="contact-tab">
              进入 /var/lib/tomcat/webapps 目录，删除 manager/host-manager 等目录
            </div> -->
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-primary" data-dismiss="modal">关闭</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import moment from 'moment'
import baseline_params from '../pages/baseline_params'

export default {
  name: "baselineDetailModal",
  data: function () {
    return {
      data: {}
    };
  },
  methods: {    
    showModal(data) {
      this.data = data
      this.$refs.baseline_params.setData(data)
      $("#showBaselineDetailModal").modal();
    }
  },
  components: {
    baseline_params
  }
};
</script>
