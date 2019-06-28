<template>
  <div>
    <div class="header py-4">
      <div class="container">
        <div class="d-flex">
          <div class="dropdown">
            <a href="javascript:" class="nav-link pr-0 leading-none" data-toggle="dropdown" style="padding-left: 0">
              <span class="avatar" :style="{ 'background-image': current_app.language ? 'url(/static/images/lang/' + current_app.language + '.png)' : undefined }" />
              <span class="ml-2 d-none d-lg-block">
                <span class="text-muted" style="margin-left: -2px; ">
                  当前应用
                  <i class="fa fa-caret-down" />
                </span>
                <small class="text-default d-block mt-1">
                  {{ current_app.name }}
                </small>
              </span>
            </a>
            <div class="dropdown-menu dropdown-menu-left dropdown-menu-arrow">
              <div class="form-group" style="margin: 6px 15px 10px 15px; ">
                <input v-model.trim="keyword" type="text" class="form-control form-control-sm">
              </div>
              <div style="max-height: 300px; overflow: scroll; ">
                <a v-for="row in app_list_filtered" :key="row.id" class="dropdown-item" href="javascript:" @click.prevent="setCurrentApp(row)">
                  <i class="dropdown-icon fe fe-lock" />
                  {{ row.name }}
                </a>
              </div>
              <div class="dropdown-divider" />
              <router-link :class="{'dropdown-item': true, 'active': false}" :to="{name: 'settings', params: {setting_tab: 'app'}}">
                <i class="dropdown-icon fe fe-settings" />
                应用管理 ({{ app_list.length }})
              </router-link>
            </div>
          </div>
          <div class="d-flex order-lg-2 ml-auto">
            <div class="nav-item d-none d-md-flex">
              <a href="javascript:" class="btn btn-sm btn-outline-primary" @click="showAddHostModal">
                添加主机
              </a>
            </div>
            <div class="dropdown">
              <a href="javascript:" class="nav-link pr-0 leading-none" data-toggle="dropdown">
                <span class="ml-2 d-none d-lg-block">
                  <span class="text-default">
                    openrasp <i class="fa fa-caret-down" />
                  </span>
                  <small class="text-muted d-block mt-1">
                    管理员权限
                  </small>
                </span>
              </a>
              <div class="dropdown-menu dropdown-menu-right dropdown-menu-arrow" x-placement="bottom-end">
                <!-- <a class="dropdown-item" href="javascript:">
                  <i class="dropdown-icon fe fe-settings"></i> 用户设置
                </a>
                <div class="dropdown-divider"></div> -->
                <RouterLink class="dropdown-item" :to="{ name: 'audit', params: { app_id: current_app.id } }">
                  <i class="dropdown-icon fe fe-user-check" />
                  操作审计
                </RouterLink>
                <a class="dropdown-item" href="https://rasp.baidu.com/#section-support" target="_blank">
                  <i class="dropdown-icon fa fa-qq" /> 技术支持
                </a>
                <div class="dropdown-divider"/>
                <a class="dropdown-item" href="javascript:" @click="doLogout()">
                  <i class="dropdown-icon fe fe-log-out" /> 退出登录
                </a>
              </div>
            </div>
          </div>
          <a href="#" class="header-toggler d-lg-none ml-3 ml-lg-0" data-toggle="collapse" data-target="#headerMenuCollapse">
            <span class="header-toggler-icon" />
          </a>
        </div>
      </div>
    </div>
    <div id="headerMenuCollapse" class="header collapse d-lg-flex p-0">
      <div class="container">
        <div class="row align-items-center">
          <div class="col-lg order-lg-first">
            <ul v-if="current_app.id" class="nav nav-tabs border-0 flex-column flex-lg-row">
              <li class="nav-item">
                <RouterLink :to="{ name: 'dashboard', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-home" />
                  安全总览
                </RouterLink>
              </li>
              <li class="nav-item">
                <RouterLink :to="{ name: 'vulns', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-eye" />
                  漏洞列表
                </RouterLink>
              </li>
              <li class="nav-item">
                <RouterLink :to="{ name: 'events', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-bell" />
                  攻击事件
                </RouterLink>
              </li>
              <li class="nav-item dropdown">
                <RouterLink :to="{ name: 'baseline', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-check-square" />
                  安全基线
                </RouterLink>
              </li>
              <li class="nav-item">
                <RouterLink :to="{ name: 'hosts', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-cloud" />
                  主机管理
                </RouterLink>
              </li>
              <li class="nav-item">
                <RouterLink :to="{ name: 'plugins', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-zap" />
                  插件管理
                </RouterLink>
              </li>
              <li class="nav-item">
                <RouterLink :to="{ name: 'exceptions', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-alert-circle" />
                  异常日志
                </RouterLink>
              </li>
              <li class="nav-item dropdown">
                <RouterLink :to="{ name: 'settings', params: { setting_tab: 'general', app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-settings" />
                  系统设置
                </RouterLink>
              </li>
              <li class="nav-item">
                <a href="https://rasp.baidu.com/doc" target="_blank" class="nav-link">
                  <i class="fe fe-file-text" />
                  帮助文档
                </a>
              </li>
              <!-- <li class="nav-item">
                <a href="https://rasp.baidu.com/#section-support" target="_blank" class="nav-link">
                  <i class="fa fa-qq" />
                  技术支持
                </a>
              </li> -->
            </ul>
          </div>
        </div>
      </div>
    </div>

    <div v-if="no_plugin" class="alert alert-warning">
      <div class="container">
        <strong>注意!</strong> 当前应用没有配置任何检测插件，请前往 <router-link :to="{name: 'plugins'}">
