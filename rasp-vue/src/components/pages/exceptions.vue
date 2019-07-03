<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          异常日志
        </h1>
        <div class="page-options d-flex">
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="fetchData(1)" />
          </div>
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <b-form-input v-model.trim="hostname" type="text" class="form-control" placeholder="搜索主机或者IP" @keyup.enter="fetchData(1)" />
          </div>
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <b-form-input v-model.trim="message" type="text" class="form-control" placeholder="搜索消息" @keyup.enter="fetchData(1)" />
          </div>
          <button class="btn btn-primary ml-2" @click="fetchData(1)">
            搜索
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
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="fetchData" />
          </nav>

          <b-table hover bordered :items="data" :fields="fields">
            <template slot="event_time" slot-scope="scope">
              {{ moment(scope.item.event_time).format('YYYY-MM-DD') }}
              <br>
              {{ moment(scope.item.event_time).format('HH:mm:ss') }}
            </template>
            <template slot="message" slot-scope="scope">
              [{{ scope.item.level }}] {{ scope.item.message }}
            </template>
            <template slot="server_hostname" slot-scope="scope">
              {{ scope.item.server_hostname }}<br/>
              <span v-for="nic in scope.item.server_nic" :key="nic.name">{{ nic.name }}: {{ nic.ip }}<br/></span>
            </template>
            <template slot="button" slot-scope="scope">
              <a href="javascript:" @click="showExceptionDetail(scope.item)">
                查看详情
              </a>
            </template>
          </b-table>

          <p v-if="! loading && total == 0" class="text-center">暂无数据</p>

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

    <ExceptionDetailModal ref="showExceptionDetail" />
  </div>
</template>

<script>
import EventDetailModal from '@/components/modals/eventDetailModal'
import ExceptionDetailModal from '@/components/modals/exceptionDetailModal'
import DatePicker from '@/components/DatePicker'
import { mapGetters } from 'vuex'
import isIp from 'is-ip'

export default {
  name: 'Exceptions',
  components: {
    EventDetailModal,
    ExceptionDetailModal,
    DatePicker
  },
  data() {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      hostname: '',
      message: '',
      total: 0,
      fields: [
        { key: 'event_time',      label: '异常时间', class: 'text-nowrap' },
        { key: 'error_code',      label: '异常编号', class: 'text-nowrap' },
        // { key: 'level', label: '级别', class: 'text-nowrap' },
        { key: 'server_hostname', label: '主机信息', tdAttr: { 'style': 'min-width: 150px;' } },
        { key: 'message',         label: '内容' },
        { key: 'button',          label: '查看详情', class: 'text-nowrap' }
      ]
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    current_app() { this.fetchData(1) },
    selected() { this.fetchData(1) }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.fetchData(1)
  },
  methods: {
    ceil: Math.ceil,
    fetchData(page) {
      const body = {
        data: {
          start_time: this.$refs.datePicker.start.valueOf(),
          end_time: this.$refs.datePicker.end.valueOf(),
          app_id: this.current_app.id,
          server_hostname: this.hostname || undefined,
          message: this.message || undefined
        },
        page: page,
        perpage: 10
      }
      return this.request.post('/v1/api/log/error/search', body)
        .then(res => {
          this.currentPage = page
          this.data = res.data
          this.total = res.total
          this.loading = false
        })
    },
    showExceptionDetail(data) {
      this.$refs.showExceptionDetail.showModal(data)
    },
  }
}
</script>

