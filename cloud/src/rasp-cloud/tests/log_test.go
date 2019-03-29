package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	"time"
	"rasp-cloud/models/logs"
	"github.com/bouk/monkey"
	"errors"
	"reflect"
	"github.com/olivere/elastic"
	"context"
)

func TestPostLog(t *testing.T) {
	Convey("Subject: Test Post Log", t, func() {
		Convey("when log type is attack", func() {
			r := inits.GetResponse("POST", "/v1/agent/log/attack", `[
			{"app_id" : "`+ start.TestApp.Id+ `",
            "attack_location" : {
            "latitude" : 0,
            "location_en" : "-",
            "location_zh_cn" : "-",
            "longitude" : 0
            },
            "attack_params" : {
              "mysql_connection_id" : "80",
              "query" : "SELECT * FROM vuln WHERE id = 1 or 88=88 or 77=77 or 99=99 ",
              "server" : "mysql"
            },
            "attack_source" : "0:0:0:0:0:0:0:1",
            "attack_type" : "sql",
            "body" : "------WebKitFormBoundarywDlHUATC0PUJIDfu\r\nContent-Disposition: form-data; name=\"id\"\r\n\r\n1 or 88=88 or 77=77 or 99=99 \r\n------WebKitFormBoundarywDlHUATC0PUJIDfu--\r\n",
            "client_ip" : "",
            "event_time" : "1551882976000",
            "event_type" : "attack",
            "intercept_state" : "block",
            "path" : "/vulns/013-multipart-mysql.jsp",
            "plugin_algorithm" : "sqli_userinput",
            "plugin_confidence" : 90,
            "plugin_message" : "SQLi - SQL query structure altered by user input, request parameter name: id",
            "plugin_name" : "java_builtin_plugin",
            "rasp_id" : "f5e618eaae43a3c5df13e649bb899e47",
            "referer" : "http://localhost:8080/vulns/013-multipart-mysql.jsp",
            "request_id" : "53e4de3429094d948378653accc29094",
            "request_method" : "post",
            "server_hostname" : "27AFCB34B359922",
            "server_ip" : "0:0:0:0:0:0:0:1",
            "server_nic" : [
            {
              "ip" : "172.20.94.113",
              "name" : "eth3"
            },
            {
              "ip" : "192.168.168.1",
              "name" : "eth5"
            },
            {
              "ip" : "192.168.23.1",
              "name" : "eth6"
            }
            ],
            "server_type" : "tomcat",
            "server_version" : "7.0.78.0",
            "stack_trace" : "com.mysql.jdbc.StatementImpl.executeQuery(StatementImpl.java)\norg.apache.jsp._013_002dmultipart_002dmysql_jsp.runQuery(_013_002dmultipart_002dmysql_jsp.java:34)\norg.apache.jsp._013_002dmultipart_002dmysql_jsp._jspService(_013_002dmultipart_002dmysql_jsp.java:208)\norg.apache.jasper.runtime.HttpJspBase.service(HttpJspBase.java:70)\njavax.servlet.http.HttpServlet.service(HttpServlet.java:731)\norg.apache.jasper.servlet.JspServletWrapper.service(JspServletWrapper.java:439)\norg.apache.jasper.servlet.JspServlet.serviceJspFile(JspServlet.java:395)\norg.apache.jasper.servlet.JspServlet.service(JspServlet.java:339)\njavax.servlet.http.HttpServlet.service(HttpServlet.java:731)\norg.apache.catalina.core.ApplicationFilterChain.internalDoFilter(ApplicationFilterChain.java:303)\n",
            "target" : "localhost",
            "url" : "http://localhost:8080/vulns/013-multipart-mysql.jsp",
            "user_agent" : "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36"}]`)
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when log type is policy", func() {
			r := inits.GetResponse("POST", "/v1/agent/log/policy", `[
			{ "app_id" : "`+ start.TestApp.Id+ `",
			"server_nic" : [
            {
              "name" : "matrix0",
              "ip" : "172.27.0.1"
            },
            {
              "name" : "xgbe0",
              "ip" : "10.115.184.54"
            }
          ],
          "server_type" : "tomcat",
          "host" : "nmg01-scloud-siem-admin.nmg01.baidu.com",
          "server_version" : "8.0.50.0",
          "event_type" : "security_policy",
          "message" : "Database security baseline - Connecting to a mysql instance with high privileged account root, connectionString is jdbc:mysql://127.0.0.1:3306/jeecmsv8",
          "event_time" : "1551882976000",
          "type" : "policy-alarm",
          "server_hostname" : "gzns-scloud-api-db02.gzns.baidu.com",
          "policy_id" : "3006",
          "rasp_id" : "e68745ffb5715170f4a5871634206bd1",
          "stack_md5" : "0a9eb2096e10383d6f9e53b7332d798c",
          "@version" : "1",
          "policy_params" : {
            "username" : "root",
            "server" : "mysql",
            "connectionString" : "jdbc:mysql://127.0.0.1:3306/jeecmsv8"
          },
          "@timestamp" : "2018-11-21T15:26:05.057Z",
          "stack_trace" : "com.mysql.jdbc.NonRegisteringDriver.connect(NonRegisteringDriver.java:299)\ncom.mchange.v2.c3p0.DriverManagerDataSource.getConnection(DriverManagerDataSource.java:134)\ncom.mchange.v2.c3p0.WrapperConnectionPoolDataSource.getPooledConnection(WrapperConnectionPoolDataSource.java:182)\ncom.mchange.v2.c3p0.WrapperConnectionPoolDataSource.getPooledConnection(WrapperConnectionPoolDataSource.java:171)\ncom.mchange.v2.c3p0.impl.C3P0PooledConnectionPool$1PooledConnectionResourcePoolManager.acquireResource(C3P0PooledConnectionPool.java:137)\ncom.mchange.v2.resourcepool.BasicResourcePool.doAcquire(BasicResourcePool.java:1014)\ncom.mchange.v2.resourcepool.BasicResourcePool.access$800(BasicResourcePool.java:32)\ncom.mchange.v2.resourcepool.BasicResourcePool$AcquireTask.run(BasicResourcePool.java:1810)\ncom.mchange.v2.async.ThreadPoolAsynchronousRunner$PoolThread.run(ThreadPoolAsynchronousRunner.java:547)\n",
          "path" : "/home/work/openrasp-server/openrasp-logs/policy-alarm/policy.log"}]`)
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when log type is error", func() {
			r := inits.GetResponse("POST", "/v1/agent/log/error", `[
			{ "app_id" : "`+ start.TestApp.Id+ `",
          "@version" : "1",
          "rasp_id" : "68e21b7df62b96bbf327db5b51530df1",
          "stack_trace" : "java.net.URL.<init>(URL.java:600)\njava.net.URL.<init>(URL.java:490)\njava.net.URL.<init>(URL.java:439)\ncom.baidu.openrasp.messaging.LogConfig.syslogManager(LogConfig.java:52)\ncom.baidu.openrasp.cloud.KeepAlive.handleResponse(KeepAlive.java:112)\ncom.baidu.openrasp.cloud.KeepAlive.access$000(KeepAlive.java:36)\ncom.baidu.openrasp.cloud.KeepAlive$KeepAliveThread.run(KeepAlive.java:52)\njava.lang.Thread.run(Thread.java:748)\n",
          "pid" : 7672,
          "message" : "syslog url: tcp://172.24.180.174:8848 parsed error",
          "event_time" : "1551882976000",
          "host" : "nmg01-scloud-siem-admin.nmg01.baidu.com",
          "error_code" : 20004,
          "path" : "/home/work/openrasp-server-agent/openrasp-logs/error-alarm/error.log",
          "type" : "error-alarm",
          "server_hostname" : "geyanping-IdeaCentre-B540",
          "level" : "WARN",
          "server_nic" : [
            {
              "ip" : "172.23.232.86",
              "name" : "cscotun0"
            },
            {
              "ip" : "172.17.0.1",
              "name" : "docker0"
            },
            {
              "ip" : "172.20.94.84",
              "name" : "eth0"
            }
          ]}]`)
			So(r.Status, ShouldEqual, 0)
		})

	})
}

