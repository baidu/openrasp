<template>
  <div id="addHostModal" class="modal no-fade" tabindex="-1" role="dialog">
    <div class="modal-dialog modal-lg" role="document">
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
                  手动安装
                </a>
              </li>
              <li class="nav-item">
                <a class="nav-link" data-toggle="tab" href="#batch-tab">
                  批量部署
                </a>
              </li>
              <li class="nav-item">
                <a class="nav-link" data-toggle="tab" href="#docker-tab">
                  Docker 部署
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
              <li class="nav-item">
                <a class="nav-link" data-toggle="tab" href="#iast-tab">
                  Fuzz 工具安装
                </a>
              </li>
            </ul>
            <br>
            <div id="myTabContent" class="tab-content">
              <div id="common-tab" class="tab-pane fade">
                <p>OpenRASP 自动安装程序可以覆盖绝大多数场景，如果你的环境无法自动安装，请参考 <a href="https://rasp.baidu.com/doc/install/software.html" target="_blank">安装客户端</a> 进行手动安装。以下是连接管理后台所需要的关键参数，Java 版本请附加到 conf/openrasp.yml，PHP 版本请附加到 ini 文件。</p>
                <h4>Java 版本</h4>
                <pre>cloud.enable: true
cloud.backend_url: {{ agent_urls[0] }}
cloud.app_id: {{ current_app.id }}
cloud.app_secret: {{ current_app.secret }}
cloud.heartbeat_interval: 90</pre>

                <h4>PHP 版本</h4>
                <pre>openrasp.app_id={{ current_app.id }}
openrasp.app_secret={{ current_app.secret }}            
openrasp.backend_url={{ agent_urls[0] }}
openrasp.heartbeat_interval=90</pre>

                <!-- <h4>AppID</h4>
                <pre>{{ current_app.id }}</pre>
                <h4>AppSecret</h4>
                <pre>{{ current_app.secret }}</pre>
                <h4>BackendURL<small v-if="agent_urls.length > 1" style="margin-left: 5px;">[任选一个即可]</small></h4>
                <pre>{{ agent_urls.join("\n") }}</pre> -->
              </div>

              <div id="batch-tab" class="tab-pane fade">
                <div class="alert alert-warning">
                  批量部署脚本，会自动安装并重启应用，更多信息请看
                  <a target="_blank" href="https://rasp.baidu.com/doc/install/deploy.html" class="active router-link-active">大规模部署</a>
                  文档
                </div>
                <h4>1. 下载自动安装程序</h4>
                <pre style="white-space: inherit; ">curl https://packages.baidu.com/app/openrasp/release/{{rasp_version}}/installer.sh -o installer.sh</pre>
                <h4>2. 执行脚本</h4>
                <pre style="white-space: inherit; ">bash installer.sh -i -a {{ current_app.id }} -b {{ current_app.secret }} -c {{ agent_urls[agent_url_id] }}</pre>
              </div>
              <div id="docker-tab" class="tab-pane fade">
                <div class="alert alert-warning">
                  在构建镜像阶段加入 OpenRASP 即可，更多信息请看
                  <a target="_blank" href="https://rasp.baidu.com/doc/install/deploy.html#container" class="active router-link-active">大规模部署</a>
                  文档
                </div>
                <h4>Java Tomcat 容器示例</h4>
                <pre>ADD https://packages.baidu.com/app/openrasp/release/{{rasp_version}}/rasp-java.tar.gz /tmp
RUN cd /tmp \
    && tar -xf rasp-java.tar.* \
    && /jdk/bin/java -jar rasp-*/RaspInstall.jar -install /tomcat/ -heartbeat 90 -appid {{ current_app.id }} -appsecret {{ current_app.secret }} -backendurl {{ agent_urls[agent_url_id] }} \
    && rm -rf rasp-*
# 对于Alpine Linux容器，需要安装系统依赖
# RUN apk add --no-cache libcurl libstdc++</pre>
                <h4>Java SpringBoot 容器示例</h4>
                <pre>ADD https://packages.baidu.com/app/openrasp/release/{{rasp_version}}/rasp-java.tar.gz /tmp
RUN cd /tmp \
    && tar -xf rasp-java.tar.* \
    && mv rasp-*/rasp/ /rasp/ \
    && rm -f rasp-java.tar.gz
# 对于Alpine Linux容器，需要安装系统依赖
# RUN apk add --no-cache libcurl libstdc++

RUN echo "cloud.enable: true" >> /rasp/conf/openrasp.yml \
    && echo "cloud.backend_url: {{ agent_urls[agent_url_id] }}" >> /rasp/conf/openrasp.yml \
    && echo "cloud.app_id: {{ current_app.id }}" >> /rasp/conf/openrasp.yml \
    && echo "cloud.app_secret: {{ current_app.secret }}" >> /rasp/conf/openrasp.yml

