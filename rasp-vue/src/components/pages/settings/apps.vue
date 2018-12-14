<template>
  <div>
    <!-- begin app settings -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          应用管理
        </h3>
      </div>
      <div class="card-body">
        <vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }" />

        <table v-if="! loading" class="table table-bordered table-hover">
          <thead>
            <th>
              名称
            </th>
            <th>
              语言
            </th>
            <th>
              备注
            </th>
            <th>
              操作
            </th>
          </thead>
          <tbody>
            <tr v-for="row in data" :key="row.id">
              <td>
                {{ row.name }}
              </td>
              <td>
                {{ row.language }}
              </td>
              <td>
                {{ row.description }}
              </td>
              <td>
                <a href="javascript:" @click="editApp(row, true)">
                  编辑
                </a>
                <a href="javascript:" @click="deleteApp(row)">
                  删除
                </a>
              </td>
            </tr>
          </tbody>
        </table>
        <nav v-if="! loading">
          <b-pagination v-model="currentPage" align="center" :total-rows="total" :per-page="10" @change="loadApps" />
        </nav>
      </div>
      <div class="card-footer text-right">
        <div class="d-flex">
          <button class="btn btn-primary" @click="editApp({})">
            添加
          </button>
        </div>
      </div>
    </div>

    <appEditModal ref="appEditModal" @save="onEdit($event)" />
    <!-- end app settings -->
  </div>
  </div\>
</template>

<script>
import appEditModal from '../../modals/appEditModal'
import { mapGetters, mapActions, mapMutations } from 'vuex'

export default {
  name: 'AppSettings',
  components: {
    appEditModal
  },
  data: function() {
    return {
      data: [],
      loading: false,
      total: 0,
      currentPage: 1
    }
  },
  watch: {
    current_app() { this.loadApps(1) }
  },
  mounted: function() {
    this.loadApps(1)
  },
  methods: {
    ...mapActions(['loadAppList']),
    loadApps(page) {
      this.loading = true
      return this.request.post('v1/api/app/get', {
        page: page,
        perpage: 10
      }).then(res => {
        this.currentPage = page
        this.data = res.data
        this.total = res.total
        this.loading = false
      })
    },
    deleteApp: function(data) {
      if (!confirm('确认操作')) {
        return
      }

      var self = this
      var body = {
        id: data.id
      }

      this.api_request('v1/api/app/delete', body, function(data) {
        self.loadApps(1)
      })
    },
    editApp: function(data, is_edit) {
      this.$refs.appEditModal.showModal({
        app_id: data.id,
        name: data.name,
        language: data.language,
        description: data.description
      }, is_edit)
    },
    onEdit: function(event) {
      console.log(event)

      var self = this
      var url = event.is_edit ? 'v1/api/app/config' : 'v1/api/app'

      this.api_request(url, event.data, function(data) {
        self.loadApps(1)
        self.loadAppList()
      })
    }
  }
}
</script>
