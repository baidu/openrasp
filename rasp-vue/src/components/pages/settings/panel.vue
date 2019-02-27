<template>
  <div>
    <b-card>
      <div slot="header">
        <h3 class="card-title">
          后台设置
        </h3>
      </div>
      <div class="form-group">
        <label class="form-label">
          管理后台地址 [用户生成报警邮件URL]
        </label>
        <input
          v-model="data['panel_url']"
          type="text"
          class="form-control"
        />
      </div>
      <div class="form-group">
        <label class="form-label">
          Agent 服务器列表
        </label>
        <textarea
          type="text"
          class="form-control"
        />
      </div>      
      <div slot="footer">
        <b-button variant="primary" @click="saveData">
          保存
        </b-button>
      </div>
    </b-card>
  </div>
</template>

<script>
import { mapGetters } from "vuex";

export default {
  name: "PanelSettings",
  data() {
    return {
      data: { 
        panel_url: '',
        agent_url: []
      },
    };
  },
  computed: {
    ...mapGetters(["current_app"]),
  },
  methods: {
    getData(data) {
      data["inject.custom_headers"] = data["inject.custom_headers"] || {};
      this.browser_headers.forEach(header => {
        if (
          !header.options.some(
            option =>
              option.value === data["inject.custom_headers"][header.name]
          )
        ) {
          header.options.push({
            name: data["inject.custom_headers"][header.name],
            value: data["inject.custom_headers"][header.name]
          });
        }
      });
      this.data = data;
    },
    saveData: function() {
      return this.request
        .post("v1/api/server/url", this.data)
        .then(() => {
          alert("保存成功");
        });
    }
  }
};
</script>
