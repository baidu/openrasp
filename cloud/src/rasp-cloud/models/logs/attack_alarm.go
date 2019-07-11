//Copyright 2017-2019 Baidu Inc.
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http: //www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

package logs

import (
	"crypto/md5"
	"fmt"
	"rasp-cloud/es"
	"github.com/olivere/elastic"
	"time"
	"context"
	"github.com/oschwald/geoip2-golang"
	"github.com/astaxie/beego"
	"net"
	"rasp-cloud/tools"
	"encoding/json"
	"rasp-cloud/conf"
)

var (
	AttackAlarmInfo = AlarmLogInfo{
		EsType:       "attack-alarm",
		EsIndex:      "openrasp-attack-alarm",
		EsAliasIndex: "real-openrasp-attack-alarm",
		AlarmBuffer:  make(chan map[string]interface{}, conf.AppConfig.AlarmBufferSize),
		FileLogger:   initAlarmFileLogger("openrasp-logs/attack-alarm", "attack.log"),
	}
	geoIpDbPath string
	geoIpDb     *geoip2.Reader

	AttackTypeMap = map[interface{}]string{
		"sql":                        "SQL 注入",
		"sql_exception":              "SQL 语句异常",
		"command":                    "命令执行",
		"xxe":                        "XXE 外部实体加载",
		"directory":                  "目录遍历",
		"rename":                     "文件重命名",
		"readFile":                   "任意文件下载",
		"include":                    "任意文件包含",
		"writeFile":                  "任意文件写入",
		"ssrf":                       "SSRF 服务端请求伪造",
		"ognl":                       "OGNL 代码执行",
		"webdav":                     "任意文件上传 (PUT)",
		"fileUpload":                 "任意文件上传",
		"deserialization":            "Transformer 反序列化",
		"xss_echo":                   "Echo XSS 跨站脚本攻击",
		"xss_userinput":              "BODY XSS 跨站脚本攻击",
		"webshell_callable":          "WebShell - 变形后门",
		"webshell_eval":              "WebShell - 中国菜刀",
		"webshell_command":           "WebShell - 命令执行",
		"webshell_file_put_contents": "WebShell - 后门上传",
		"webshell_ld_preload":        "WebShell - LD_PRELOAD 后门",
	}

	AttackInterceptMap = map[interface{}]string{
		"block": "拦截请求",
		"log":   "记录日志",
	}
)

func init() {
	var err error
	registerAlarmInfo(&AttackAlarmInfo)
	geoIpDbPath = "geoip/GeoLite2-City.mmdb"
	geoIpDb, err = geoip2.Open(geoIpDbPath)
	if err != nil {
		tools.Panic(tools.ErrCodeGeoipInit, "failed to open geoip database", err)
	}
}

func AddAttackAlarm(alarm map[string]interface{}) error {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to add attack alarm: ", r)
		}
	}()
	if stack, ok := alarm["stack_trace"]; ok && stack != nil && stack != "" {
		_, ok = stack.(string)
		if ok {
			alarm["stack_md5"] = fmt.Sprintf("%x", md5.Sum([]byte(stack.(string))))
		}
	}
	setAlarmLocation(alarm)
	return AddAlarmFunc(AttackAlarmInfo.EsType, alarm)
}

func setAlarmLocation(alarm map[string]interface{}) {
	if attackSource, ok := alarm["attack_source"]; ok && attackSource != nil {
		_, ok = attackSource.(string)
		if ok {
			attackIp := net.ParseIP(attackSource.(string))
			record, err := geoIpDb.City(attackIp)
			if err != nil {
				beego.Error("failed to parse attack ip to location: " + err.Error())
			}
			if record != nil {
				alarm["attack_location"] = map[string]interface{}{
					"location_zh_cn": record.Country.Names["zh-CN"] + "-" + record.City.Names["zh-CN"],
					"location_en":    record.Country.Names["en"] + "-" + record.City.Names["en"],
					"latitude":       record.Location.Latitude,
					"longitude":      record.Location.Longitude,
				}
			}
		}
	}
}

