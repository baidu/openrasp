<template>
  <div class="container">
    <div class="row" style="height: 100vh;">
      <div class="col col-login m-auto">
        <div class="text-center mb-6" v-if="logo">
          <img :src="logo" class="h-6" alt="">
        </div>
        <form class="card" @submit="doLogin()">
          <div class="card-body p-6">
            <div class="card-title text-center">
              {{ title }}
            </div>
            <div class="form-group">
              <label class="form-label">
                用户名
              </label>
              <input v-model.trim="username" type="text" class="form-control">
            </div>
            <div class="form-group">
              <label class="form-label">
                密码
                <a href="https://rasp.baidu.com/doc/install/panel.html#forget-password" target="_blank" class="float-right small">
                  忘记密码?
                </a>
              </label>
              <input v-model="password" type="password" class="form-control" placeholder="输入密码" autocomplete="off">
            </div>
            <div class="form-footer">
              <button type="submit" class="btn btn-primary btn-block" :plain="true" @click.prevent="doLogin()">
                登录
              </button>
            </div>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script>
import axios from 'axios'
import { request } from '@/util'

export default {
  name: 'Login',
  data: function() {
    return {
      username: 'openrasp',
      password: '',
      title:    'OpenRASP 管理后台登录',
      logo:     ''
    }
  },
  mounted: function() {
    this.updateLogo()
  },
  methods: {
    updateLogo: function() {
      var self = this
      axios.get('static/frontend.json').then(function (res) {
        try {
          var data = res.data
          if (data.enable) {
            self.title = data.title
            self.logo  = data.logo
          }
        } catch (e) {
          console.log(e)
        }
      })
    },
    doLogin: function() {
      return request.post('v1/user/login', {
        username: this.username,
        password: this.password
      }).then(res => {
        if (this.$route.query.redirect) {
          location.href = this.$route.query.redirect
        } else {
          this.$router.replace({
            name: 'dashboard'
          })
        }
      })
    }
  }
}
</script>
