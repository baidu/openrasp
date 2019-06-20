<template>
  <div class="container">
    <div class="row" style="height: 100vh;">
      <div class="col col-login m-auto">
        <!-- <div class="text-center mb-6">
                    <img src="/static/images/openrasp.png" class="h-6" alt="">
                </div> -->
        <form class="card" @submit="doLogin()">
          <div class="card-body p-6">
            <div class="card-title">
              OpenRASP 管理后台登录
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
import { request } from '@/util'

export default {
  name: 'Login',
  data: function() {
    return {
      username: 'openrasp',
      password: ''
    }
  },
  methods: {
    doLogin: function() {
      return request.post('v1/user/login', {
        username: this.username,
        password: this.password
      }).then(res => {
        this.$router.replace({
          name: 'dashboard'
        })
      })
    }
  }
}
</script>
