FROM tomcat:8.0
MAINTAINER OpenRASP Developers

ARG version_testcase
ENV version_testcase=${version_testcase:-v1.0.1}

ARG version_rasp
ENV version_rasp=${version_rasp:-v0.24}

ENV TOMCAT_HOME /usr/local/tomcat
ENV PATH $TOMCAT_HOME/bin:$PATH

ADD "https://github.com/baidu-security/openrasp-testcases/releases/download/${version_testcase}/vulns.war" "$TOMCAT_HOME"/webapps
ADD "https://github.com/baidu/openrasp/releases/download/${version_rasp}/rasp-java.tar.gz" /tmp/rasp-java.tar.gz

RUN cd /tmp \
	&& tar -zxvf rasp-java.tar.gz \
	&& java -jar rasp-*/RaspInstall.jar "$TOMCAT_HOME"

EXPOSE 8080
CMD ["catalina.sh", "run"]
