<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          Agent 管理
        </h1>
        <div class="page-options d-flex">
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <input v-model="hostname" type="text" class="form-control w-10" placeholder="搜索主机或者IP">
          </div>

          <button class="btn btn-primary ml-2" @click="loadRaspList(1)">
            搜索
          </button>
        </div>
      </div>
      <div class="card">
        <div class="card-body">
          <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

          <table v-if="! loading" class="table table-hover table-bordered">
            <thead>
              <tr>
                <th nowrap>
                  主机名
                </th>
                <th>
                  IP
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
                <td>
                  {{ row.hostname }}
                </td>
                <td nowrap>
                  {{ row.register_ip }}
                </td>
                <td nowrap>
                  {{ row.language }}/{{ row.version }} <br>
                  official/{{ row.plugin_version }}
                </td>
                <td>
                  {{ row.rasp_home }}
                </td>
                <td nowrap>
                  {{ moment(row.last_heartbeat_time * 1000).format('YYYY-MM-DD') }} <br>
                  {{ moment(row.last_heartbeat_time * 1000).format('hh:mm:ss') }}
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
                  <a href="javascript:" @click="doDelete(row)">
                    删除
                  </a>
                </td>
              </tr>
            </tbody>
          </table>
          <nav v-if="! loading">
            <b-pagination v-model="currentPage" align="center" :total-rows="total" :per-page="10" />
          </nav>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'
import isIp from 'is-ip'

export default {
  name: 'Hosts',
  data: function() {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      total: 0,
      hostname: ''
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    'currentPage': function(newVal, oldVal) {
      this.loadRaspList(newVal)
    },
    current_app() {
      this.loadRaspList(1)
    }
  },
  methods: {
    loadRaspList: function(page) {
      var self = this
      var body = {
        data: {
          app_id: this.current_app.id
        },
        page: page,
        perpage: 10
      }

      if (this.hostname) {
        if (isIp(this.hostname)) {
          body.data.register_ip = this.hostname
        } else {
          body.data.hostname = this.hostname
        }
      }

      this.api_request('v1/api/rasp/search', body, function(data) {
        self.data = data.data
        self.total = data.total
        self.loading = false
      })
    },
    doDelete: function(data) {
      if (!confirm('确认删除? 删除前请先在主机端卸载 OpenRASP agent')) {
        return
      }
      var body = {
        id: data.id
      }

      this.api_request('v1/api/rasp/delete', body, function(
        data
      ) {
        this.loadRaspList()
      })
    }
  }
}
</script>
