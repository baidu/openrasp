<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          扫描任务列表
        </h1>
        <div class="page-options d-flex" style="margin-top: 5px">
          <div class="p-4">
            <label class="custom-switch" for="checkRefresh">
              <input v-model="refreshFreq" type="checkbox" id="checkRefresh" class="custom-switch-input" :value=true>
              <span class="custom-switch-indicator" />
              <span class="custom-switch-description">自动刷新</span>
            </label>
          </div>

          <div class="p-4">
            <label class="custom-switch" for="autoStart">
              <input v-model="autoStartFlag" type="checkbox" id="autoStart"
                     class="custom-switch-input" :value=false @click="autoStartTask()">
              <span class="custom-switch-indicator" />
              <span class="custom-switch-description">自动启动扫描</span>
            </label>
          </div>

          <div class="p-2">
            <button type="button" class="btn btn-primary ml-2" @click="refreshDriver()">
              <span class="fa fa-refresh" aria-hidden="true"></span> 刷新
            </button>
          </div>
        </div>
      </div>
      <div class="card">
        <div class="card-body">
          <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

          <nav v-if="! loading && total > 0">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block; ">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} / {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="fetchData" />
          </nav>

          <table v-if="! loading" class="table table-hover table-bordered">
            <thead>
            <tr>
              <th nowrap>
                目标
              </th>
              <th nowrap>
                状态
              </th>
              <th nowrap>
                已扫描/已失败/总任务
              </th>
              <th>
                最后收到任务
              </th>
              <th>
                进程资源消耗
              </th>
              <th nowrap>
                操作
              </th>
            </tr>
            </thead>
            <tbody>
            <tr v-for="(row, i) in data">
              <td>
                <a>{{row.host}}:{{row.port}}</a>
              </td>
              <td>
                {{ getTaskStatus(row, i)}}
              </td>
              <td>
                {{row.scanned}}/{{row.failed}}/{{row.total}}
              </td>
              <td>
                {{ row.last_time == 0? "—" : moment(row.last_time * 1000).format('YYYY-MM-DD HH:mm:ss')}}
              </td>
              <td>
                <div v-if="row.id == undefined">—</div>
                <div v-else>CPU {{row.cpu}} / MEM {{row.mem}}</div>
              </td>
              <td align="left">
                <div class="" style="vertical-align:middle">
                  <button type="button" class="btn btn-primary" v-model="taskObject[i]"
                          @click="stopTask(i)" :disabled="taskObject[i] == unscanned || loadIcon[i]"
                          v-show="taskObject[i] != unscanned">
                    <i class="fas fa-spinner fa-spin" v-show="loadIcon[i]"></i>
                    停止扫描
                  </button>
                  <button type="button" class="btn btn-primary"
                          @click="startTask(i)" :disabled="taskObject[i] == running || loadIcon[i]"
                          v-show="taskObject[i] ==  unscanned">
                    <i class="fas fa-spinner fa-spin" v-show="loadIcon[i]"></i>
                    启动扫描
                  </button>
                  <button type="button" class="btn btn-default" data-toggle="modal" @click="getConfig(row)">
                    设置
                  </button>
                  <button type="button" class="btn btn-danger" ng-model="status"
                          @click="cleanTask(i, true)"
                          v-show="taskObject[i] == unscanned">
                    清空队列
                  </button>
                  <button type="button" class="btn btn-danger" ng-model="status"
                          @click="cleanTask(i, false)"
                          v-show="taskObject[i] == unscanned">
                    删除任务
                  </button>
                </div>
              </td>
            </tr>
            </tbody>
          </table>

          <p v-if="! loading && register == 0" class="text-center" v-model="register">扫描器未连接/已离线</p>
          <p v-if="! loading && total == 0 && register == 1" class="text-center" v-model="register">连接中</p>
          <p v-if="! loading && total == 0 && register == 2" class="text-center" v-model="register">扫描器已连接，暂无数据</p>

          <nav v-if="! loading && total > 10">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block; ">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} / {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="fetchData" />
          </nav>

        </div>
      </div>
    </div>

    <IastConfigModal ref="showIastConfigDetail" />
  </div>
</template>

<script>
import IastConfigModal from '@/components/modals/iastConfigModal'
import DatePicker from '@/components/DatePicker'
import { mapGetters } from 'vuex'
import isIp from 'is-ip'

