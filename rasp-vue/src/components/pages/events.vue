<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          攻击事件
        </h1>
        <div class="page-options d-flex">
          <div class="input-icon ml-2 w-50">
            <span class="input-icon-addon">
              <i class="fe fe-calendar" />
            </span>
            <DatePicker ref="datePicker" @selected="loadEvents(1)" />
          </div>
          <EventTypePicker ref="eventTypePicker" @selected="loadEvents(1)" />
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search" />
            </span>
            <input v-model="srcip" type="text" class="form-control w-10" placeholder="攻击来源" @keyup.enter="loadEvents(1)">
          </div>
          <button class="btn btn-primary ml-2" @click="loadEvents(1)">
            搜索
          </button>
        </div>
      </div>
      <div class="card">
        <div class="card-body">
          <VueLoading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

          <table v-if="! loading" class="table table-striped table-bordered">
            <thead>
              <tr>
                <th>
                  攻击时间
                </th>
                <th>
                  URL
                </th>
                <th>
                  攻击来源
                </th>
                <th>
                  拦截状态
                </th>
                <th nowrap>
                  攻击类型
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
                <td style="max-width: 500px; ">
                  <a :href="row.url" target="_blank">
                    {{ row.url }}
                  </a>
                </td>

                <td nowrap>
                  <a target="_blank" :href="'https://www.virustotal.com/#/ip-address/' + (row.client_ip ? row.client_ip : row.attack_source)">
                    {{ row.client_ip ? row.client_ip : row.attack_source }}
                  </a>
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
          <nav v-if="! loading">
            <b-pagination v-model="currentPage" align="center" :total-rows="total" :per-page="10" @change="loadEvents($event)" />
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
import EventTypePicker from '@/components/EventTypePicker'
import { attack_type2name, block_status2name } from '../../util'
import { mapGetters } from 'vuex'

export default {
  name: 'Events',
  components: {
    EventDetailModal,
    DatePicker,
    EventTypePicker
  },
  data() {
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
  watch: {
    current_app() { this.loadEvents(1) }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.loadEvents(1)
  },
  methods: {
    attack_type2name,
    block_status2name,
    showEventDetail(data) {
      this.$refs.showEventDetail.showModal(data)
    },
    loadEvents(page) {
      this.loading = true
      return this.request.post('v1/api/log/attack/search', {
        data: {
          start_time: this.$refs.datePicker.start.valueOf(),
          end_time: this.$refs.datePicker.end.valueOf(),
          attack_type: this.$refs.eventTypePicker.selected(),
          attack_source: this.srcip,
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

