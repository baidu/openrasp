# OpenRASP 

[![Build Status](https://www.travis-ci.org/baidu/openrasp.svg?branch=master)](https://www.travis-ci.org/baidu/openrasp)
[![Build Status](https://img.shields.io/badge/README-切换语言-yellow.svg)](readme-zh_CN.md)

### Introduction

Unlike perimeter control solutions like WAF, OpenRASP directly integrates its protection engine into the application server by instrumentation. It can monitor various events including database queries, file operations and network requests etc.

When an attack happens, WAF matches the malicious request with its signatures and blocks it. OpenRASP takes a different approach by hooking sensitive functions and examines/blocks the inputs fed into them. As a result, this examination is context-aware and in-place. It brings in the following benefits:

1.	Only successful attacks can trigger alarms, resulting in lower false positive and higher detection rate;
2.	Detailed stack trace is logged, which makes the forensic analysis easier;
3.	Insusceptible to malformed protocol.

### Quick Start

See detailed installation instructions [here](https://github.com/baidu/openrasp/wiki/Installation)

We also provide a few test cases that are corresponding to OWASP TOP 10 attacks, [download here](https://rasp.baidu.com/doc/install/testcase.html)

### FAQ

#### 1. List of supported web application servers

We've fully tested OpenRASP on the following application servers for Linux platforms:

* Java
  * Tomcat 6-9
  * JBoss 4.X
  * Jetty 7-9
  * Resin 3-4
  * SpringBoot 1-2
  * IBM WebSphpere 8.5, 9.0
  * WebLogic 10.3.6, 12.2.1
* PHP
  * 5.3-5.6, 7.0-7.4

The support of other web application servers will also be soon included in the coming releases.

#### 2. Performance impact on application servers

We ran multiple intense and long-lasting stress tests prior to release. Even in the worst-case scenario (where the hook point got continuously triggered) the server's performance was only reduced by 1\~4%

#### 3. Integration with existing SIEM or SOC

OpenRASP logs alarms in JSON format, which can be easily picked up by Logstash, rsyslog or Flume.

#### 4. How to develop a new plugin?

A plugin receives a callback when an event occurs. It then determines if the current behavior is malicious or not and blocks the associated request if necessary.

Detailed plugin development instructions can be found [here](https://rasp.baidu.com/doc/dev/main.html)

### Contact

Technical support:

* [RASP QQ group #2: 595568655](http://shang.qq.com/wpa/qunwpa?idkey=5016bac5431b23316a79efdcd2c4dadd6ef8b99b231e4ed10f1e265573a66e1c)
* [RASP QQ group #1 (full): 259318664](http://shang.qq.com/wpa/qunwpa?idkey=5016bac5431b23316a79efdcd2c4dadd6ef8b99b231e4ed10f1e265573a66e1c)
* [OpenRASP Google Group](https://groups.google.com/forum/#!forum/openrasp)

Business inquires, comments and security reports:

* General email: `openrasp-support # baidu.com`







