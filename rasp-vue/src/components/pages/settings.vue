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
          <AlarmSettings ref="alarmSettings" :data="data" />
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
              <i class="fe fe-shield" />
            </span>
            应用加固
          </template>
          <HardeningSettings ref="hardeningSettings" />
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
        <b-tab title-link-class="list-group-item border-0 w-100" title-item-class="px-0">
          <template slot="title">
            <span class="icon mr-3">
              <i class="fe fe-settings" />
            </span>
            后台设置
          </template>
          <PanelSettings ref="panelSettings" />
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
import HardeningSettings from '@/components/pages/settings/hardening'
import PanelSettings from '@/components/pages/settings/panel'
import { mapGetters } from 'vuex'
import defaultsDeep from 'lodash.defaultsdeep'
import { getDefaultConfig } from '@/util'

export default {
  name: 'Settings',
  components: {
    AlarmSettings,
    WhitelistSettings,
    AppSettings,
    AuthSettings,
    AlgorithmSettings,
    GeneralSettings,
    HardeningSettings,
    PanelSettings
  },
  data: function() {
    return {
      tab_names: ['general', 'alarm', 'whitelist', 'algorithm', 'hardening', 'auth', 'app', 'panel'],
      tab_index: 0,
      data: undefined,
      loading: false
    }
  },
  computed: {
    ...mapGetters(['current_app']),
    setting_tab() { return this.$route.params.setting_tab }
  },
  watch: {
    current_app() { this.loadSettings() },
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
      this.loading = true
      this.request.post('v1/api/app/get', {
        app_id: this.current_app.id
      }).then(data => {
        this.loading = false
        this.data = defaultsDeep(getDefaultConfig(), data)
        this.$refs.generalSettings.setData(this.data.general_config)
        this.$refs.hardeningSettings.setData(this.data.general_config)
        this.$refs.whitelistSettings.setData(this.data.whitelist_config)
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