func AggregationAttackWithTime(startTime int64, endTime int64, interval string, timeZone string,
	appId string) (map[string]interface{}, error) {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
	defer cancel()
	timeAggrName := "aggr_time"
	interceptAggrName := "request_sum"
	timeAggr := elastic.NewDateHistogramAggregation().Field("event_time").TimeZone(timeZone).
		Interval(interval).ExtendedBounds(startTime, endTime)
	interceptAggr := elastic.NewTermsAggregation().Field("intercept_state")
	timeAggr.SubAggregation(interceptAggrName, interceptAggr)
	timeQuery := elastic.NewRangeQuery("event_time").Gte(startTime).Lte(endTime)
	aggrResult, err := es.ElasticClient.Search(AttackAlarmInfo.EsAliasIndex + "-" + appId).
		Query(elastic.NewBoolQuery().Must(timeQuery)).
		Aggregation(timeAggrName, timeAggr).
		Size(0).
		Do(ctx)
	if err != nil {
		if aggrResult != nil && aggrResult.Error != nil {
			errMsg, err := json.Marshal(aggrResult.Error)
			if err != nil {
				beego.Error(string(errMsg))
			}
		}
		return nil, err
	}

	var dataResult = make([][]int64, 2)
	dataResult[0] = make([]int64, 0)
	dataResult[1] = make([]int64, 0)
	var labels = make([]interface{}, 0)
	var result = make(map[string]interface{})
	if aggrResult != nil && aggrResult.Aggregations != nil {
		if terms, ok := aggrResult.Aggregations.Terms(timeAggrName); ok && terms.Buckets != nil {
			labelCount := len(terms.Buckets)
			dataResult[0] = make([]int64, labelCount)
			dataResult[1] = make([]int64, labelCount)
			labels = make([]interface{}, labelCount)
			for index, timeTerm := range terms.Buckets {
				labels[index] = timeTerm.Key
				if interceptTerm, ok := timeTerm.Terms(interceptAggrName); ok {
					for _, item := range interceptTerm.Buckets {
						if item.Key == "block" {
							dataResult[0][index] = item.DocCount
						} else if item.Key == "log" {
							dataResult[1][index] = item.DocCount
						}
					}
				}
			}
		}
	}
	result["data"] = dataResult
	result["labels"] = labels
	return result, nil
}

func AggregationAttackWithUserAgent(startTime int64, endTime int64, size int,
	appId string) ([][]interface{}, error) {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
	defer cancel()
	uaAggr := elastic.NewTermsAggregation().Field("header.user-agent.keyword").Size(size).OrderByCount(false)
	timeQuery := elastic.NewRangeQuery("event_time").Gte(startTime).Lte(endTime)
	aggrName := "aggr_ua"
	aggrResult, err := es.ElasticClient.Search(AttackAlarmInfo.EsAliasIndex + "-" + appId).
		Query(timeQuery).
		Aggregation(aggrName, uaAggr).
		Size(0).
		Do(ctx)
	if err != nil {
		if aggrResult != nil && aggrResult.Error != nil {
			errMsg, err := json.Marshal(aggrResult.Error)
			if err != nil {
				beego.Error(string(errMsg))
			}
		}
		return nil, err
	}
	result := make([][]interface{}, 0)
	if aggrResult != nil && aggrResult.Aggregations != nil {
		if terms, ok := aggrResult.Aggregations.Terms(aggrName); ok && terms.Buckets != nil {
			result = make([][]interface{}, len(terms.Buckets))
			for index, item := range terms.Buckets {
				result[index] = make([]interface{}, 2, 2)
				result[index][0] = item.Key
				result[index][1] = item.DocCount
			}
		}
	}
	return result, nil
}

func AggregationAttackWithType(startTime int64, endTime int64, size int,
	appId string) ([][]interface{}, error) {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
	defer cancel()
	typeAggr := elastic.NewTermsAggregation().Field("attack_type").Size(size).OrderByCount(false)
	timeQuery := elastic.NewRangeQuery("event_time").Gte(startTime).Lte(endTime)
	aggrName := "aggr_type"
	aggrResult, err := es.ElasticClient.Search(AttackAlarmInfo.EsAliasIndex + "-" + appId).
		Query(timeQuery).
		Aggregation(aggrName, typeAggr).
		Size(0).
		Do(ctx)
	if err != nil {
		if aggrResult != nil && aggrResult.Error != nil {
			errMsg, err := json.Marshal(aggrResult.Error)
			if err != nil {
				beego.Error(string(errMsg))
			}
		}
		return nil, err
	}
	result := make([][]interface{}, 0)
	if aggrResult != nil && aggrResult.Aggregations != nil {
		if terms, ok := aggrResult.Aggregations.Terms(aggrName); ok && terms.Buckets != nil {
			result = make([][]interface{}, len(terms.Buckets))
			for index, item := range terms.Buckets {
				result[index] = make([]interface{}, 2, 2)
				result[index][0] = item.Key
				result[index][1] = item.DocCount
			}
		}
	}
	return result, nil
}
