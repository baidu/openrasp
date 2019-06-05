<template>
  <div>
    <!-- begin auth settings -->

    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          修改登录密码
        </h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="form-label">
            原密码
          </label>
          <input v-model="oldpass" type="password" class="form-control" autocomplete="off">
        </div>
        <div class="form-group">
          <label class="form-label">
            新密码
          </label>
          <input v-model="newpass1" type="password" class="form-control" autocomplete="off">
        </div>
        <div class="form-group">
          <label class="form-label">
            再次输入新密码
          </label>
          <input v-model="newpass2" type="password" class="form-control" autocomplete="off">
        </div>
      </div>
      <div class="card-footer text-right">
        <div class="d-flex">
          <button class="btn btn-primary" @click="changePass()">
            保存
          </button>
        </div>
      </div>
    </div>

    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          TOKEN 管理 
          <a href="https://rasp.baidu.com/doc/hacking/cloud-api.html#panel-api-description" target="_blank" style="color: #467fcf">[帮助文档]</a>
        </h3>
      </div>
      <div class="card-body">
        <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

        <table v-if="! loading" class="table table-striped table-bordered">
          <thead>
            <tr>
              <th>
                Token
              </th>
              <th>
                备注
              </th>
              <th>
                操作
              </th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="row in data" :key="row.token">
              <td nowrap>
                {{ row.token }}
              </td>
              <td>
                {{ row.description }}
              </td>
              <td nowrap>
                <a href="javascript:" @click="editToken(row)">
                  编辑
                </a>
                <a href="javascript:" @click="deleteToken(row)">
                  删除
                </a>
              </td>
            </tr>
          </tbody>
        </table>
        <nav v-if="! loading">
          <b-pagination v-model="currentPage" align="center" :total-rows="total" :per-page="10" @change="loadTokens" />
        </nav>
      </div>
      <div class="card-footer text-right">
        <div class="d-flex">
          <button class="btn btn-primary" @click="createToken()">
            创建
          </button>
        </div>
      </div>
    </div>
    <!-- end auth settings -->
  </div>
</template>

<script>
export default {
  name: 'AuthSettings',
  data: function() {
    return {
      data: [],
      total: 0,
      currentPage: 1,
      loading: false,
      oldpass: '',
      newpass1: '',
      newpass2: ''
    }
  },
  mounted: function() {
    this.loadTokens(1)
  },
  methods: {
    changePass: function() {
      if (this.oldpass.length > 0 && this.newpass1.length > 0 && this.newpass1 == this.newpass2) {
        this.api_request('v1/user/update', {
          old_password: this.oldpass,
          new_password: this.newpass1
        }, function(data) {
          alert('密码修改成功，点击确认重新登录')
          location.href = '/#/login'
        })
      } else {
        alert('两次密码输入不一致，请重新输入')
      }
    },
    createToken: function() {
      var self = this
      var descr = prompt('请输入备注信息')

      if (descr && descr.length) {
        self.api_request('v1/api/token', {
          description: descr
        }, function(data) {
          self.loadTokens(1)
        })
      }
    },
    editToken: function(data) {
      var self = this
      var descr = prompt('请输入新的备注信息')
      if (!descr) { return }

      this.api_request('v1/api/token', {
        description: descr,
        token: data.token
      }, function(data) {
        self.loadTokens(1)
      })
    },
    deleteToken: function(data) {
      if (!confirm('删除 ' + data.token + ' 吗')) {
        return
      }

      var self = this
      var body = {
        token: data.token
      }

      this.api_request('v1/api/token/delete', body, function(data) {
        self.loadTokens(1)
      })
    },
    loadTokens(page) {
      this.loading = true
      return this.request.post('v1/api/token/get', {
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
