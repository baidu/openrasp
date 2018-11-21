<template>
  <div id="settings-whitelist" class="tab-pane fade">
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          防护引擎白名单
        </h3>
      </div>
      <div class="card-body">
        <p>最多允许200个URL，单条URL长度限制为200字符</p>
        <table class="table table-bordered table-hover">
          <thead>
            <th nowrap>
              #
            </th>
            <th>
              URL
            </th>
            <th>
              检测点
            </th>
            <th>
              操作
            </th>
          </thead>
          <tbody>
            <tr v-for="(row, index) in data" :key="index">
              <td nowrap>
                {{ index + 1 }}
              </td>
              <td>
                {{ row.url }}
              </td>
              <td>
                <span v-if="row.hook['all']">
                  所有 Hook 点
                </span>
                <span v-if="! row.hook['all']">
                  {{ whitelist2str(row.hook).join(', ') }}
                </span>
              </td>
              <td nowrap>
                <a href="javascript:" @click="showModal(row, index)">
                  编辑
                </a>
                <a href="javascript:" @click="deleteItem(index)">
                  删除
                </a>
              </td>
            </tr>
          </tbody>
        </table>
      </div>
      <div class="card-footer">
        <button class="btn btn-info" @click="showModal({hook: {}})">
          添加
        </button>
        <button class="btn btn-primary pull-right" @click="doSave()">
          保存
        </button>
      </div>
    </div>

    <whitelistEditModal ref="whitelistEditModal" v-on:save="onEdit($event)"></whitelistEditModal>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'
import whitelistEditModal from '../../modals/whitelistEditModal'
import { attack_type2name } from '../../../util/'

export default {
  name: 'WhitelistSettings',
  data: function () {
    return {
      data: [],
      whitelist_all: false
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  methods: {
    attack_type2name: attack_type2name,
    whitelist2str (row) {
      var tmp = []
      Object.keys(row).forEach (function (key) {
        if (row[key]) {
          tmp.push(attack_type2name(key))
        }
      })

      return tmp
    },
    setData: function (data) {
      this.data = data
    },
    onEdit: function (event) {
      if (event.index == undefined) {
        this.data.push(event.data)
      } else {
        this.data.splice(event.index, 1, event.data)
      }
    },
    showModal: function (data, index) {
      if (index == undefined && self.data.length >= 200) {
        alert('为了保证性能，白名单最多支持 200 条')
        return
      }

      this.$refs.whitelistEditModal.showModal(data, index)
    },
    deleteItem: function (index) {
      if (! confirm ('确认删除'))
        return     

      this.data.splice(index, 1)
    },
    doSave: function () {
      var self = this
      var body = {
        app_id: this.current_app.id,
        config: this.data
      }

      this.api_request('v1/api/app/whitelist/config', body, function (data) {
        alert ('保存成功')
      })
    }
  },
  components: {
    whitelistEditModal
  }
}
</script>
