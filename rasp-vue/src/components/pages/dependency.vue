<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          依赖库信息
        </h1>
        <div class="page-options d-flex">
          <!--
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="fetchData(1)" />
          </div>
          -->
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
            <b-form-input v-model.trim="key_word" type="text" class="form-control" placeholder="搜索厂商或者产品" @keyup.enter="fetchData(1)" />
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
  name: 'Dependency',
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
      key_word: '',
      total: 0,
      fields: [
        { key: 'vendor',     label: '厂商',    class: 'text-nowrap' },
        { key: 'product',    label: '产品',    class: 'text-nowrap' },
        { key: 'version',    label: '版本号',   tdAttr: { 'style': 'min-width: 150px;' } },
        { key: 'rasp_count', label: '受影响主机' },
        { key: 'button',     label: '查看详情', class: 'text-nowrap' }
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
          app_id: this.current_app.id,
          hostname: this.hostname || undefined,
          key_word: this.key_word || undefined
        },
        page: page,
        perpage: 10
      }
      return this.request.post('v1/api/dependency/aggr', body)
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

