<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          漏洞列表
        </h1>
        <div class="page-options d-flex">
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="loadEvents(1)" />
          </div>
          <b-dropdown text="拦截状态" class="ml-2" right>
            <b-container style="width: 250px;">
              <b-form-row>
                <template v-for="(value, key) in status_types">
                  <b-col :key="key">
                    <label class="custom-switch">
                      <input v-model="selected_status" type="checkbox" class="custom-switch-input" :value="key">
                      <span class="custom-switch-indicator" />
                      <span class="custom-switch-description">
                        {{ value }}
                      </span>
                    </label>
                  </b-col>
                </template>
              </b-form-row>
            </b-container>
          </b-dropdown>           
          <b-dropdown text="漏洞类型" class="ml-2" right>
            <b-container style="width: 500px;">
              <b-form-row>
                <b-col>
                  <label class="custom-switch">
                    <input type="checkbox" class="custom-switch-input" checked @change="selectAll">
                    <span class="custom-switch-indicator" />
                    <span class="custom-switch-description">
                      全选
                    </span>
                  </label>
                </b-col>
                <div class="w-100" />
                <template v-for="(value, key, index) in attack_types">
                  <b-col :key="key">
                    <label class="custom-switch">
                      <input v-model="selected" type="checkbox" class="custom-switch-input" :value="key">
                      <span class="custom-switch-indicator" />
                      <span class="custom-switch-description">
                        {{ value }}
                      </span>
                    </label>
                  </b-col>
                  <div v-if="index % 2 === 1" class="w-100" />
                </template>
              </b-form-row>
            </b-container>
          </b-dropdown>
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <input v-model.trim="url" type="text" class="form-control w-10" placeholder="目标URL" @keyup.enter="loadEvents(1)">
          </div>
          <button class="btn btn-primary ml-2" @click="loadEvents(1)">
            搜索
          </button>
        </div>
      </div>
      <div class="card">
        <div class="card-body">
          <VueLoading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

          <nav v-if="! loading && total > 0">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block; ">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} / {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="loadEvents($event)" />
          </nav>

          <table v-if="! loading" class="table table-striped table-bordered">
            <thead>
              <tr>
                <th>
                  最后发现
                </th>
                <th style="min-width: 150px; ">
                  URL
                </th>
                <th>
                  攻击来源
                </th>
                <th>
                  最后状态
                </th>
                <th nowrap>
                  漏洞类型
                </th>
                <th>
                  报警消息
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
                <td style="max-width: 500px; word-break: break-word; ">
                  <a :href="row.url" target="_blank">
                    {{ displayURL(row) }}
                  </a>
                </td>

                <td nowrap>
                  <div v-if="row.attack_source">
                    <a target="_blank" :href="'https://www.virustotal.com/#/ip-address/' + (row.client_ip ? row.client_ip : row.attack_source)">
                      {{ row.client_ip ? row.client_ip : row.attack_source }}                  
                    </a>                    
                    <br/>
                  </div>
                  利用 {{ row.attack_count }} 次
                </td>
                <td nowrap>
                  {{ block_status2name(row.intercept_state) }}
                </td>
                <td nowrap>
                  <span class="tag tag-danger">
                    {{ attack_type2name(row.attack_type) }}
                  </span>
                </td>
                <td>
                  {{ row.plugin_message }}
                </td>
                <td nowrap>
                  <a href="javascript:" @click="showEventDetail(row)">
                    查看详情
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
            <b-pagination v-model="currentPage" align="right" :total-rows="total" :per-page="10" @change="loadEvents($event)" />
          </nav>

        </div>
      </div>
    </div>

    <EventDetailModal ref="showEventDetail" />
  </div>
</template>

<script>
import EventDetailModal from '@/components/modals/eventDetailModal'
import DatePicker from '@/components/DatePicker'
import { attack_type2name, block_status2name, attack_types, status_types } from '@/util'
import { mapGetters } from 'vuex'

export default {
  name: 'vulns',
  components: {
    EventDetailModal,
    DatePicker
  },
  data() {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      url: '',
      total: 0,
      attack_types,
      status_types,
      selected_status: Object.keys(status_types),
      selected: Object.keys(attack_types)
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    current_app() { this.loadEvents(1) },
    selected() { this.loadEvents(1) },
    selected_status() { this.loadEvents(1) }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.loadEvents(1)
  },
  methods: {
    ceil: Math.ceil,
    displayURL(row) {
      if (! row.url) {
        return '-'
      }

      if (row.url.length > 100) {
        return row.url.substring(0, 100) + ' ...'
      }

      return row.url
    },
    selectAllStatus({ target }) {
      this.selected_status = target.checked ? Object.keys(this.status_types) : []
    },    
    selectAll({ target }) {
      this.selected = target.checked ? Object.keys(this.attack_types) : []
    },
    attack_type2name,
    block_status2name,
    showEventDetail(data) {
      this.$refs.showEventDetail.showModal(data)
    },
    loadEvents(page) {
      this.loading = true
      return this.request.post('v1/api/log/attack/aggr/vuln', {
        data: {
          start_time: this.$refs.datePicker.start.valueOf(),
          end_time: this.$refs.datePicker.end.valueOf(),
          attack_type: this.selected,
          intercept_state: this.selected_status,
          url: this.url,
          app_id: this.current_app.id
        },
        page: page,
        perpage: 10
      }).then(res => {
        this.currentPage = page
        this.data = res.data
        this.total = res.total
        this.loading = false
      })
    }
  }
}
</script>

