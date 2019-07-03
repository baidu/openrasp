<template>
  <div>
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          防护引擎白名单
        </h3>
      </div>
      <div class="card-body">
        <p>最多允许200个URL，单条URL长度限制为200字符</p>
        <b-table hover bordered :items="data" :fields="fields">
          <template slot="index" slot-scope="scope" nowrap>
            {{ scope.index + 1 }}
          </template>
          <template slot="hook" slot-scope="scope">
            <span v-if="scope.value.all">
              所有 Hook 点
            </span>
            <span v-if="!scope.value.all">
              {{ whitelist2str(scope.value) }}
            </span>
          </template>
          <template slot="command" slot-scope="scope">
            <a href="javascript:" @click="showModal(scope.index)">
              编辑
            </a>
            <a href="javascript:" @click="deleteItem(scope.index)">
              删除
            </a>
          </template>
        </b-table>
      </div>
      <div v-bind:class="{'card-footer': true, 'sticky-card-footer': sticky}">
        <button class="btn btn-info" @click="showModal(-1)">
          添加
        </button>
        <button class="btn btn-primary pull-right" @click="doSave()">
          保存
        </button>
      </div>
    </div>

    <b-modal id="whitelistEditModal" ref="modal" no-fade title="添加/编辑 白名单" size="lg" hide-header-close @hidden="hideModal()" @shown="$refs.focus.focus()">
      <div class="form-group">
        <label>URL - 不区分 http/https，格式如 <span class="text-danger">rasp.baidu.com/phpmyadmin/</span></label>
        <input ref="focus" v-model.trim="modalData.url" maxlength="200" type="text" class="form-control" maxlen="200">
      </div>
      <div class="form-group">
        <label>检测点</label>
        <div class="row">
          <div class="col-12">
            <label class="custom-switch">
              <input v-model="modalData.hook.all" type="checkbox" checked="modalData.hook.all" class="custom-switch-input">
              <span class="custom-switch-indicator" />
              <span class="custom-switch-description">
                关闭所有检测点
              </span>
            </label>
          </div>
        </div>
        <div v-if="!modalData.hook.all" class="row">
          <div v-for="(item, key) in attack_types" :key="key" class="col-6">
            <label class="custom-switch">
              <input v-model="modalData.hook[key]" type="checkbox" checked="modalData.hook[key]" class="custom-switch-input">
              <span class="custom-switch-indicator" />
              <span class="custom-switch-description">
                {{ item }}
              </span>
            </label>
          </div>
        </div>
      </div>
      <div slot="modal-footer" class="w-100">
        <b-button class="float-right ml-2 btn-info" variant="default" @click="hideModal()">
          取消
        </b-button>
        <b-button class="float-right ml-2" variant="primary" @click="hideModal(true)">
          确定
        </b-button>
      </div>
    </b-modal>
  </div>
</template>

<script>
import { mapGetters, mapActions, mapMutations } from "vuex";
import { attack_type2name, attack_types } from '@/util/'

export default {
  name: 'WhitelistSettings',
  data: function() {
    return {
      data: [],
      index: 0,
      fields: [
        { key: 'index', label: '#', tdAttr: {'nowrap': ''} },
        { key: 'url', label: 'URL' },
        { key: 'hook', label: '检测点', tdAttr: {'style': 'min-width: 150px; '} },
        { key: 'command', label: '操作', tdAttr: {'nowrap': ''} }
      ],
      modalData: { url: '', hook: {}},
      attack_types
    }
  },
  computed: {
    ...mapGetters(['current_app', 'sticky'])
  },
  mounted: function() {

  },
  methods: {
    ...mapMutations(["setSticky"]),
    whitelist2str(row) {
      return Object.keys(row).filter(key => row[key]).map(key => attack_type2name(key)).join(', ')
    },
    setData: function(data) {
      this.data = data
    },
    showModal(index) {
      if (index === -1 && this.data.length >= 200) {
        alert('为了保证性能，白名单最多支持 200 条')
        return
      }

      this.setSticky(false)
      this.index = index
      Object.assign(this.modalData, JSON.parse(JSON.stringify(this.data[index] || {})))
      this.$refs.modal.show()
    },
    hideModal(save) {
      if (save === true) {
        if (!this.modalData.url || this.modalData.url.trim().length == 0) {
          alert('URL 未填写')
          return
        }
        if (this.modalData.url.startsWith('http://') || this.modalData.url.startsWith('https://')) {
          alert('URL 无需以 http/https 开头，请删除')
          return
        }

        let hookSelected = Object.values(this.modalData.hook).indexOf(true)
        if (hookSelected < 0) {
          alert('请至少选择一个 hook 点来加白名单')
          return
        }

        if (this.index == -1) {
          this.index = this.data.length
        }

        this.$set(this.data, this.index, this.modalData)
        console.log (this.index, this.data)
      }
      this.modalData = { url: '', hook: {}}
      this.$refs.modal.hide()
      this.setSticky(true)
    },
    deleteItem: function(index) {
      if (!confirm('确认删除?')) {
        return
      }
      this.data.splice(index, 1)
    },
    doSave() {
      return this.request.post('v1/api/app/whitelist/config', {
        app_id: this.current_app.id,
        config: this.data
      }).then(() => {
        alert('保存成功')
      })
    }
  }
}
</script>
