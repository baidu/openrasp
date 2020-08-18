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
        <input v-model="data.panel_url" type="text" class="form-control" />
      </div>
      <div class="form-group">
        <label class="form-label">
          若开启<a href="https://rasp.baidu.com/doc/install/panel.html#load-balance" target="_blank">负载均衡</a>，请填写 Agent 服务器列表 [用于生成 "添加主机" 里的安装命令，一行一个]
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
    <b-card>
      <div slot="header">
        <h3 class="card-title">
          清空数据
        </h3>
      </div>     
      <p>点击执行后，会清空如下内容（<strong>仅当前应用</strong>）</p>      
      <ul>
        <li v-for="x in ['攻击事件', '基线报警', '异常日志', '请求数量', '崩溃信息']">{{x}}</li>
      </ul>
      <div slot="footer">
        <b-button variant="danger" @click="removeLogs">
          执行
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
        panel_url: "",
        agent_url: [],
        agent_urls_text: ""
      }
    };
  },
  computed: {
    ...mapGetters(["current_app"])
  },
  mounted: function() {
    this.loadData();
  },
  methods: {
    loadData(data) {
      this.request.post("v1/api/server/url/get", {}).then(res => {
        this.data = res;
        this.data.agent_urls_text = res.agent_urls.join("\n");
      });
    },
    parseAgentURL: function() {
      var tmp = [];
      this.data.agent_urls_text.split("\n").forEach(function(item) {
        item = item.trim();
        if (item.length) {
          tmp.push(item);
        }
      });

      return tmp;
    },
    saveData: function() {
      var data = {
        panel_url: this.data.panel_url,
        agent_urls: this.parseAgentURL()
      };
      return this.request.post("v1/api/server/url", data).then(() => {
        alert("保存成功");
      });
    },
    removeLogs: function() {
      if (! confirm('清空日志不可恢复，请确认')) {
        return
      }

      var data = {
        app_id: this.current_app.id
      }

      return this.request.post("v1/api/server/clear_logs", data).then(() => {
        alert("操作成功");
      });
    }
  }
};
</script>
