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
              <i class="fe fe-calendar">
              </i>
            </span>
            <DatePicker ref="datePicker" v-on:selected="loadEvents(1)"></DatePicker>
          </div>
          <EventTypePicker ref="eventTypePicker" v-on:selected="loadEvents(1)"></EventTypePicker>
          <div class="input-icon ml-2">
            <span class="input-icon-addon">
              <i class="fe fe-search">
              </i>
            </span>
            <input type="text" class="form-control w-10" placeholder="攻击来源" v-model="srcip" />
          </div>
          <button class="btn btn-primary ml-2" @click="loadEvents(1)">
            搜索
          </button>
        </div>
      </div>
      <div class="card">
        <div class="card-body">

          <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }"></vue-loading>

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
                  <br />
                  {{ moment(row.event_time).format('HH:mm:ss') }}
                </td>
                <td style="max-width: 500px; ">
                  <a v-bind:href="row.url" target="_blank">
                    {{ row.url }}
                  </a>
                </td>

                <td nowrap>
                  {{ row.attack_source }}
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
                  <a href="javascript:" @click="showEventDetail(row)" target="_blank">
                    查看详情
                  </a>
                </td>
              </tr>
            </tbody>
          </table>
          <nav v-if="! loading">
            <b-pagination align="center" :total-rows="total" v-model="currentPage" :per-page="10">
            </b-pagination>
          </nav>
        </div>
      </div>
    </div>

    <eventDetailModal ref="showEventDetail"></eventDetailModal>
  </div>

</template>

<script>
import eventDetailModal from "@/components/modals/eventDetailModal"
import DatePicker from "@/components/DatePicker"
import EventTypePicker from "@/components/EventTypePicker"
import { attack_type2name, block_status2name } from '../../util'
import { mapGetters } from 'vuex'

export default {
  name: "events",
  data: function () {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      srcip: "",
      total: 0
    }
  },
  watch: {
    currentPage: function (newVal, oldVal) {
      this.loadEvents(newVal)
    },
    current_app() {
      this.loadEvents(1);
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  activated: function () {
    if (!this.loading && !this.data.length) {
      this.loading = true
      this.loadEvents(1)
    }
  },
  methods: {
    attack_type2name: attack_type2name,
    block_status2name: block_status2name,
    showEventDetail: function (data) {
      this.$refs.showEventDetail.showModal(data)
    },
    loadEvents: function (page) {
      var self = this
      var body = {
        data: {
          start_time: this.$refs.datePicker.start.unix() * 1000,
          end_time: this.$refs.datePicker.end.unix() * 1000,
          attack_type: this.$refs.eventTypePicker.selected(),
          attack_source: this.srcip,
          app_id: this.current_app.id
        },
        page: page,
        perpage: 10
      }

      this.loading = true
      this.api_request("v1/api/log/attack/search", body, function (
        data
      ) {
        self.data = data.data
        self.total = data.total
        self.loading = false
      })
    }
  },
  components: {
    eventDetailModal,
    DatePicker,
    EventTypePicker
  }
}
</script>