插件页面
</router-link> 进行配置
      </div>
    </div>

    <div v-if="all_log" class="alert alert-warning">
      <div class="container">
        当前以「记录日志」模式运行，可前往 <router-link :to="{name: 'settings', params: {setting_tab: 'algorithm'}}">
防护设置
</router-link> 关闭
      </div>
    </div>

    <AddHostModal ref="addHost" />
  </div>
</template>
<script>
import AddHostModal from '@/components/modals/addHostModal.vue'
import { mapGetters, mapActions, mapMutations } from 'vuex'
import Cookie from 'js-cookie'

export default {
  name: 'Navigation',
  components: {
    AddHostModal
  },
  data: function() {
    return {
      keyword: '',
      no_plugin: false,
      all_log: false
    }
  },
  computed: {
    ...mapGetters(['current_app', 'app_list', 'sticky']),
    app_list_filtered: function() {
      var keyword = this.keyword.toLowerCase()
      return this.app_list.filter(function(app) {
        return app.name.toLowerCase().indexOf(keyword) != -1
      })
    }
  },
  watch: {
    current_app(app) {
      this.no_plugin = !this.current_app.selected_plugin_id || !this.current_app.selected_plugin_id.length
      this.all_log = false

      // 检查是否开启日志模式
      if (!this.no_plugin) {
        this.all_log = this.current_app.algorithm_config.meta.all_log
      }

      this.$router.push({
        name: this.$route.name,
        params: {
          app_id: app.id
        }
      })
    }
  },
  methods: {
    ...mapActions(['loadAppList']),
    ...mapMutations(['setCurrentApp']),
    ...mapMutations(['setSticky']),
    showAddHostModal() {
      this.$refs.addHost.showModal()
    },
    doLogout() {
      return this.request.post('v1/user/logout', {})
        .then(res => {
          Cookie.set('RASP_AUTH_ID', null)
          location.href = '/#/login'
        })
    }
  },
  mounted: function() {
    this.request.post('v1/api/server/url/get', {}).then(res => {
      if (!res.panel_url) {
        console.log('panel_url not set, initializing')
        var current_url = location.href.split(/\?|#/)[0]

        this.request.post('v1/api/server/url', {
          panel_url: current_url,
          agent_urls: [current_url]
        }).then(res => {})
      } else {
        console.log('panel_url already configured')
      }
    })
  }
}
</script>
