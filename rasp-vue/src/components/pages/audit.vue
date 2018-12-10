<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          操作审计
        </h1>

        <div class="page-options d-flex">
          <div class="input-icon ml-2 w-100">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="loadAudit(1)" />
          </div>
          <button class="btn btn-primary ml-2" @click="loadAudit(1)">
            搜索
          </button>
        </div>
      </div>
      <div class="card">
        <div class="card-body">
          <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

          <table v-if="! loading" class="table table-bordered">
            <thead>
              <tr>
                <th>操作时间</th>
                <th>操作内容</th>
                <th>操作人</th>
                <th>IP 地址</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="row in data" :key="row.id">
                <td nowrap>
                  {{ moment(row.time).format('YYYY-MM-DD HH:mm:ss') }}
                </td>
                <td>{{ row.content }}</td>
                <td nowrap>
                  {{ row.user.length ? row.user : '-' }}
                </td>
                <td nowrap>
                  {{ row.ip }}
                </td>
              </tr>
            </tbody>
          </table>
          <nav v-if="! loading">
            <b-pagination v-model="currentPage" align="center" :total-rows="total" :per-page="10" @change="loadAudit($event)" />
          </nav>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import DatePicker from '@/components/DatePicker'
import { mapGetters } from 'vuex'

export default {
  name: 'Audit',
  data: function() {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      srcip: '',
      total: 0
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  mounted() {
    this.loadAudit(1)
  },
  methods: {
    loadAudit(page) {
      this.loading = true
      return this.request.post('v1/api/operation/search', {
        page: page,
        perpage: 10,
        start_time: this.$refs.datePicker.start.valueOf(),
        end_time: this.$refs.datePicker.end.valueOf(),
        data: {}
      }).then(res => {
        this.currentPage = page
        this.data = res.data
        this.total = res.total
        this.loading = false
      })
    }
  },
  components: {
    DatePicker
  }
}
</script>

