<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div v-if="false" class="row row-cards">
        <div class="col-6 col-sm-4 col-lg-2">
          <div class="card">
            <div class="card-body p-3 text-center">
              <div class="text-right text-green">
                6%
                <i class="fe fe-chevron-up" />
              </div>
              <div class="h1 m-0">
                <span class="text-danger">
                  1
                </span>
              </div>
              <div class="text-muted mb-4">
                未处理
              </div>
            </div>
          </div>
        </div>
        <div class="col-6 col-sm-4 col-lg-2">
          <div class="card">
            <div class="card-body p-3 text-center">
              <div class="text-right text-red">
                -3%
                <i class="fe fe-chevron-down" />
              </div>
              <div class="h1 m-0">
                17
              </div>
              <div class="text-muted mb-4">
                拦截数量
              </div>
            </div>
          </div>
        </div>
        <div class="col-6 col-sm-4 col-lg-2">
          <div class="card">
            <div class="card-body p-3 text-center">
              <div class="text-right text-green">
                9%
                <i class="fe fe-chevron-up" />
              </div>
              <div class="h1 m-0">
                7
              </div>
              <div class="text-muted mb-4">
                放行数量
              </div>
            </div>
          </div>
        </div>
        <div class="col-6 col-sm-4 col-lg-2">
          <div class="card">
            <div class="card-body p-3 text-center">
              <div class="text-right text-green">
                3%
                <i class="fe fe-chevron-up" />
              </div>
              <div class="h1 m-0">
                1
              </div>
              <div class="text-muted mb-4">
                在线主机
              </div>
            </div>
          </div>
        </div>
        <div class="col-6 col-sm-4 col-lg-2">
          <div class="card">
            <div class="card-body p-3 text-center">
              <div class="text-right text-red">
                -2%
                <i class="fe fe-chevron-down" />
              </div>
              <div class="h1 m-0">
                3
              </div>
              <div class="text-muted mb-4">
                失联主机
              </div>
            </div>
          </div>
        </div>
        <div class="col-6 col-sm-4 col-lg-2">
          <div class="card">
            <div class="card-body p-3 text-center">
              <div class="text-right text-red">
                -1%
                <i class="fe fe-chevron-down" />
              </div>
              <div class="h1 m-0">
                10d
              </div>
              <div class="text-muted mb-4">
                运行时间
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="row row-cards">
        <div class="col-lg-6">
          <div class="card">
            <div class="card-header">
              <h3 class="card-title">
                攻击趋势
              </h3>
            </div>
            <EventTrend ref="event_trend" />
          </div>
        </div>
        <div class="col-md-6">
          <div class="row">
            <div class="col-sm-6">
              <div class="card">
                <div class="card-header">
                  <h3 class="card-title">
                    TOP 10 攻击UA
                  </h3>
                </div>
                <div class="card-body">
                  <TopAttackUa ref="top_attack_ua" />
                </div>
              </div>
            </div>
            <div class="col-sm-6">
              <div class="card">
                <div class="card-header">
                  <h3 class="card-title">
                    TOP 10 攻击类型
                  </h3>
                </div>
                <div class="card-body">
                  <TopAttackType ref="top_attack_type" />
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'

import TopAttackType from './charts/top_attack_type'
import TopAttackUa from './charts/top_attack_ua'
import EventTrend from './charts/event_trend'

export default {
  name: 'Dashboard',
  components: {
    TopAttackType,
    TopAttackUa,
    EventTrend
  },
  data: function() {
    return {}
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  watch: {
    current_app() { this.loadChartData() }
  },
  mounted() {
    if (!this.current_app.id) {
      return
    }
    this.loadChartData()
  },
  methods: {
    loadChartData: function() {
      var self = this
      var body = {
        app_id: this.current_app.id,
        size: 10,
        start_time: this.moment().subtract(1, 'months').unix() * 1000,
        end_time: this.moment().unix() * 1000,
        interval: 'day',
        time_zone: '+08:00'
      }

      this.api_request('v1/api/log/attack/aggr/type', body, function(data) {
        self.$refs.top_attack_type.setData(data)
      })

      this.api_request('v1/api/log/attack/aggr/ua', body, function(data) {
        self.$refs.top_attack_ua.setData(data)
      })

      this.api_request('v1/api/log/attack/aggr/time', body, function(data) {
        self.$refs.event_trend.setData(data)
      })
    }
  }
}
</script>

