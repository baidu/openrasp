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
              
              <div v-if="data.merged_code">
                <div class="h6">
                  应用源代码
                </div>
                <pre><div v-for="(row, index) in data.merged_code" :key="index">{{data.merged_code.length - index}}. {{row.stack}}<br/>{{row.code}}
                </div></pre>
              </div>

            </div>
            <div id="home" class="tab-pane fade" role="tabpanel" aria-labelledby="home-tab">
              <div class="h6">
                请求编号
              </div>
              <p>
                {{ data.request_id ? data.request_id : '-' }}
              </p>
              <div class="h6">
                请求 URL
              </div>
              <p style="word-break: break-all; ">
                {{ data.request_method ? data.request_method.toUpperCase() : '' }} 
                <a target="_blank" @href="data.url">
                  {{ data.url ? data.url : '-' }}
                </a>
              </p>
              <div class="h6">
                请求来源
              </div>
              <p>
                {{ data.attack_source ? data.attack_source : '-' }}
                <span v-if="data.attack_location && data.attack_location.location_zh_cn != '-'">
                  {{ data.attack_location.location_zh_cn }}
                </span>
              </p>
              <div class="h6" v-if="data.client_ip">
                客户端真实 IP
              </div>
              <p v-if="data.client_ip">
                {{ data.client_ip }}
              </p>            

              <div v-if="data.header && Object.keys(data.header).length">
                <div class="h6" v-if="data.header.referer">
                  请求 Referer
                </div>
                <p style="white-space: normal; word-break: break-all; " v-if="data.header.referer">
                  {{ data.header.referer }}
                </p>

                <div class="h6" v-if="data.header.user_agent">
                  请求 UA
                </div>
                <p style="word-break: break-all; " v-if="data.header.user_agent">
                  {{ data.header.user_agent }}
                </p>

                <div class="h6">
                  完整 Header 信息
                </div>
                <pre>{{mergeHeaders(data.header)}}</pre>
              </div>

              <!-- 兼容没有 header 字段的老版本 -->
              <div v-else>
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
              </div>              

              <div v-if="data.body">
                <div class="h6">
                  请求 BODY
                </div>
                <pre style="word-break: break-all; ">{{ data.body }}</pre>
              </div>
            </div>
            <div id="profile" class="tab-pane fade" role="tabpanel" aria-labelledby="profile-tab">
              <div class="h6">
                主机名称
              </div>
              <p>
                {{ data.server_hostname }}
              </p>

              <div class="h6">
                服务器 IP
              </div>
              <ul>
                <li v-for="nic in data.server_nic" :key="nic.name">
                  {{ nic.name }}: {{ nic.ip }}
                </li>
              </ul>

              <div v-if="data.server_type">
                <div class="h6">
                  应用版本
                </div>
                <p style="word-break: break-all; ">
                  {{ data.server_type }}/{{ data.server_version }}
                </p>
              </div>            
            </div>
            <div id="contact" class="tab-pane fade" role="tabpanel" aria-labelledby="contact-tab">
              <fix_solutions ref="fix_solutions"></fix_solutions>
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
import fix_solutions from '../pages/fix_solutions'

export default {
  name: 'EventDetailModal',
  components: {
    attack_params,
    fix_solutions
  },
  data: function() {
    return {
      data: {
        url: '',
        attack_location: {},
        source_code: [],
        stack_trace: '',
        merged_code: []
      }
    }
  },
  methods: {
    attack_type2name: attack_type2name,
    showModal(data) {
      this.data = data
      this.$refs.attack_params.setData(data)
      this.$refs.fix_solutions.setData(data)
      this.mergeStackAndSource(data)

      $('#showEventDetailModal').modal()
    },
    mergeHeaders(data) {
      var result = ''
      for (let key in data) {
        result = result + "\n" + key + ": " + data[key]
      }

      return result.trim()
    },
    mergeStackAndSource(data) {
      if (! data.source_code || ! data.source_code.length) {
        return
      }
      
      let stack_trace = data.stack_trace.trim().split("\n")
      if (stack_trace.length != data.source_code.length) {
        console.error("Error: stack_trace size '" + stack_trace.length + "' is different from source_code size '" + data.source_code.length + "', skipped")
        return
      }

      this.data.merged_code = []
      for (let i = 0; i < stack_trace.length; i ++) {
        var stack = stack_trace[i]
        var code  = data.source_code[i].trim()

        this.data.merged_code.push({
          stack: stack,
          code: code.length ? code : "(空)"
        })
      }
    }
  }
}
</script>
