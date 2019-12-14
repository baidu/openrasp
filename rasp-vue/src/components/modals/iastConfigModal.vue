<template>
  <div id="setConfigModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog modal-lg" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            任务设置
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body">
            <form class="form-horizontal" id>
              <div class="form-group">
                <div class="col">
                  <label>请选择插件</label>
                </div>
                <div class="row" style="margin-top: 3px;">
                  <template v-for="(value, key, index) in data.scan_plugin_status">
                      <div class="col">
                        <label class="custom-switch" style="min-width: 200px;">
                          <input v-model="value.enable" type="checkbox" class="custom-switch-input">
                          <span class="custom-switch-indicator" @click="value.enable = !value.enable"></span>
                          <span class="custom-switch-description">{{value.show_name}}</span>
                        </label>
                      </div>
                  </template>
                </div>
              </div>
              <div class="form-group">
                <div class="col">
                  <label>url白名单正则（被正则命中的url不会被扫描, 正则会匹配request_uri部分,
                    例如 http://test.com/path/eg?p=1&b=2会匹配 /path/eg?p=1&b=2, 当不希望扫描</label>
                  <input type="text" class="form-control" v-model="data.white_url_reg">
                  <label v-show="byurl_regex_error" style="color:#ff0000">*白名单正则非法</label>
                </div>
              </div>
              <div class="form-group">
                <div class="col">
                  <label for="concurrent" align="center">最大并发请求数</label>
                  <input type="number" class="form-control" id="concurrent"
                         v-model="data.scan_rate.concurrent" placeholder="20">
                  <label v-show="data.scan_rate.concurrent <= 0" style="color:#F00">*最大并发数应大于0</label>
                </div>
              </div>
              <div class="form-group">
                <div class="col">
                  <label for="minInterval" align="center">请求间隔(ms)</label>
                </div>
                <div class="row">
                  <div class="col">
                    <input type="number" class="form-control" id="minInterval"
                           v-model="data.scan_rate.minInterval" placeholder="0">
                    <label v-show="data.scan_rate.minInterval < 0" style="color:#F00">*最小间隔应大于等于0</label>
                  </div>
                  <div class="col" align="center">
                    <h5>～</h5>
                  </div>
                  <div class="col">
                    <input type="number" class="form-control" id="maxInterval"
                           v-model="data.scan_rate.maxInterval" placeholder="1000">
                    <label v-show="data.scan_rate.maxInterval <= 0" style="color:#F00">*最大间隔应大于0</label>
                  </div>
                </div>
              </div>
              <div class="form-group">
                <div class="col">
                    <label>扫描请求使用的http代理</label>
                  <input type="text" class="form-control"
                         id="scanProxy" v-model="data.scan_proxy">
                </div>
              </div>
            </form>

        </div>
        <div class="modal-footer">
          <button type="button" class="btn btn-primary pull-left"
                  @click="setConfigTask()"
                  :data-dismiss=modalDisplay>确定
          </button>
          <button class="btn btn-primary" :click="reset()" data-dismiss="modal">
            关闭
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { validateRegex } from "@/util"


export default {
  name: 'IastConfigModal',
  data: function() {
    return {
      data: {
          scan_rate: {
              concurrent: 0,
              minInterval: 0,
              maxInterval: 10
          }
      },
      scan_proxy: "",
      concurrent: 0,
      minInterval: 0,
      maxInterval: 10,
      urlWhiteRegex: "",
      scan_plugin_status: {},
      byurl_regex_error: false,
      modalDisplay: false
    }
  },
  watch: {
      'data.white_url_reg': function(newVal, oldVal) {
          this.byurl_regex_error = this.validateRegex(newVal)
      }
  },
  methods: {
    showModal(data) {
      this.data = data
      $('#setConfigModal').modal()
    },
    validateRegex,
    getRequest(url, order, data) {
        const body = {
            order: order,
            data: data,
            headers: {'Content-Type': 'application/json'}
        }
        return this.request.post(url, body)
    },
    // set config task
    setConfigTask() {
        var host = this.data.host
        var port = this.data.port
        var scanRate = this.data.scan_rate
        this.modalDisplay = "modal";
        this.concurrent = isNaN(Number(scanRate.concurrent))? undefined : Number(scanRate.concurrent);
        this.minInterval = isNaN(Number(scanRate.minInterval))? undefined : Number(scanRate.minInterval);
        this.maxInterval = isNaN(Number(scanRate.maxInterval))? undefined : Number(scanRate.maxInterval);
        this.white_url_reg == undefined? "" : this.data.white_url_reg;
        this.scan_proxy == undefined? "" : this.data.scan_proxy;

        if(!this.byurl_regex_error && this.concurrent > 0 && this.maxInterval > 0 && this.minInterval >= 0){
            if(this.minInterval <= this.maxInterval){
                console.log("baseUrl:", this.data.baseUrl)
                this.getRequest(this.data.baseUrl, "setConfig", {
                    "host": host,
                    "port": port,
                    "config": {
                        "scan_plugin_status": this.data.scan_plugin_status,
                        "scan_rate": {
                            "max_concurrent_request": this.concurrent,
                            "max_request_interval": this.maxInterval,
                            "min_request_interval": this.minInterval
                        },
                        "white_url_reg": this.urlWhiteRegex,
                        "scan_proxy": this.scan_proxy
                    }
                }).then(res => {
                    var status = res.status;
                    if(status == 0){
                        alert("设置成功");
                        this.concurrent = undefined;
                        this.minInterval = undefined;
                        this.maxInterval = undefined;
                    }else{
                        alert(res.description);
                    }
                })

            }else{
                this.minInterval = undefined
                alert("最小间隔不应超过最大间隔")
                this.modalDisplay = false;
            }
        }else{
            this.modalDisplay = false;
        }
    },
    setPluginStatus(key) {
        this.data.scan_plugin_status[pl].enable = !this.data.scan_plugin_status[pl].enable
    },
    reset() {
      this.concurrent = undefined;
      this.minInterval = undefined;
      this.maxInterval = undefined;
    }
  }
}
</script>