func getAttackLogSearchData() map[string]interface{} {
	return map[string]interface{}{
		"page":    1,
		"perpage": 1,
		"data": map[string]interface{}{
			"app_id":          start.TestApp.Id,
			"start_time":      time.Now().Second() * 1000000,
			"end_time":        time.Now().Second() * 1000000,
			"attack_type":     &[]string{"sql"},
			"local_ip":        "127.0.0.1",
			"attack_source":   "127.0.0.1",
			"intercept_state": &[]string{"block"},
		},
	}
}

func getPolicyLogSearchData() map[string]interface{} {
	return map[string]interface{}{
		"page":    1,
		"perpage": 1,
		"data": map[string]interface{}{
			"app_id":          start.TestApp.Id,
			"start_time":      time.Now().Second() * 1000000,
			"end_time":        time.Now().Second() * 1000000,
			"policy_id":       &[]string{"3006"},
			"server_hostname": "ubuntu",
		},
	}
}

func getNormalSearchData() map[string]interface{} {
	return map[string]interface{}{
		"page":    1,
		"perpage": 1,
		"data": map[string]interface{}{
			"app_id":     start.TestApp.Id,
			"start_time": 1,
			"end_time":   time.Now().Second() * 10000000000000,
		},
	}
}

func TestLogSearch(t *testing.T) {
	Convey("Subject: Test Log Search Api\n", t, func() {

		Convey("when log type is attack", func() {
			r := inits.GetResponse("POST", "/v1/api/log/attack/search",
				inits.GetJson(getAttackLogSearchData()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the es has errors in log search", func() {
			monkey.Patch(logs.SearchLogs, func(startTime int64, endTime int64, isAttachAggr bool, query map[string]interface{}, sortField string,
				page int, perpage int, ascending bool, index ...string) (int64, []map[string]interface{}, error) {
				return 0, nil, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(getAttackLogSearchData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(getAttackLogSearchData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			r = inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(getAttackLogSearchData()))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(logs.SearchLogs)
		})

		Convey("when log type is policy", func() {
			r := inits.GetResponse("POST", "/v1/api/log/policy/search",
				inits.GetJson(getPolicyLogSearchData()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when log type is error", func() {
			r := inits.GetResponse("POST", "/v1/api/log/error/search",
				inits.GetJson(getNormalSearchData()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when search data app_id is empty", func() {
			data := getNormalSearchData()
			data["data"].(map[string]interface{})["app_id"] = ""
			r := inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when search data is nil", func() {
			data := getNormalSearchData()
			data["data"] = nil
			r := inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when search data app_id is not exist", func() {
			data := getNormalSearchData()
			data["data"].(map[string]interface{})["app_id"] = "222222222222222222"
			r := inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when start time is less than 0", func() {
			data := getNormalSearchData()
			data["data"].(map[string]interface{})["start_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when end time is less than 0", func() {
			data := getNormalSearchData()
			data["data"].(map[string]interface{})["end_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when start time is less than end_time", func() {
			data := getNormalSearchData()
			data["data"].(map[string]interface{})["start_time"] = 10
			data["data"].(map[string]interface{})["end_time"] = 5
			r := inits.GetResponse("POST", "/v1/api/log/error/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/policy/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/search", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func getAggrParam() map[string]interface{} {
	return map[string]interface{}{
		"app_id":     start.TestApp.Id,
		"end_time":   1551882976000,
		"interval":   "day",
		"size":       10,
		"start_time": 1549463775000,
		"time_zone":  "+08:00",
	}
}

func TestAttackLogAggr(t *testing.T) {
	Convey("Subject: Test Attack Log Aggr Api\n", t, func() {

		Convey("when aggr with time", func() {
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(getAggrParam()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when aggr with ua", func() {
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/ua", inits.GetJson(getAggrParam()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when aggr with attack type", func() {
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(getAggrParam()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when aggr app_id does not exist", func() {
			data := getAggrParam()
			data["app_id"] = "222222222222222222"
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when aggr app_id is empty", func() {
			data := getAggrParam()
			data["app_id"] = ""
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when aggr app_id does not exist in time aggr", func() {
			data := getAggrParam()
			data["app_id"] = "222222222222222222"
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when aggr app_id is empty in time aggr", func() {
			data := getAggrParam()
			data["app_id"] = ""
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id is empty for vuln search", func() {
			data := getNormalSearchData()
			data["app_id"] = nil
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/vuln", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when start time is less than 0", func() {
			data := getAggrParam()
			data["start_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when end time is less than 0", func() {
			data := getAggrParam()
			data["end_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when start time is less than end_time", func() {
			data := getAggrParam()
			data["start_time"] = 10
			data["end_time"] = 5
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when size is less than 0", func() {
			data := getAggrParam()
			data["size"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when time interval is greater than 1 year", func() {
			data := getAggrParam()
			data["start_time"] = 1
			data["end_time"] = time.Now().UnixNano() / 1000000
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when aggr with vuln", func() {
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/vuln", inits.GetJson(getAttackLogSearchData()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when start time is less than 0 in time aggr", func() {
			data := getAggrParam()
			data["start_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when end time is less than 0 in time aggr", func() {
			data := getAggrParam()
			data["end_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when start time is less than end_time in time aggr", func() {
			data := getAggrParam()
			data["start_time"] = 10
			data["end_time"] = 5
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when time interval is greater than 1 year in time aggr", func() {
			data := getAggrParam()
			data["start_time"] = 1
			data["end_time"] = time.Now().UnixNano() / 1000000
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when time interval is empty", func() {
			data := getAggrParam()
			data["interval"] = ""
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when time zone is empty", func() {
			data := getAggrParam()
			data["time_zone"] = ""
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of time interval is greater than 32", func() {
			data := getAggrParam()
			data["interval"] = inits.GetLongString(33)
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of time zone is greater than 32", func() {
			data := getAggrParam()
			data["time_zone"] = inits.GetLongString(33)
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the es has errors", func() {
			monkey.Patch(logs.AggregationAttackWithTime, func(int64, int64, string, string,
				string) (map[string]interface{}, error) {
				return nil, errors.New("")
			})
			data := getAggrParam()
			r := inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(logs.AggregationAttackWithTime)

			monkey.Patch(logs.AggregationAttackWithType, func(startTime int64, endTime int64, size int,
				appId string) ([][]interface{}, error) {
				return nil, errors.New("")
			})
			data = getAggrParam()
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(logs.AggregationAttackWithType)

			monkey.Patch(logs.AggregationAttackWithUserAgent, func(startTime int64, endTime int64, size int,
				appId string) ([][]interface{}, error) {
				return nil, errors.New("")
			})
			data = getAggrParam()
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/ua", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(logs.AggregationAttackWithUserAgent)

			monkey.Patch(logs.SearchLogs, func(startTime int64, endTime int64, isAttachAggr bool, query map[string]interface{}, sortField string,
				page int, perpage int, ascending bool, index ...string) (int64, []map[string]interface{}, error) {
				return 0, nil, errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/vuln", inits.GetJson(getAttackLogSearchData()))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(logs.SearchLogs)

			monkey.PatchInstanceMethod(reflect.TypeOf(&elastic.SearchService{}), "Do",
				func(*elastic.SearchService, context.Context) (*elastic.SearchResult, error) {
					return &elastic.SearchResult{Error: &elastic.ErrorDetails{}}, errors.New("")
				},
			)
			data = getAggrParam()
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/type", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/ua", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/vuln", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			r = inits.GetResponse("POST", "/v1/api/log/attack/aggr/time", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.UnpatchInstanceMethod(reflect.TypeOf(&elastic.SearchService{}), "Do")
		})

	})
}

func TestAddLogWithFile(t *testing.T) {
	Convey("Subject: Test Add Log With File\n", t, func() {
		Convey("when es addr is empty", func() {
			err := logs.AddLogWithFile("attack-alarm", map[string]interface{}{})
			So(err, ShouldEqual, nil)
		})

		Convey("when log type is not supported", func() {
			logs.AddLogWithFile("invalid-alarm", map[string]interface{}{})
		})

		Convey("when alarm is nil", func() {
			logs.AddLogWithFile("attack-alarm", nil)
		})
	})

}
