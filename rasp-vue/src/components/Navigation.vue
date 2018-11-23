<template>
  <div>
    <div class="header py-4">
      <div class="container">
        <div class="d-flex">
          <div class="dropdown">
            <a href="javascript:" class="nav-link pr-0 leading-none" data-toggle="dropdown" style="padding-left: 0">
              <span class="avatar" v-bind:style="{ 'background-image': 'url(/static/images/lang/' + current_app.language + '.png)' }">
              </span>
              <span class="ml-2 d-none d-lg-block">
                <span class="text-muted" style="margin-left: -2px; ">
                  当前应用
                  <i class="fa fa-caret-down">
                  </i>
                </span>
                <small class="text-default d-block mt-1">
                  {{ current_app.name }}
                </small>
              </span>
            </a>
            <div class="dropdown-menu dropdown-menu-left dropdown-menu-arrow">
              <a :key="row.id" class="dropdown-item" href="javascript:" v-for="row in app_list" @click.prevent="changeApp(row)">
                <i class="dropdown-icon fe fe-lock">
                </i>
                {{ row.name }} ( {{ row.lang }} {{ row.id }} )
              </a>
              <div class="dropdown-divider">
              </div>
              <a class="dropdown-item" href="javascript:">
                <i class="dropdown-icon fe fe-settings">
                </i>
                应用管理
              </a>
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
                  <span class="text-default">admin <i class="fa fa-caret-down"></i></span>
                  <small class="text-muted d-block mt-1">管理员权限</small>
                </span>
              </a>
              <div class="dropdown-menu dropdown-menu-right dropdown-menu-arrow" x-placement="bottom-end">
                <a class="dropdown-item" href="javascript:">
                  <i class="dropdown-icon fe fe-settings"></i> 用户设置
                </a>
                <div class="dropdown-divider"></div>
                <a class="dropdown-item" href="javascript:" @click="doLogout()">
                  <i class="dropdown-icon fe fe-log-out"></i> 退出登录
                </a>
              </div>
            </div>
          </div>
          <a href="#" class="header-toggler d-lg-none ml-3 ml-lg-0" data-toggle="collapse" data-target="#headerMenuCollapse">
            <span class="header-toggler-icon">
            </span>
          </a>
        </div>
      </div>
    </div>
    <div class="header collapse d-lg-flex p-0" id="headerMenuCollapse">
      <div class="container">
        <div class="row align-items-center">
          <div class="col-lg order-lg-first">
            <ul class="nav nav-tabs border-0 flex-column flex-lg-row" v-if="current_app.id">
              <li class="nav-item">
                <router-link :to="{ name: 'dashboard', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-home">
                  </i>
                  安全总览
                </router-link>
              </li>
              <li class="nav-item">
                <router-link :to="{ name: 'events', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-bell">
                  </i>
                  攻击事件
                </router-link>
              </li>
              <li class="nav-item dropdown">
                <router-link :to="{ name: 'baseline', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-check-square">
                  </i>
                  安全基线
                </router-link>
              </li>
              <li class="nav-item">
                <router-link :to="{ name: 'hosts', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-cloud">
                  </i>
                  主机管理
                </router-link>
              </li>
              <li class="nav-item">
                <router-link :to="{ name: 'plugins', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-zap">
                  </i>
                  插件管理
                </router-link>
              </li>
              <li class="nav-item">
                <router-link :to="{ name: 'audit', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-user-check">
                  </i>
                  操作日志
                </router-link>
              </li>
              <li class="nav-item dropdown">
                <router-link :to="{ name: 'settings', params: { app_id: current_app.id } }" class="nav-link">
                  <i class="fe fe-settings">
                  </i>
                  系统设置
                </router-link>
              </li>
              <li class="nav-item">
                <a href="https://rasp.baidu.com/doc" target="_blank" class="nav-link">
                  <i class="fe fe-file-text">
                  </i>
                  帮助文档
                </a>
              </li>
              <li class="nav-item">
                <a href="https://rasp.baidu.com/#section-support" target="_blank" class="nav-link">
                  <i class="fa fa-qq">
                  </i>
                  技术支持
                </a>
              </li>
            </ul>
          </div>
        </div>
      </div>
    </div>

    <addHostModal ref="addHost"></addHostModal>
  </div>

</template>
<script>
import addHostModal from "@/components/modals/addHostModal.vue"
import { mapGetters, mapActions, mapMutations } from "vuex"

export default {
  name: "Navigation",
  data: function() {
    return {
      urlHasId: false
    }
  },
  computed: {
    ...mapGetters(["current_app", "app_list"])
  },
  methods: {
    ...mapActions(["loadAppList"]),
    ...mapMutations(["setCurrentApp", "setAuthStatus"]),
    showAddHostModal() {
      this.$refs.addHost.showModal()
    },
    changeApp(data) {
      this.setCurrentApp(data)
    },
    doLogout() {
      var self = this
      self.api_request('v1/user/logout', {}, function (data) {
        self.setAuthStatus(false)
      })
    }
  },
  mounted() {
    if (this.$route.params.app_id) {
      this.loadAppList(this.$route.params.app_id)
    } else {
      this.loadAppList()
    }
  },
  watch: {
    current_app(newValue) {
      var name = this.$route.name
      if (!name) {
        name = "dashboard"
      }

      this.$router.push({
        name: name,
        params: {
          app_id: newValue.id
        }
      })
    }
  },
  components: {
    addHostModal
  }
}
</script>
