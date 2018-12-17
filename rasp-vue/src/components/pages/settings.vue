<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <b-tabs v-model="tab_index" vertical nav-wrapper-class="col-3" no-fade nav-class="list-group list-group-transparent mx-0" class="w-100" @input="onTabIndex">
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-settings" />
            </span>
            通用设置
          </template>
          <GeneralSettings ref="generalSettings" />
        </b-tab>
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-alert-triangle" />
            </span>
            报警设置
          </template>
          <AlarmSettings ref="alarmSettings" />
        </b-tab>
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-list" />
            </span>
            黑白名单
          </template>
          <WhitelistSettings ref="whitelistSettings" />
        </b-tab>
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-filter" />
            </span>
            防护设置
          </template>
          <AlgorithmSettings ref="algorithmSettings" />
        </b-tab>
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-user" />
            </span>
            登录认证
          </template>
          <AuthSettings ref="authSettings" />
        </b-tab>
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-server" />
            </span>
            应用管理
          </template>
          <AppSettings ref="appSettings" />
        </b-tab>
      </b-tabs>
    </div>
  </div>
</template>

<script>
import AlarmSettings from '@/components/pages/settings/alarm'
import AppSettings from '@/components/pages/settings/apps'
import AuthSettings from '@/components/pages/settings/auth'
import GeneralSettings from '@/components/pages/settings/general'
import WhitelistSettings from '@/components/pages/settings/whitelist'
import AlgorithmSettings from '@/components/pages/settings/algorithm'

import { mapGetters } from 'vuex'

export default {
  name: 'Settings',
  components: {
    AlarmSettings,
    WhitelistSettings,
    AppSettings,
    AuthSettings,
    AlgorithmSettings,
    GeneralSettings
  },
  data: function() {
    return {
      tab_names: ['general', 'alarm', 'whitelist', 'algorithm', 'auth', 'app'],
      tab_index: 0,
      data: {},
      loading: false
    }
  },
  computed: {
    ...mapGetters(['current_app']),
    setting_tab() { return this.$route.params.setting_tab }
  },
  watch: {
    current_app() {
      this.loadSettings()
    },
    setting_tab: 'setTabIndex'
  },
  mounted() {
    this.setTabIndex(this.setting_tab)
    if (!this.current_app.id) {
      return
    }
    this.loadSettings()
  },
  methods: {
    loadSettings: function() {
      var self = this
      var body = {
        app_id: this.current_app.id
      }

      self.loading = true

      this.api_request('v1/api/app/get', body, function(data) {
        self.loading = false

        self.$refs.generalSettings.setData(data.general_config)
      	self.$refs.whitelistSettings.setData(data.whitelist_config)
        self.$refs.alarmSettings.setData({
          ding_alarm_conf: data.ding_alarm_conf,
          http_alarm_conf: data.http_alarm_conf,
          email_alarm_conf: data.email_alarm_conf
        })
      })
    },
    setTabIndex(val) {
      this.tab_index = this.tab_names.indexOf(val) || 0
    },
    onTabIndex(val) {
      this.$router.push({
        params: {
          setting_tab: this.tab_names[val]
        }
      })
    }
  }
}
</script>