export default {
  name: 'Iast',
  components: {
    IastConfigModal,
    DatePicker
  },
  data() {
    return {
      data: [],
      taskObject: {},
      config: "",
      timer: '',
      refreshFreq: true,
      autoStartFlag: false,
      loadIcon: [],
      loading: false,
      currentPage: 1,
      hostname: '',
      key_word: '',
      total: 0,
      register: 0,
      baseUrl: 'v1/iast',
      running: "运行中",
      cancel: "终止",
      pause: "暂停",
      unknown: "未知状态",
      unscanned: '未启动'
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    current_app() { this.fetchData(1) },
    currentPage() { this.fetchData(this.currentPage) }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.fetchData(1)
    this.timer = setInterval(this.getInterval, 3000);
    this.setLoadIcons()
  },
  beforeDestroy() {
      clearInterval(this.timer);
  },
  methods: {
    ceil: Math.ceil,
    getInterval() {
        if (this.refreshFreq) {
            this.refreshDriver();
        }
    },
    showIastConfigDetail(data) {
        this.$refs.showIastConfigDetail.showModal(data)
    },
    fetchData(page) {
      const body = {
          order: "getAllTasks",
          data: {"page": this.currentPage, "app_id": this.current_app.id},
          headers: {'Content-Type': 'application/json'}
      }
      return this.request.post(this.baseUrl, body)
        .then(res => {
          this.page = page
          if (res.data == undefined) {
              this.status = res.status
              this.data = []
              this.total = 0
          } else {
            this.status = res.data.status
            this.data = res.data.result
            this.total = res.data.total
          }
          this.register = res.register
          this.loading = false
        })
    },
    getRequest(url, order, data) {
        data["app_id"] = this.current_app.id
        const body = {
            order: order,
            data: data,
            headers: {'Content-Type': 'application/json'}
        }
        return this.request.post(url, body)
    },
    setLoadIcons() {
        for(var i = 0; i < this.data.length; i++){
            this.loadIcon.push(false)
        }
    },
    getTaskStatus(row, i) {
        var scannerId = row.id
        if (scannerId == undefined){
            this.taskObject[i] = this.unscanned
            return this.unscanned;
        }else{
            this.taskObject[i] = this.running
            return this.running;
        }
    },
    // stop specific task
    stopTask(i) {
        this.loadIcon[i] = true;
        var scannerId = this.data[i].id
        this.getRequest(this.baseUrl, "stopTask", {"scanner_id": Number(scannerId)})
            .then(res => {
                var status = res.status;
                this.fetchData(this.currentPage);
                if (status == 0) {
                    // alert('终止成功!');
                } else {
                    alert(res.status)
                }
                this.loadIcon[i] = false;
            })
    },

    //start task
    startTask(idx) {
      this.loadIcon[idx] = true;
      var host = this.data[idx].host
      var port = this.data[idx].port
      this.getRequest(this.baseUrl, "startTask", {"host": host, "port": port, "config": {}})
          .then(res => {
              var status = res.status;
              this.fetchData(this.currentPage);
              if(status == 0){
                  // alert("启动扫描任务成功！");
              }else {
                  alert(res.description);
              }
              this.loadIcon[idx] = false;
          })
    },

    cleanTask(taskId, urlOnly) {
      if(urlOnly == false){
          var reallyClean = confirm("确认删除任务?")
          if(reallyClean == true){
              var host = this.data[taskId].host
              var port = this.data[taskId].port
              this.getRequest(this.baseUrl, "cleanTask", {"host": host, "port": port, "url_only": false})
                  .then(res => {
                      var status = res.status;
                      this.fetchData(this.currentPage);
                      if(status == 0){
                          alert("清除成功!");
                      }else {
                          alert(res.description);
                      }
                  })

          }
      } else {
          var tmpUrlOnly = confirm("确认清空扫描队列?")
          if(tmpUrlOnly == true){
              var host = this.data[taskId].host
              var port = this.data[taskId].port
              this.getRequest(this.baseUrl, "cleanTask", {"host": host, "port": port, "url_only": true})
                  .then(res => {
                      var status = res.status;
                      this.fetchData(this.currentPage);
                      if(status == 0){
                          alert("清除成功!");
                      }else {
                          alert(res.description);
                      }
                  })
          }
      }
    },

    // driver refresh
    refreshDriver() {
       this.fetchData(this.currentPage);
    },

    // auto start
    autoStartTask() {
      this.autoStartFlag = !this.autoStartFlag
      this.getRequest(this.baseUrl, "autoStartTask", {"auto_start": this.autoStartFlag})
          .then(res => {
              var status = res.status;
              if(status != 0){
                  alert(res.description);
              }
          })
    },

    getConfig(row) {
        var host = row.host
        var port = row.port
        this.getRequest(this.baseUrl, "getConfig", {"host": host, "port": port})
            .then(res => {
                var status = res.status;
                if(status != 0){
                    alert(res.description);
                }
                var showData = res.data
                showData['host'] = host
                showData['port'] = port
                showData['baseUrl'] = this.baseUrl
                this.showIastConfigDetail(showData)
            })
    }

  }
}
</script>

