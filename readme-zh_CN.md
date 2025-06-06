# OpenRASP

### 背景

最近几年，Web攻击手段开始变得复杂，攻击面也越来越广。传统的安全防护手段，WAF、IDS 等等，大多是基于规则，已经不能满足企业对安全的基本需求。Gartner 在2014年提出了 "应用自我保护" 技术的概念，即 "对应用服务的保护，不应该依赖于外部系统；应用应该具备自我保护的能力"。

为此，百度推出了开源的自适应安全产品 -- OpenRASP，希望通过开源免费的形式，让更多的人参与进来，并让互联网变得更加安全

### 里程碑

[点击这里查看 roadmap](https://github.com/baidu/openrasp/wiki)

### 项目简介

RASP 技术以探针的形式，将保护引擎注入到应用服务中，能够对应用程序的执行流进行实时的检测，并拦截攻击。当应用服务器收到一个恶意请求，保护引擎就可以结合上下文，识别用户输入，检查应用逻辑是否被用户输入所修改，并决定是否阻断这个请求。

这种做法具有如下优势，

1. 只有成功的攻击才会触发报警，所以误报率和漏报率都要低一些
2. RASP可以记录详细的调用堆栈，帮助你理解漏洞成因
3. 不受畸形协议的影响

### 开始使用

请参考[官方文档](https://rasp.baidu.com/doc/install/main.html)进行安装和配置

另外，为了方便你对软件进行测试，我们提供了一组测试用例，包含OWASP常见安全漏洞，[点击这里查看](https://rasp.baidu.com/doc/install/testcase.html)

备份文档地址: https://test-730.gitbook.io/openrasp-documents-old

### 常见问题

##### 1. OpenRASP 都支持哪些应用服务器?

> 目前，我们支持 Tomcat 6-9, JBoss 4.X, Jetty 7-9, Resin 3-4, SpringBoot 1-2, WebSphere 8.5 & 9.0, WebLogic 10.3.6 & 12.2,  PHP 5.3-7.4 等服务器，更多服务器支持开发中

##### 2. OpenRASP 是否会影响服务器的性能?

> 通常，OpenRASP 对服务器的性能影响在 1~5%，响应延迟 1~8ms，可忽略

##### 3. 如何集成到现有的 SIEM/SOC 平台中?

> 通过 Logstash、rsyslog 等方式，你可以采集每台服务器上的报警日志，并发到中央日志处理平台

##### 4. 如何开发一个检测插件?

> 插件由JS实现，我们提供了详细的开发和测试 SDK，具体请参考[插件开发文档](https://rasp.baidu.com/doc/dev/main.html)

### 联系我们

在使用软件中发现任何问题，你可以通过邮件、QQ技术讨论群或者论坛的形式进行技术交流

* [RASP 技术讨论群-1: 259318664](http://shang.qq.com/wpa/qunwpa?idkey=5016bac5431b23316a79efdcd2c4dadd6ef8b99b231e4ed10f1e265573a66e1c)

官方合作、OEM 合作申请:

* `fuxi-pm # baidu.com`

