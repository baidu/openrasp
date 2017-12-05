FROM tomcat:8.0
MAINTAINER OpenRASP Team

ARG version_testcase
ENV version_testcase=${version_testcase:-v1.0.1}

ARG version_rasp
ENV version_rasp=${version_rasp:-v0.20}

ENV TOMCAT_HOME /usr/local/tomcat
ENV PATH $TOMCAT_HOME/bin:$PATH
RUN wget --no-check-certificate "https://github.com/baidu-security/openrasp-testcases/releases/download/${version_testcase}/vulns.war" -P "$TOMCAT_HOME"/webapps

RUN wget --no-check-certificate "https://github.com/baidu/openrasp/releases/download/${version_rasp}/rasp-any.tar.gz"
RUN tar -zxvf rasp-any.tar.gz
RUN mv rasp-*/rasp "$TOMCAT_HOME"

RUN cd rasp-* && java -jar RaspInstall.jar "$TOMCAT_HOME"

EXPOSE 8080
CMD ["catalina.sh", "run"]
