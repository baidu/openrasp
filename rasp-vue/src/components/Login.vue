<template>
    <div class="container">
        <div class="row">
            <div class="col col-login mx-auto">
                <!-- <div class="text-center mb-6">
                    <img src="/static/images/openrasp.png" class="h-6" alt="">
                </div> -->
                <form class="card" @submit="doLogin()">
                    <div class="card-body p-6">
                        <div class="card-title">管理后台登录</div>
                        <div class="form-group">
                            <label class="form-label">用户名</label>
                            <input type="text" class="form-control" v-model="username">
                        </div>
                        <div class="form-group">
                            <label class="form-label">
                                密码
                                <a href="https://rasp.baidu.com/doc" target="_blank" class="float-right small">忘记密码?</a>
                            </label>
                            <input type="password" class="form-control" placeholder="输入密码" v-model="password">
                        </div>
                        <div class="form-footer">
                            <button type="submit" class="btn btn-primary btn-block" @click.prevent="doLogin()" :plain="true">登录</button>
                        </div>
                    </div>
                </form>

            </div>
        </div>
    </div>
</template>

<script>
import { mapGetters } from 'vuex'
import router from '../router'

export default {
    name: 'Login',
    data: function () {
        return {
            username: 'openrasp',
            password: 'admin@123'
        }
    },
    methods: {
        doLogin: function () {
            var self = this
            var body = {
                username: this.username,
                password: this.password
            }

            // @FIXME
            var path = self.$route.params.redirect ? self.$route.params.redirect : '#/dashboard/'
            console.log (path)

            self.api_request('v1/user/login', body, function (data) {
                location.href = path
                location.reload()

                // self.$router.replace({
                //     path: path
                // })
            })

            return false
        }
    }
}
</script>
