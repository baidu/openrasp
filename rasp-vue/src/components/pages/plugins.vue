<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">
          插件管理
        </h1>
        <div class="page-options d-flex">
          <FileUpload ref="fileUpload"></FileUpload>
        </div>
        <button class="btn btn-primary ml-2" @click="doUpload()">提交</button>
        <button class="btn btn-info ml-2" @click="loadPluginList(1)">刷新</button>
      </div>
      <div class="card">
        <div class="card-body">
          <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }"></vue-loading>

          <table class="table table-bordered" v-if="! loading">
            <thead>
              <tr>
                <th>上传时间</th>
                <th>插件版本</th>
                <th>当前版本</th>
                <th>操作</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="row in data" :key="row.id">
                <td>{{ moment(row.upload_time).format('YYYY-MM-DD hh:mm:ss') }}</td>
                <td>official: {{ row.version }}</td>
                <td>
                  <span v-if="current_app.selected_plugin_id == row.id">是</span>
                </td>
                <td>
                  <a href="javascript:" @click="doSelect(row)">推送</a> &nbsp;
                  <a href="javascript:" @click="doDownload(row)">下载</a> &nbsp;
                  <a href="javascript:" @click="doDelete(row)">删除</a>
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
  </div>

</template>

<script>
import axios from 'axios'
import FileUpload from "@/components/FileUpload"
import { mapGetters, mapActions, mapMutations } from "vuex"

export default {
  name: "plugins",
  data: function() {
    return {
      currentPage: 1,
      total: 1,
      data: [],
      loading: false
    }
  },
  watch: {
    currentPage: function(newVal, oldVal) {
      this.loadPluginList(newVal)
    },
    current_app() {
      this.loadPluginList(1)
    }
  },
  computed: {
    ...mapGetters(["current_app"])
  },
  activated: function() {
    if (this.current_app.id && !this.loading && !this.data.length) {
      this.loadPluginList(1)
    }
  },
  methods: {
    ...mapActions(["loadAppList"]),
    loadPluginList: function(page) {
      var self = this
      var data = {
        page: page,
        perpage: 10,
        app_id: this.current_app.id
      }

      this.loading = true
      this.api_request("v1/api/app/plugin/get", data, function(data) {
        self.data = data.data
        self.total = data.total

        self.loading = false
      })
    },
    doDownload: function(row) {
      var self = this
      var body = {
        id: row.id
      }

      axios({
        url: "/v1/api/plugin/download",
        method: "POST",
        data: JSON.stringify(body),
        responseType: "blob"
      }).then(response => {
        const url = window.URL.createObjectURL(new Blob([response.data]))
        const link = document.createElement("a")
        link.href = url
        link.setAttribute("download", "plugin.js") //or any other extension
        document.body.appendChild(link)
        link.click()
      })
    },
    doUpload: function() {
      var self = this
      var file = this.$refs.fileUpload.file

      if (file) {
        var data = new FormData()
        data.append("plugin", file)

        this.api_request(
          "v1/api/plugin?app_id=" + self.current_app.id,
          data,
          function(data) {
            self.loadPluginList(1)
            self.$refs.fileUpload.clear()
          }
        )
      }
    },
    doDelete: function(row) {
      var self = this
      var body = {
        id: row.id
      }

      if (!confirm("确认删除?")) {
        return
      }

      this.api_request("v1/api/plugin/delete", body, function(data) {
        self.loadPluginList(1)
      })
    },
    doSelect: function(row) {
      if (!confirm("确认下发?")) {
        return
      }

      var self = this
      var body = {
        app_id: this.current_app.id,
        plugin_id: row.id
      }

      this.api_request("v1/api/app/plugin/select", body, function(data) {
        self.loadAppList()
      })
    }
  },
  components: {
    FileUpload
  }
}
</script>