# 对于 JDK9，需要额外增加 --add-opens java.base/jdk.internal.loader=ALL-UNNAMED 参数
CMD java -javaagent:"/rasp/rasp.jar" -jar /springboot.jar</pre>

                <h4>PHP 容器示例</h4>
                <pre>ADD https://packages.baidu.com/app/openrasp/release/{{rasp_version}}/rasp-php-linux.tar.bz2 /tmp/
RUN cd /tmp \
    && tar -xf rasp-php-linux.tar.bz2 \
    && php rasp-php-*/install.php -d /opt/rasp/ --heartbeat 90 --backend-url {{ agent_urls[agent_url_id] }} --app-id {{ current_app.id }} --app-secret {{ current_app.secret }} \
    && rm -rf rasp-php*</pre>
              </div>
              <div id="java-tab" class="tab-pane fade show active">
                <h4>1. 下载 Java Agent 安装包</h4>
                <pre style="white-space: inherit; ">curl https://packages.baidu.com/app/openrasp/release/{{rasp_version}}/rasp-java.tar.gz -o rasp-java.tar.gz<br>tar -xvf rasp-java.tar.gz<br>cd rasp-*/</pre>
                <h4>2. 执行 RaspInstall 进行安装</h4>
                <p>请先替换 /path/to/tomcat 为你的服务器路径，再执行命令安装</p>
                <pre style="white-space: inherit; ">java -jar RaspInstall.jar -heartbeat 90 -appid {{ current_app.id }} -appsecret {{ current_app.secret }} -backendurl {{ agent_urls[agent_url_id] }} -install <font color="red">/path/to/tomcat</font></pre>
                <h4>3. 重启 Tomcat/JBoss/WebLogic/SpringBoot 应用服务器</h4>
                <pre style="white-space: inherit; ">/path/to/tomcat/bin/shutdown.sh<br>/path/to/tomcat/bin/startup.sh</pre>
              </div>
              <div id="php-tab" class="tab-pane fade">
                <h4>1. 下载 PHP 安装包</h4>
                <pre style="white-space: inherit; ">curl https://packages.baidu.com/app/openrasp/release/{{rasp_version}}/rasp-php-linux.tar.bz2 -o rasp-php-linux.tar.bz2<br>tar -xvf rasp-php-linux.tar.bz2<br>cd rasp-*/</pre>
                <h4>2. 执行 install.php 进行安装</h4>
                <pre style="white-space: inherit; ">php install.php -d <font color="red">/opt/rasp</font> --heartbeat 90 --app-id {{ current_app.id }} --app-secret {{ current_app.secret }} --backend-url {{ agent_urls[agent_url_id] }}</pre>
                <h4>3. 重启 PHP-FPM 或者 Apache 服务器</h4>
                <pre style="white-space: inherit; ">service php-fpm restart</pre>
                <p>-或者-</p>
                <pre style="white-space: inherit; ">apachectl -k restart</pre>
              </div>
              <div id="iast-tab" class="tab-pane fade">
                <h4>1. 下载或者升级 Fuzz 工具</h4>
                <pre style="white-space: inherit; ">pip3 install --upgrade git+https://github.com/baidu-security/openrasp-iast</pre>
                <h4>2. 配置 MySQL 服务器 - 使用 MySQL root 账号执行以下命令授权</h4>
                <pre>DROP DATABASE IF EXISTS openrasp;
CREATE DATABASE openrasp default charset utf8mb4 COLLATE utf8mb4_general_ci;
grant all privileges on openrasp.* to 'rasp'@'%' identified by 'rasp123';
grant all privileges on openrasp.* to 'rasp'@'localhost' identified by 'rasp123';
</pre>
                <h4>3. 配置 Fuzz 工具 - 请修正 MySQL 服务器地址</h4>
                <pre style="white-space: inherit; ">openrasp-iast config -a {{ current_app.id }} -b {{ current_app.secret }} -c {{ panel_url }} -m mysql://rasp:rasp123@127.0.0.1/openrasp</pre>
                <h4>4. 启动 Fuzz 工具</h4>
                <pre style="white-space: inherit; ">openrasp-iast start -f</pre>
                <p>-或者后台启动-</p>
                <pre style="white-space: inherit; ">openrasp-iast start</pre>
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
import { mapGetters, mapActions, mapMutations } from "vuex";
import { rasp_version } from '@/util'

export default {
  name: 'AddHostModal',
  data: function() {
    return {
      data: {},
      location: location,
      agent_url_id: 0,
      agent_urls: [],
      panel_url: '',
      rasp_version: rasp_version
    }
  },
  computed: {
    ...mapGetters(['current_app', 'sticky'])
  },
  mounted: function() {
    var self = this

    $('#addHostModal').on('hidden.bs.modal', function () {
      self.setSticky(true)
    })
  },
  methods: {
    ...mapMutations(['setSticky']),
    showModal(data) {
      return this.request.post('v1/api/server/url/get', {})
        .then(res => {
          this.agent_urls = res.agent_urls
          this.panel_url = res.panel_url
          this.setSticky(false)
          $('#addHostModal').modal()
        })
    }
  }
}

</script>
