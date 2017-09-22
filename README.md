# OpenRASP [![Build Status](https://www.travis-ci.org/baidu/openrasp.svg?branch=master)](https://www.travis-ci.org/baidu/openrasp)

### Introduction

Unlike perimeter control solutions like WAF, OpenRASP directly integrates its protection engine into the application server by instrumentation. It can monitor various events including database queries, file operations and network requests etc.

When an attack happens, WAF matches the malicious request with its signatures and blocks it. OpenRASP takes a different approach by hooking sensitive functions and examines/blocks the inputs fed into them. As a result, this examination is context-aware and in-place. It brings in the following benefits:

1.	Only successful attacks can trigger alarms, resulting in lower false positive and higher detection rate;
2.	Detailed stack trace is logged, which makes the forensic analysis easier;
3.	Insusceptible to malformed protocol.

### Quick Start

See detailed installation instructions [here](https://rasp.baidu.com/doc/install/main.html)

We also provide a few test cases that are corresponding to OWASP TOP 10 attacks, [download here](https://rasp.baidu.com/doc/install/testcase.html)

### FAQ

##### 1. List of supported web application servers

Only Java based web application servers are supported for now. The support of other web application servers will also be soon included in the coming releases.

We've fully tested OpenRASP on the following application servers for both Linux and Windows platforms.

* Tomcat 6-8
* JBoss 4.X
* WebLogic 11/12

##### 2. Performance impact on application servers

We ran multiple intense and long-lasting stress tests prior to release. Even in the worst-case scenario (where the hook point got continuously triggered) the server’s performance was only reduced by 10%

##### 3. Integration with existing SIEM or SOC

OpenRASP logs alarms in JSON format, which can be easily picked up by LogStash, rsyslog or Flume.

##### 4. How to develop a new plugin?

A plugin receives a callback when an event occurs. It then determines if the current behavior is malicious or not and blocks the associated request if necessary.

Detailed plugin development instructions can be found [here](https://rasp.baidu.com/doc/dev/main.html)

### Contact

Technical support:

* [Baidu Security Forum](http://anquan.baidu.com/bbs)
* [RASP QQ group #1: 259318664](http://shang.qq.com/wpa/qunwpa?idkey=5016bac5431b23316a79efdcd2c4dadd6ef8b99b231e4ed10f1e265573a66e1c)
* [openrasp-en-discuss Mailing List](https://sourceforge.net/projects/openrasp-en/lists/openrasp-en-discuss)

Business inquires, comments, concenrs and opinions:

* General email: `fuxi-pm # baidu.com`









