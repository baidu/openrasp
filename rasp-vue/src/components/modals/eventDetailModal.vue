<template>
  <div id="showEventDetailModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog modal-lg" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            报警详情
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body" style="padding-top: 0">
          <ul id="myTab" class="nav nav-tabs" role="tablist">
            <li class="nav-item">
              <a id="home-tab" class="nav-link active" data-toggle="tab" href="#vuln">
                漏洞详情
              </a>
            </li>
            <li class="nav-item">
              <a id="home-tab" class="nav-link" data-toggle="tab" href="#home">
                请求信息
              </a>
            </li>
            <li class="nav-item">
              <a id="profile-tab" class="nav-link" data-toggle="tab" href="#profile" role="tab">
                资产信息
              </a>
            </li>
            <li class="nav-item">
              <a id="contact-tab" class="nav-link" data-toggle="tab" href="#contact" role="tab">
                修复建议
              </a>
            </li>
          </ul>
          <br>
          <div id="myTabContent" class="tab-content">
            <div id="vuln" class="tab-pane fade show active" role="tabpanel" aria-labelledby="home-tab">
              <div class="h6">
                报警时间
              </div>
              <p>{{ moment(data.event_time).format('YYYY-MM-DD HH:mm:ss') }}</p>
              <div class="h6">
                报警消息
              </div>
              <p style="word-break: break-all; ">
                [{{ attack_type2name(data.attack_type) }}] {{ data.plugin_message }}
              </p>
              <attack_params ref="attack_params" />
              <div class="h6" v-if="data.stack_trace">
                应用堆栈
              </div>
              <pre v-if="data.stack_trace">{{ data.stack_trace }}</pre>
            </div>
            <div id="home" class="tab-pane fade" role="tabpanel" aria-labelledby="home-tab">
              <div class="h6">
                请求编号
              </div>
              <p>{{ data.request_id }}</p>
              <div class="h6">
                请求 URL
              </div>
              <p style="word-break: break-all; ">
                {{ data.request_method ? data.request_method.toUpperCase() : '' }} <a target="_blank" @href="data.url">
                  {{ data.url }}
                </a>
              </p>
              <div class="h6">
                请求来源
              </div>
              <p>
                {{ data.attack_source }}
                <span v-if="data.attack_location.location_zh_cn != '-'">
                  {{ data.attack_location.location_zh_cn }}
                </span>
              </p>
              <div class="h6" v-if="data.client_ip">
                客户端真实 IP
              </div>
              <p v-if="data.client_ip">
                {{ data.client_ip }}
              </p>              
              <div class="h6">
                请求 Referer
              </div>
              <p style="white-space: normal; word-break: break-all; ">
                {{ data.referer ? data.referer : '-' }}
              </p>
              <div class="h6">
                请求 UA
              </div>
              <p style="word-break: break-all; ">
                {{ data.user_agent ? data.user_agent : '-' }}
              </p>
              <div v-if="data.body" class="h6">
                请求 BODY
              </div>
              <pre v-if="data.body" style="white-space: normal; word-break: break-all; ">{{ data.body ? data.body : '-' }}</pre>
            </div>
            <div id="profile" class="tab-pane fade" role="tabpanel" aria-labelledby="profile-tab">
              <div class="h6">
                主机名称
              </div>
              <p>{{ data.server_hostname }}</p>
              <div class="h6">
                服务器 IP
              </div>
              <ul>
                <li v-for="nic in data.server_nic" :key="nic.name">
                  {{ nic.name }}: {{ nic.ip }}
                </li>
              </ul>
              <div class="h6">
                应用版本
              </div>
              <p style="word-break: break-all; ">
                {{ data.server_type }}/{{ data.server_version }}
              </p>
            </div>
            <div id="contact" class="tab-pane fade" role="tabpanel" aria-labelledby="contact-tab">
              暂无
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
import { attack_type2name } from '../../util'
import attack_params from '../pages/attack_params'

export default {
  name: 'EventDetailModal',
  components: {
    attack_params
  },
  data: function() {
    return {
      data: {
        url: '',
        attack_location: {}
      }
    }
  },
  methods: {
    attack_type2name: attack_type2name,
    showModal(data) {
      this.data = data
      this.$refs.attack_params.setData(data)

      $('#showEventDetailModal').modal()
    }
  }
}
</script>
