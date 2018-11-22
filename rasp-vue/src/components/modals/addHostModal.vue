<template>
  <div class="modal no-fade" id="addHostModal" tabindex="-1" role="dialog">
    <div class="modal-dialog" role="document">
      <div class="modal-content">
        <div class="modal-header">
          <h5 class="modal-title">添加主机</h5>
          <button type="button" class="close" data-dismiss="modal" aria-label="Close"></button>
        </div>
        <div class="modal-body" style="padding-top: 0">
          <ul class="nav nav-tabs" id="myTab" role="tablist">
            <li class="nav-item">
              <a class="nav-link active" data-toggle="tab" href="#java-tab">Java 服务器</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" data-toggle="tab" href="#php-tab">PHP 服务器</a>
            </li>
          </ul>
          <br>
          <div class="tab-content" id="myTabContent">
            <div class="tab-pane fade show active" id="java-tab">
              <h4>1. 下载 Java Agent 安装包</h4>
              <pre style="white-space: inherit; ">wget http://{{ location.host }}/packages/rasp-java.tar.gz<br/>tar -xvf rasp-java.tar.gz<br/>cd rasp-*/</pre>
              <h4>2. 执行 RaspInstall 进行安装</h4>
              <pre style="white-space: inherit; ">java -jar RaspInstall.jar -install /path/to/tomcat -appid {{ current_app.id }} -appsecret {{ current_app.secret }} -backendurl http://{{ location.host }}</pre>
              <h4>3. 重启 Tomcat/JBoss/SpringBoot 应用服务器</h4>
              <pre style="white-space: inherit; ">/path/to/tomcat/bin/shutdown.sh<br/>/path/to/tomcat/bin/startup.sh</pre>
            </div>
            <div class="tab-pane fade" id="php-tab">
              <h4>1. 下载 PHP 安装包</h4>
              <pre style="white-space: inherit; ">wget http://{{ location.host }}/packages/rasp-php.tar.gz</pre>
              <h4>2. 执行 install.php 进行安装</h4>
              <pre style="white-space: inherit; ">php install.php -d /opt/rasp --app-id {{ current_app.id }} --app-secret {{ current_app.secret }} --backend-url http://{{ location.host }}</pre>
              <h4>3. 重启 PHP-FPM 或者 Apache 服务器</h4>
              <pre style="white-space: inherit; ">service php-fpm restart</pre>
              <p>-或者-</p>
              <pre style="white-space: inherit; ">apachectl -k restart</pre>
            </div>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-secondary mr-auto" href="https://rasp.baidu.com/doc/install/software.html" target="_blank">
            了解更多
          </button>
          <button class="btn btn-primary" data-dismiss="modal">关闭</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'

export default {
  name: 'addHostModal',
  data: function () {
    return {
      data: {},
      location: location
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  methods: {
    showModal(data) {
      $('#addHostModal').modal()
    }
  }
}

</script>
