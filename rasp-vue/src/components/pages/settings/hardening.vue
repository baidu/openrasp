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
              v-model="custom_headers[row.name]"
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
      data: { custom_headers: {}},
      browser_headers
    }
  },
  computed: {
    ...mapGetters(['current_app']),
    custom_headers() {
      return this.data['inject.custom_headers'] || {}
    }
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
