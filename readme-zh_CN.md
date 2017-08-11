# OpenRASP

### 简介

RASP 以探针的形式，将保护引擎注入到应用服务中，可在文件、数据库、网络等多个层面，对应用进行全面的保护。当发生敏感的行为时，可以结合请求上下文进行的判断，并阻断攻击，具有低误报率，低漏报率的优点

### 开始使用

请参考[官方文档](http://rasp.baidu.com/doc/install/main.html)进行安装和配置

另外，为了方便你对软件进行测试，我们提供了一组测试用例，包含OWASP常见安全漏洞，[点击这里下载](http://rasp.baidu.com/doc/install/testcase.html)

### 常见问题

##### 1. OpenRASP 都支持哪些应用服务器?

> 目前，我们支持 Tomcat 6-8, JBoss 4.X, WebLogic 11/12 等服务器，更多服务器支持开发中

##### 2. OpenRASP 是否会影响服务器的性能?

> 经过长时间的压力测试，OpenRASP 对服务器的性能影响在 5% 以内

##### 3. 如何集成到现有的 SIEM/SOC 平台中?

> 通过 LogStash、rsyslog 等方式，你可以采集每台服务器上的报警日志，并发到中央日志处理平台

##### 4. 如何开发一个检测插件?

> 插件由JS实现，我们提供了详细的开发和测试 SDK，具体请参考[插件开发文档](http://rasp.baidu.com/doc/dev/main.html)

### 联系我们

在使用软件中发现任何问题，你可以通过邮件、QQ技术讨论群或者论坛的形式进行技术交流

* [百度安全论坛](http://anquan.baidu.com/bbs)
* [RASP 技术讨论群 群1: 259318664](http://shang.qq.com/wpa/qunwpa?idkey=5016bac5431b23316a79efdcd2c4dadd6ef8b99b231e4ed10f1e265573a66e1c)

官方合作、OEM 合作申请:

* `fuxi-pm # baidu.com`


