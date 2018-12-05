<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          基线检查
        </h1>
        <div class="page-options d-flex">
          <div class="input-icon ml-2 w-50">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="loadEvents(1)" />
          </div>

          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <input v-model="hostname" type="text" class="form-control w-10" placeholder="搜索主机或者IP">
          </div>
          <button class="btn btn-primary ml-2" @click="loadEvents(1)">
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
                  报警时间
                </th>
                <th nowrap>
                  应用信息
                </th>
                <th nowrap>
                  主机信息
                </th>
                <th>
                  报警内容
                </th>
                <th>
                  操作
                </th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="row in data" :key="row.id">
                <td nowrap>
                  {{ moment(row.event_time).format('YYYY-MM-DD') }}
                  <br>
                  {{ moment(row.event_time).format('HH:mm:ss') }}
                </td>
                <td nowrap>
                  {{ row.server_type }}/{{ row.server_version }}
                </td>
                <td nowrap>
                  {{ row.server_hostname }}
                </td>
                <td>
                  [{{ row.policy_id }}] {{ row.message }}
                </td>
                <td nowrap>
                  <a href="javascript:" target="_blank" @click="showBaselineDetailModal(row)">
                    查看详情
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

    <baselineDetailModal ref="baselineDetailModal" />
  </div>
</template>

<script>
import baselineDetailModal from '@/components/modals/baselineDetailModal'
import DatePicker from '@/components/DatePicker'
import isIp from 'is-ip'
import { mapGetters } from 'vuex'

export default {
  name: 'Baseline',
  data: function() {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      hostname: '',
      total: 0
    }
  },
  watch: {
    currentPage: function(newVal, oldVal) {
      this.loadEvents(newVal)
    },
    current_app(newVal, oldVal) {
      this.loadEvents(1)
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  activated: function() {
    if (!this.loading && !this.data.length) {
      this.loading = true
      this.loadEvents(1)
    }
  },
  methods: {
    showBaselineDetailModal: function(data) {
      this.$refs.baselineDetailModal.showModal(data)
    },
    loadEvents: function(page) {
      var self = this
      var body = {
        data: {
          app_id: this.current_app.id,
          start_time: this.$refs.datePicker.start.valueOf(),
          end_time: this.$refs.datePicker.end.valueOf()
        },
        page: page,
        perpage: 10
      }

      if (this.hostname) {
        if (isIp(this.hostname)) {
          body.data.local_ip = this.hostname
        } else {
          body.data.server_hostname = this.hostname
        }
      }

      this.loading = true
      this.api_request('v1/api/log/policy/search', body, function(
        data
      ) {
        self.data = data.data
        self.total = data.total
        self.loading = false
      })
    }
  },
  components: {
    baselineDetailModal,
    DatePicker
  }
}
</script>
