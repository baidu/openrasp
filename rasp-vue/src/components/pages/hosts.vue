<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          Agent 管理
        </h1>
        <div class="page-options d-flex" style="margin-top: 5px;">
          <select v-model="currentVersion" class="form-control">
            <option value="">
              版本: 全部
            </option>
            <option :value="v.version" v-for="v in agent_versions" :key="v.version">
              版本: {{v.version}} ({{ v.count }})
            </option>
          </select>
        </div>
        <div class="page-options d-flex" style="margin-top: 5px; margin-left: 10px; ">
          <div>
            <b-dropdown :text="'状态' + toHostStatus()" class="">
              <div class="row px-2">
                <div class="col-6">
                  <label class="custom-switch">
                    <input v-model="filter.online" type="checkbox" checked="filter.online" class="custom-switch-input" @change="$emit('selected')">
                    <span class="custom-switch-indicator" />
                    <span class="custom-switch-description">
                      在线
                    </span>
                  </label>
                </div>
                <div class="col-6">
                  <label class="custom-switch">
                    <input v-model="filter.offline" type="checkbox" checked="filter.offline" class="custom-switch-input" @change="$emit('selected')">
                    <span class="custom-switch-indicator" />
                    <span class="custom-switch-description">
                      离线
                    </span>
                  </label>
                </div>
              </div>
            </b-dropdown>
            <b-dropdown :text="'语言' + toLanguageStatus()" class="" style="margin-left: 10px; ">
              <div class="row px-2">
                <div class="col-6">
                  <label class="custom-switch">
                    <input v-model="filter.language_java" type="checkbox" checked="filter.language_java" class="custom-switch-input" @change="$emit('selected')">
                    <span class="custom-switch-indicator" />
                    <span class="custom-switch-description">
                      Java
                    </span>
                  </label>
                </div>
                <div class="col-6">
                  <label class="custom-switch">
                    <input v-model="filter.language_php" type="checkbox" checked="filter.language_php" class="custom-switch-input" @change="$emit('selected')">
                    <span class="custom-switch-indicator" />
                    <span class="custom-switch-description">
                      PHP
                    </span>
                  </label>
                </div>
              </div>
            </b-dropdown>
          </div>
          <div class="input-icon ml-3">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <input v-model.trim="hostname" type="text" class="form-control w-10" placeholder="主机名/备注/RASP目录/IP/OS" @keyup.enter="loadRaspList(1)">
          </div>

          <button class="btn btn-primary ml-2" @click="loadRaspList(1)">
            搜索
          </button>

          <a class="btn btn-primary ml-2" v-bind:href="getHref()" target="_blank">
            导出
          </a>

          <button class="btn btn-info ml-2" @click="deleteExpired()">
            清理
          </button>
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
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="loadRaspList($event)" />
          </nav>

          <table v-if="! loading" class="table table-hover table-bordered">
            <thead>
              <tr>
                <th nowrap>
                  主机名
                </th>
                <th>
                  注册 IP
                </th>
                <th>
                  RASP 版本
                </th>
                <th>
                  RASP 目录
                </th>
                <th>
                  上次通信
                </th>
                <th>
                  状态
                </th>
                <th>
                  操作
                </th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="row in data" :key="row.id">
                <td style="min-width: 100px; ">
                  <a href="javascript:" @click="showHostDetail(row)">{{ row.hostname }}</a>
                  <span v-if="row.description"><br>[{{ row.description }}]</span>
                </td>
                <td nowrap>
                  {{ row.register_ip }}
                </td>
                <td nowrap>
                  {{ row.language }}/{{ row.version }} <br>
                  {{ row.plugin_name ? row.plugin_name : 'official' }}/{{ row.plugin_version }}
                </td>
                <td>
                  {{ row.rasp_home }}
                </td>
                <td nowrap>
                  {{ moment(row.last_heartbeat_time * 1000).format('YYYY-MM-DD') }} <br>
                  {{ moment(row.last_heartbeat_time * 1000).format('HH:mm:ss') }}
                </td>
                <td nowrap>
                  <span v-if="! row.online" class="text-danger">
                    离线
                  </span>
                  <span v-if="row.online">
                    正常
                  </span>
                </td>
                <td nowrap>
                  <a href="javascript:" @click="setComment(row)">
                    备注
                  </a>
                  <a href="javascript:" v-if="! row.online" @click="doDelete(row)">
                    删除
                  </a>
                </td>
              </tr>
            </tbody>
          </table>

          <p v-if="! loading && total == 0" class="text-center">暂无数据</p>

          <nav v-if="! loading && total > 10">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block; ">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} / {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="loadRaspList($event)" />
          </nav>
        </div>
      </div>
    </div>

    <HostDetailModal ref="showHostDetail" />
  </div>
</template>

<script>
import isIp from 'is-ip'
import { mapGetters, mapActions, mapMutations } from 'vuex';
import HostDetailModal from '@/components/modals/hostDetailModal'

