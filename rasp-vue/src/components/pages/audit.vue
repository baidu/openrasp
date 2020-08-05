<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          操作审计
        </h1>

        <div class="page-options d-flex">
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="loadAudit(1)" />
          </div>
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <input v-model.trim="ip" type="text" class="form-control w-10" placeholder="搜索IP" @keyup.enter="loadAudit(1)">
          </div>
          <button class="btn btn-primary ml-2" @click="loadAudit(1)">
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
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="loadAudit($event)" />
          </nav>

          <table v-if="! loading" class="table table-bordered">
            <thead>
              <tr>
                <th>操作时间</th>
                <th>操作类型</th>
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
                <td nowrap>
{{ audit_types[row.type_id] }}
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

          <p v-if="! loading && total == 0" class="text-center">
暂无数据
</p>

          <nav v-if="! loading && total > 10">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block; ">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} / {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="loadAudit($event)" />
          </nav>
</div>
      </div>
    </div>
  </div>
</template>

<script>
import { audit_types } from '@/util'
import DatePicker from '@/components/DatePicker'
import { mapGetters } from 'vuex'

export default {
  name: 'Audit',
  data: function() {
    return {
      audit_types,
      data: [],
      loading: false,
      currentPage: 1,
      ip: '',
      total: 0
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    current_app() { this.loadAudit(1) }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.loadAudit(1)
  },
  methods: {
    ceil: Math.ceil,
    loadAudit(page) {
      this.loading = true
      return this.request.post('v1/api/operation/search', {
        page: page,
        perpage: 10,
        start_time: this.$refs.datePicker.start.valueOf(),
        end_time: this.$refs.datePicker.end.valueOf(),
        data: {
          app_id: this.current_app.id,
          ip: this.ip
        }
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

