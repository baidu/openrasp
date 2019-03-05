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
          管理后台地址 [用于生成邮件里的报警链接]
        </label>
        <input
          v-model="data.panel_url"
          type="text"
          class="form-control"
        />
      </div>
      <div class="form-group">
        <label class="form-label">
          Agent 服务器列表 [用于生成 "添加主机" 里的安装命令]
        </label>
        <textarea
          type="text"
          class="form-control"
          v-model="data.agent_urls_text"
          rows="5"
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
        agent_url: [],
        agent_urls_text: ''
      },
    };
  },
  computed: {
    ...mapGetters(["current_app"]),
  },
  mounted: function() {
    this.loadData()
  },
  methods: {
    loadData(data) {
      this.request
        .post("v1/api/server/url/get", {})
        .then((res) => {
          this.data = res
          this.data.agent_urls_text = res.agent_urls.join("\n")
      });
    },
    parseAgentURL: function() {
      var tmp = []
      this.data.agent_urls_text.split("\n").forEach(function (item) {
        item = item.trim()
        if (item.length) {
          tmp.push(item)
        }
      })

      return tmp
    },
    saveData: function() {
      var data = {
        panel_url: this.data.panel_url,
        agent_urls: this.parseAgentURL()
      }
      return this.request
        .post("v1/api/server/url", data)
        .then(() => {
          alert("保存成功");
        });
    }
  }
};
</script>
