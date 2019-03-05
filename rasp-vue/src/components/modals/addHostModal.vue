<template>
  <div id="addHostModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">
            添加主机
          </h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close" />
        </div>
        <div class="modal-body" style="padding-top: 0">
          <div v-if="agent_urls.length == 0">
            <p>
              <br>
              你还没有设置 Agent 服务器地址，请先前往 
              <router-link data-dismiss="modal" :to="{name: 'settings', params: {setting_tab: 'panel'}}">[后台设置]</router-link> 页面添加
            </p>            
          </div>
          <div v-else>
            <ul id="myTab" class="nav nav-tabs" role="tablist">
              <li class="nav-item">
                <a class="nav-link" data-toggle="tab" href="#common-tab">
                  参数信息
                </a>
              </li>
              <li class="nav-item">
                <a class="nav-link active" data-toggle="tab" href="#java-tab">
                  Java 服务器
                </a>
              </li>
              <li class="nav-item">
                <a class="nav-link" data-toggle="tab" href="#php-tab">
                  PHP 服务器
                </a>
              </li>
            </ul>
            <br>
            <div id="myTabContent" class="tab-content">
              <div id="common-tab" class="tab-pane fade">
                <h4>AppID</h4>
                <pre>{{ current_app.id }}</pre>
                <h4>AppSecret</h4>
                <pre>{{ current_app.secret }}</pre>
                <h4>BackendURL<small v-if="agent_urls.length > 1" style="margin-left: 5px;">[任选一个即可]</small></h4>
                <pre>{{ agent_urls.join("\n") }}</pre>
              </div>
              <div id="java-tab" class="tab-pane fade show active">
                <h4>1. 下载 Java Agent 安装包</h4>
                <pre style="white-space: inherit; ">curl https://packages.baidu.com/app/openrasp/rasp-java.tar.gz -o rasp-java.tar.gz<br>tar -xvf rasp-java.tar.gz<br>cd rasp-*/</pre>
                <h4>2. 执行 RaspInstall 进行安装</h4>
                <pre style="white-space: inherit; ">java -jar RaspInstall.jar -install /path/to/tomcat -appid {{ current_app.id }} -appsecret {{ current_app.secret }} -backendurl {{ agent_urls[agent_url_id] }}</pre>
                <h4>3. 重启 Tomcat/JBoss/SpringBoot 应用服务器</h4>
                <pre style="white-space: inherit; ">/path/to/tomcat/bin/shutdown.sh<br>/path/to/tomcat/bin/startup.sh</pre>
              </div>
              <div id="php-tab" class="tab-pane fade">
                <h4>1. 下载 PHP 安装包</h4>
                <pre style="white-space: inherit; ">curl https://packages.baidu.com/app/openrasp/rasp-php-linux.tar.bz2 -o rasp-php-linux.tar.bz2<br>tar -xvf rasp-php-linux.tar.bz2<br>cd rasp-*/</pre>
                <h4>2. 执行 install.php 进行安装</h4>
                <pre style="white-space: inherit; ">php install.php -d /opt/rasp --app-id {{ current_app.id }} --app-secret {{ current_app.secret }} --backend-url {{ agent_urls[agent_url_id] }}</pre>
                <h4>3. 重启 PHP-FPM 或者 Apache 服务器</h4>
                <pre style="white-space: inherit; ">service php-fpm restart</pre>
                <p>-或者-</p>
                <pre style="white-space: inherit; ">apachectl -k restart</pre>
              </div>          
            </div>
          </div>
        </div>
        <div class="modal-footer">
          <a class="btn btn-secondary mr-auto" href="https://rasp.baidu.com/doc/install/software.html" target="_blank">
            了解更多
          </a>
          <button class="btn btn-primary" data-dismiss="modal">
            关闭
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'

export default {
  name: 'AddHostModal',
  data: function() {
    return {
      data: {},
      location: location,
      agent_url_id: 0,
      agent_urls: [],
      panel_url: ''
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  methods: {
    showModal(data) {
      return this.request.post('v1/api/server/url/get', {})
        .then(res => {
          this.agent_urls = res.agent_urls
          this.panel_url = res.panel_url
          $('#addHostModal').modal()
        })
    }
  }
}

</script>
