<template>
  <div>
    <b-card>
      <div slot="header">
        <h3 class="card-title">
          应用加固
        </h3>
      </div>
      <div v-for="row in browser_headers" :key="row.name" class="form-group">
        <label class="form-label">
          {{ row.descr }}
          <a :href="'https://rasp.baidu.com/doc/usage/hardening.html#' + row.name" target="_blank">
            [帮助文档]
          </a>
        </label>
        <div class="selectgroup">
          <label
            v-for="option in row.options"
            :key="option.name"
            class="selectgroup-item"
          >
            <input
              v-model="data['inject.custom_headers'][row.name]"
              type="radio"
              class="selectgroup-input"
              :name="row.name"
              :value="option.value"
            >
            <span class="selectgroup-button">
              {{ option.name }}
            </span>
          </label>
        </div>
      </div>

      <div class="form-group">
        <label class="form-label">自定义 X-Protected-By 头（要求 Agent 版本 >= 1.2.2，留空则不展示）
          <a href="https://rasp.baidu.com/doc/usage/hardening.html#X-Protected-By" target="_blank">
            [帮助文档]
          </a>
        </label>
        <input type="text" class="form-control" v-model="data['inject.custom_headers']['X-Protected-By']" maxlength="200">
      </div>

      <div slot="footer">
        <b-button
          variant="primary"
          @click="saveData"
        >
          保存
        </b-button>
      </div>
    </b-card>
  </div>
</template>

<script>
import { browser_headers } from '@/util'
import { mapGetters } from 'vuex'

export default {
  name: 'HardeningSettings',
  data() {
    return {
      data: { 
        'inject.custom_headers': {}
      },
      browser_headers
    }
  },
  computed: {
    ...mapGetters(['current_app']),
  },
  methods: {
    setData(data) {
      data['inject.custom_headers'] = data['inject.custom_headers'] || {}
      this.browser_headers.forEach(header => {
        if (!header.options.some(option => option.value === data['inject.custom_headers'][header.name])) {
          header.options.push({
            name: data['inject.custom_headers'][header.name],
            value: data['inject.custom_headers'][header.name]
          })
        }
      })
      this.data = data
    },
    saveData: function() {
      var data = Object.assign(this.data)

      // 避免下发空的配置
      if (typeof data['inject.custom_headers']['X-Protected-By'] === 'string') {
        data['inject.custom_headers']['X-Protected-By'] = data['inject.custom_headers']['X-Protected-By'].trim()
        if (data['inject.custom_headers']['X-Protected-By'].length == 0)
          data['inject.custom_headers']['X-Protected-By'] = undefined
      }

      return this.request.post('v1/api/app/general/config', {
        app_id: this.current_app.id,
        config: this.data
      }).then(() => {
        alert('保存成功')
      })
    }
  }
}
</script>