export default {
  name: 'Hosts',
  components: {
    HostDetailModal
  },
  data: function() {
    return {
      data: [],
      loading: false,
      currentVersion: '',
      agent_versions: [],
      currentPage: 1,
      total: 0,
      hostname: '',
      filter: {
        online: true,
        offline: true,
        language_java: true,
        language_php: true,
      }
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    current_app() { 
      this.currentVersion = ""
      this.loadRaspList(1) 
    },
    currentVersion() {
      this.loadRaspList(1)
    },
    // agent_versions: function(newVal, oldVal) {
    //   if (newVal.length <= 0 && oldVal.length > 0) {
    //       oldVal.forEach(element=>{
    //           element["count"] = 0
    //       })
    //       this.agent_versions = oldVal
    //   }
    // },
    filter: {
      handler() {
        if (!this.current_app.id) {
          return
        }
        this.loadRaspList(1)
        localStorage.setItem('host_filter_status', JSON.stringify(this.filter))
      },
      deep: true
    }
  },
  mounted() {
    // 记住主机状态
    // TODO: 改为类库实现
    // console.log('load filter')
    try {
      let filter = JSON.parse(localStorage.getItem('host_filter_status'))
      if (
        typeof(filter.online) == 'boolean' &&
        typeof(filter.offline) == 'boolean' &&
        typeof(filter.language_java) == 'boolean' &&
        typeof(filter.language_php) == 'boolean'
      ) {
        this.filter = filter
      }
    } catch (e) {}

    // 加载信息
    if (this.current_app.id) {
      this.loadRaspList(1)
    }
  },
  methods: {
    ceil: Math.ceil,
    getHref() {
        return '/v1/api/rasp/csv?app_id=' + this.current_app.id + '&version=' + this.currentVersion +
            '&online=' + this.filter.online + '&offline=' +  this.filter.offline + '&hostname=' + this.hostname + 
            '&language_java=' + this.filter.language_java + '&language_php=' + this.filter.language_php
    },
    enumAgentVersion() {
      const body = {
          data: {
              app_id: this.current_app.id
          },
          page: 1,
          perpage: 10
      }

      if (this.filter.online && !this.filter.offline) {
          body.data.online = true
      } else if (!this.filter.online && this.filter.offline) {
          body.data.online = false
      }
      // 筛选语言
      if (this.filter.language_java && !this.filter.language_php) {
        body.data.language = "java"
      } else if (!this.filter.language_java && this.filter.language_php) {
        body.data.language = "php"
      }
      // if (this.currentVersion) {
      //     body.data.version = this.currentVersion
      // }
      this.request.post('v1/api/rasp/search/version', body).then(res => {
        this.agent_versions = res.data
        if (this.agent_versions.length == 0 && this.currentVersion) {
            this.agent_versions.push({"version": this.currentVersion, "count": 0})
        }
      })
    },
    toHostStatus() {
      if (this.filter.online && this.filter.offline) {
        return ': 全部'
      }
      if (! this.filter.online && ! this.filter.offline) {
        return ''
      }

      if (this.filter.online) {
        return ': 仅在线'
      } else {
        return ': 仅离线'
      }      
    },
    toLanguageStatus() {
      if (this.filter.language_java && this.filter.language_php) {
        return ': 全部'
      }
      if (! this.filter.language_java && ! this.filter.language_php) {
        return ''
      }

      if (this.filter.language_java) {
        return ': 仅 Java'
      } else {
        return ': 仅 PHP'
      }      
    },
    showHostDetail(data) {
      this.$refs.showHostDetail.showModal(data)
    },
    loadRaspList(page) {
      if ((!this.filter.online && !this.filter.offline) || (!this.filter.language_java && !this.filter.language_php)) {
        this.currentPage = page
        this.data = []
        this.total = 0
        this.loading = false
        this.agent_versions = []
        return
      }

      // 每次搜索 rasp，都应该触发一次版本聚合
      this.enumAgentVersion()

      const body = {
        data: {
          app_id: this.current_app.id
        },
        page: page,
        perpage: 10
      }

      if (this.currentVersion) {
        body.data.version = this.currentVersion
      }

      if (this.hostname) {
        body.data.hostname = this.hostname
        // if (isIp(this.hostname)) {
        //   body.data.register_ip = this.hostname
        // } else {
        //  body.data.hostname = this.hostname
        // }
      }
      if (this.filter.online && !this.filter.offline) {
        body.data.online = true
      } else if (!this.filter.online && this.filter.offline) {
        body.data.online = false
      }
      // 筛选语言
      if (this.filter.language_java && !this.filter.language_php) {
        body.data.language = "java"
      } else if (!this.filter.language_java && this.filter.language_php) {
        body.data.language = "php"
      }
      this.loading = true
      return this.request.post('v1/api/rasp/search', body).then(res => {
        this.currentPage = page
        this.data = res.data
        this.total = res.total
        this.loading = false
      })
    },
    setComment: function(data) {
      var oldVal = data.description
      var newVal = prompt('输入新的备注', oldVal)

      if (newVal == null) {
        return
      }

      this.request.post('v1/api/rasp/describe', {
        id: data.id,
        description: newVal
      }).then(res => {
        this.loadRaspList(this.currentPage)
      })
    },
    doDelete: function(data) {
      if (!confirm('确认删除? 删除前请先在主机端卸载 OpenRASP Agent')) {
        return
      }
      var body = {
        id: data.id,
        app_id: this.current_app.id
      }

      this.request
        .post('v1/api/rasp/delete', body)
        .then(() => this.loadRaspList(1))
    },
    deleteExpired: function() {
      if (!confirm('删除离线超过7天的主机？')) {
        return
      }

      var body = {
        app_id: this.current_app.id,
        expire_time: 7 * 24 * 3600
      }
      this.request
        .post('v1/api/rasp/delete', body)
        .then(() => this.loadRaspList(1))
    }
  }
}
</script>
