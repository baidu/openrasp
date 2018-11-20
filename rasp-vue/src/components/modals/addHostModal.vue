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
              <pre>wget 192.168.154.200/packages/rasp-java.tar.gz</pre>
              <h4>2. 执行 RaspInstall 进行安装</h4>
              <pre>java -jar RaspInstall.jar /path/to/tomcat</pre>
              <h4>3. 重启 Tomcat/JBoss/SpringBoot 应用服务器</h4>
            </div>
            <div class="tab-pane fade" id="php-tab">
              <h4>1. 下载 PHP 安装包</h4>
              <pre>wget 192.168.154.200/packages/rasp-php.tar.gz</pre>
              <h4>2. 执行 install.php 进行安装</h4>
              <pre>php install.php -d /opt/rasp</pre>
              <h4>3. 重启 PHP-FPM 或者 Apache 服务器</h4>
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
export default {
    name: 'addHostModal',
    data: function() {
        return {
            data: []
        }
    },
    methods: {
        showModal() {
            $('#addHostModal').modal()
        }
    }
}

</script>
