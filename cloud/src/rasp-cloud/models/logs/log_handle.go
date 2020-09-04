//Copyright 2017-2020 Baidu Inc.
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
	"context"
	"encoding/json"
	"fmt"
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/logs"
	"github.com/olivere/elastic"
	"net/url"
	"os"
	"path"
	"rasp-cloud/es"
	"rasp-cloud/tools"
	"strconv"
	"time"
	"rasp-cloud/conf"
	"strings"
	"crypto/md5"
	"rasp-cloud/kafka"
)

type AggrTimeParam struct {
	AppId     string `json:"app_id"`
	StartTime int64  `json:"start_time"`
	EndTime   int64  `json:"end_time"`
	Interval  string `json:"interval"`
	TimeZone  string `json:"time_zone"`
}

type AggrFieldParam struct {
	AppId     string `json:"app_id"`
	StartTime int64  `json:"start_time"`
	EndTime   int64  `json:"end_time"`
	Size      int    `json:"size"`
}

type SearchAttackParam struct {
	Page    int `json:"page"`
	Perpage int `json:"perpage"`
	Data *struct {
		Id             string    `json:"_id,omitempty"`
		AppId          string    `json:"app_id,omitempty"`
		StartTime      int64     `json:"start_time"`
		EndTime        int64     `json:"end_time"`
		RaspId         string    `json:"rasp_id,omitempty"`
		HostName       string    `json:"server_hostname,omitempty"`
		AttackSource   string    `json:"attack_source,omitempty"`
		AttackUrl      string    `json:"url,omitempty"`
		LocalIp        string    `json:"local_ip,omitempty"`
		ClientIp       string    `json:"client_ip,omitempty"`
		StackMd5       string    `json:"stack_md5,omitempty"`
		RequestId      string    `json:"request_id,omitempty"`
		PluginMessage  string    `json:"plugin_message,omitempty"`
		AttackType     *[]string `json:"attack_type,omitempty"`
		InterceptState *[]string `json:"intercept_state,omitempty"`
	} `json:"data"`
}

type SearchPolicyParam struct {
	Page    int `json:"page"`
	Perpage int `json:"perpage"`
	Data *struct {
		Id        string    `json:"_id,omitempty"`
		AppId     string    `json:"app_id,omitempty"`
		StartTime int64     `json:"start_time"`
		EndTime   int64     `json:"end_time"`
		RaspId    string    `json:"rasp_id,omitempty"`
		HostName  string    `json:"server_hostname,omitempty"`
		LocalIp   string    `json:"local_ip,omitempty"`
		Message   string    `json:"message,omitempty"`
		PolicyId  *[]string `json:"policy_id,omitempty"`
	} `json:"data"`
}

type SearchErrorParam struct {
	Page    int `json:"page"`
	Perpage int `json:"perpage"`
	Data *struct {
		Id        string `json:"_id,omitempty"`
		AppId     string `json:"app_id,omitempty"`
		StartTime int64  `json:"start_time"`
		EndTime   int64  `json:"end_time"`
		RaspId    string `json:"rasp_id,omitempty"`
		HostName  string `json:"server_hostname,omitempty"`
		LocalIp   string `json:"local_ip,omitempty"`
		Message   string `json:"message,omitempty"`
	} `json:"data"`
}

type SearchCrashParam struct {
	Page    int `json:"page"`
	Perpage int `json:"perpage"`
	Data *struct {
		Id             string    `json:"_id,omitempty"`
		AppId          string    `json:"app_id,omitempty"`
		StartTime      int64     `json:"start_time"`
		EndTime        int64     `json:"end_time"`
		HostName       string    `json:"hostname,omitempty"`
		Language       *[]string `json:"language,omitempty"`
		RaspId         string    `json:"rasp_id,omitempty"`
		CrashMessage   string    `json:"crash_message,omitempty"`
	} `json:"data"`
}

type AlarmLogInfo struct {
	EsType       string
	EsIndex      string
	EsAliasIndex string
	FileLogger   *logs.BeeLogger
	AlarmBuffer  chan map[string]interface{}
}

var (
	AddAlarmFunc func(string, map[string]interface{}) error
	alarmInfos   = make(map[string]*AlarmLogInfo)
)

func init() {
	if conf.AppConfig.AlarmLogMode == "file" {
		AddAlarmFunc = AddLogWithFile
	} else if conf.AppConfig.AlarmLogMode == "es" {
		startEsAlarmLogPush()
		AddAlarmFunc = AddLogWithES
	} else {
		tools.Panic(tools.ErrCodeConfigInitFailed, "Unrecognized the value of RaspLogMode config", nil)
	}
}

func registerAlarmInfo(info *AlarmLogInfo) {
	alarmInfos[info.EsType] = info
	es.RegisterTTL(24*time.Duration(conf.AppConfig.EsTTL)*time.Hour, info.EsAliasIndex+"-*")
}

func initAlarmFileLogger(dirName string, fileName string) *logs.BeeLogger {
	maxSize := strconv.FormatInt(conf.AppConfig.LogMaxSize, 10)
	maxDays := strconv.Itoa(conf.AppConfig.LogMaxDays)
	if isExists, _ := tools.PathExists(dirName); !isExists {
		err := os.MkdirAll(dirName, os.ModePerm)
		if err != nil {
			tools.Panic(tools.ErrCodeLogInitFailed, "failed to init alarm logger", err)
		}
	}

	logger := logs.NewLogger()
	logPath := path.Join(dirName, fileName)
	err := logger.SetLogger(tools.AdapterAlarmFile,
		`{"filename":"`+logPath+`", "daily":true, "maxdays":`+maxDays+`, "perm":"0777","maxsize": `+maxSize+`}`)
	if err != nil {
		tools.Panic(tools.ErrCodeLogInitFailed, "failed to init alarm logger", err)
	}
	return logger
}

func startEsAlarmLogPush() {
	go func() {
		for {
			handleEsLogPush()
		}
	}()
}

func handleEsLogPush() {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to push es alarm log: ", r)
		}
	}()
	select {
	case alarm := <-AttackAlarmInfo.AlarmBuffer:
		alarms := make([]map[string]interface{}, 0, 200)
		alarms = append(alarms, alarm)
		for len(AttackAlarmInfo.AlarmBuffer) > 0 && len(alarms) < 200 {
			alarm := <-AttackAlarmInfo.AlarmBuffer
			alarms = append(alarms, alarm)
		}
		err := es.BulkInsertAlarm(AttackAlarmInfo.EsType, alarms)
		if err != nil {
			beego.Error("failed to execute es bulk insert for attack alarm: " + err.Error())
		}
	case alarm := <-PolicyAlarmInfo.AlarmBuffer:
		alarms := make([]map[string]interface{}, 0, 200)
		alarms = append(alarms, alarm)
		for len(PolicyAlarmInfo.AlarmBuffer) > 0 && len(alarms) < 200 {
			alarm := <-PolicyAlarmInfo.AlarmBuffer
			alarms = append(alarms, alarm)
		}
		err := es.BulkInsertAlarm(PolicyAlarmInfo.EsType, alarms)
		if err != nil {
			beego.Error("failed to execute es bulk insert for policy alarm: " + err.Error())
		}
	case alarm := <-CrashAlarmInfo.AlarmBuffer:
		alarms := make([]map[string]interface{}, 0, 200)
		alarms = append(alarms, alarm)
		for len(CrashAlarmInfo.AlarmBuffer) > 0 && len(alarms) < 200 {
			alarm := <-CrashAlarmInfo.AlarmBuffer
			alarms = append(alarms, alarm)
		}
		err := es.BulkInsertAlarm(CrashAlarmInfo.EsType, alarms)
		if err != nil {
			beego.Error("failed to execute es bulk insert for crash alarm: " + err.Error())
		}
	case alarm := <-ErrorAlarmInfo.AlarmBuffer:
		alarms := make([]map[string]interface{}, 0, 200)
		alarms = append(alarms, alarm)
		for len(ErrorAlarmInfo.AlarmBuffer) > 0 && len(alarms) < 200 {
			alarm := <-ErrorAlarmInfo.AlarmBuffer
			alarms = append(alarms, alarm)
		}
		err := es.BulkInsertAlarm(ErrorAlarmInfo.EsType, alarms)
		if err != nil {
			beego.Error("failed to execute es bulk insert for error alarm: " + err.Error())
		}
	}
}

func AddLogWithFile(alarmType string, alarm map[string]interface{}) error {
	if info, ok := alarmInfos[alarmType]; ok && info.FileLogger != nil {
		content, err := json.Marshal(alarm)
		if err != nil {
			return err
		}
		_, err = info.FileLogger.Write(content)
		if err != nil {
			logs.Error("failed to write rasp log: " + err.Error())
			return err
		}
	} else {
		logs.Error("failed to write rasp log, unrecognized log type: " + alarmType)
	}
	return nil
}

func AddLogWithKafka(alarmType string, log map[string]interface{}) error {
	if appId, ok := log["app_id"].(string); ok {
		kafkaIndex := "real-openrasp-" + alarmType + "-" + appId
		err := kafka.SendMessage(appId, kafkaIndex, log)
		if err != nil {
			return err
		}
	}
	return nil
}

func AddLogsWithKafka(alarmType string, appId string, logs []interface{}) error {
	kafkaIndex := "real-openrasp-" + alarmType + "-" + appId
	err := kafka.SendMessages(appId, kafkaIndex, logs)
	if err != nil {
		return err
	}
	return nil
}

func AddLogWithES(alarmType string, alarm map[string]interface{}) error {
	select {
	case alarmInfos[alarmType].AlarmBuffer <- alarm:
	default:
		logs.Error("Failed to write attack alarm to ES, " +
			"the buffer is full. Consider increase AlarmBufferSize value: " + fmt.Sprintf("%+v", alarm))
	}
	return nil
}

func getVulnAggr(attackTimeTopHitName string) (*elastic.TermsAggregation) {
	attackMaxAggrName := "attack_max_aggr"
	attackTimeTopHitAggr := elastic.NewTopHitsAggregation().
		Size(1).
		Sort("event_time", false).
		DocvalueFields("event_time", "attack_type", "intercept_state", "url",
		"path", "rasp_id", "attack_source", "plugin_algorithm", "server_ip", "server_hostname")
	attackTimeMaxAggr := elastic.NewMaxAggregation().Field("event_time")
	return elastic.NewTermsAggregation().
		Field("stack_md5").
		Size(10000).
		Order(attackMaxAggrName, false).
		SubAggregation(attackMaxAggrName, attackTimeMaxAggr).
		SubAggregation(attackTimeTopHitName, attackTimeTopHitAggr)
}

func SearchLogs(startTime int64, endTime int64, isAttachAggr bool, query map[string]interface{}, sortField string,
	page int, perpage int, ascending bool, index ...string) (int64, []map[string]interface{}, error) {
	var total int64
	var attackAggrName = "attack_aggr"
	var attackTimeTopHitName = "attack_time_top_hit"
	var typeIndex string
	filterQueries := make([]elastic.Query, 0, len(query)+1)
	shouldQueries := make([]elastic.Query, 0, len(query)+1)
	if query != nil {
		for key, value := range query {
			if key == "attack_type" {
				if v, ok := value.([]interface{}); ok {
					filterQueries = append(filterQueries, elastic.NewTermsQuery(key, v...))
				} else {
					filterQueries = append(filterQueries, elastic.NewTermQuery(key, value))
				}
			} else if key == "intercept_state" || key == "policy_id" || key == "language" {
				if v, ok := value.([]interface{}); ok {
					filterQueries = append(filterQueries, elastic.NewTermsQuery(key, v...))
				} else {
					filterQueries = append(filterQueries, elastic.NewTermQuery(key, value))
				}
			} else if key == "local_ip" {
				filterQueries = append(filterQueries,
					elastic.NewNestedQuery("server_nic", elastic.NewTermQuery("server_nic.ip", value)))
			} else if key == "attack_source" || key == "url" || key == "crash_message" || key == "hostname" ||
				key == "message" || key == "plugin_message" || key == "client_ip" {
				realValue := strings.TrimSpace(fmt.Sprint(value))
				filterQueries = append(filterQueries, elastic.NewWildcardQuery(key, "*"+realValue+"*"))
			} else if key == "server_hostname" {
				realValue := strings.TrimSpace(fmt.Sprint(value))
				shouldQueries = append(shouldQueries,
					elastic.NewWildcardQuery("server_hostname", "*"+realValue+"*"))
				shouldQueries = append(shouldQueries,
					elastic.NewNestedQuery("server_nic",
						elastic.NewWildcardQuery("server_nic.ip", "*"+realValue+"*")))
			} else {
				filterQueries = append(filterQueries, elastic.NewTermQuery(key, value))
			}
		}
	}
	filterQueries = append(filterQueries, elastic.NewRangeQuery("event_time").Gte(startTime).Lte(endTime))
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	boolQuery := elastic.NewBoolQuery().Filter(filterQueries...)
	if len(shouldQueries) > 0 {
		boolQuery.Should(shouldQueries...).MinimumNumberShouldMatch(1)
	}

	queryService := es.ElasticClient.Search(index...).Query(boolQuery)

	if isAttachAggr {
		attackAggr := getVulnAggr(attackTimeTopHitName)
		queryService.Aggregation(attackAggrName, attackAggr).Size(0)
	} else {
		queryService.From((page - 1) * perpage).Size(perpage).Sort(sortField, ascending)
	}

	queryResult, err := queryService.Do(ctx)

	if err != nil {
		if queryResult != nil && queryResult.Error != nil {
			beego.Error(queryResult.Error, index)
		}
		return 0, nil, err
	}
	result := make([]map[string]interface{}, 0)
	if !isAttachAggr {
		if queryResult != nil && queryResult.Hits != nil && queryResult.Hits.Hits != nil {
			hits := queryResult.Hits.Hits
			total = queryResult.Hits.TotalHits
			result = make([]map[string]interface{}, len(hits))
			for index, item := range hits {
				result[index] = make(map[string]interface{})
				var filterId string
				err := json.Unmarshal(*item.Source, &result[index])
				if err != nil {
					return 0, nil, err
				}
				if typeIndex == "attack" {
					requestId := result[index]["request_id"].(string)
					stackMd5 := result[index]["stack_md5"].(string)
					attackType := result[index]["attack_type"].(string)
					pluginAlgorithm := result[index]["plugin_algorithm"].(string)
					urlString := result[index]["url"].(string)
					if pluginAlgorithm == "response_dataLeak" {
						urlParse, err := url.Parse(urlString)
						if err != nil {
							return 0, nil, err
						}
						filterId = urlParse.Scheme + "://" + urlParse.Host + urlParse.Path
					} else {
						filterId = requestId + stackMd5 + attackType
					}
					result[index]["filter_id"] = filterId
				}
				es.HandleSearchResult(result[index], item.Id)
			}
		}
	} else {
		if queryResult != nil && queryResult.Aggregations != nil {
			if terms, ok := queryResult.Aggregations.Terms(attackAggrName); ok && terms.Buckets != nil {
				total = int64(len(terms.Buckets))
				result = make([]map[string]interface{}, 0, perpage)
				for i := 0; i < perpage; i++ {
					index := i + (page-1)*perpage
					if index >= int(total) {
						break
					}
					value := make(map[string]interface{})
					item := terms.Buckets[index]
					if topHit, ok := item.TopHits(attackTimeTopHitName); ok &&
						topHit.Hits != nil && topHit.Hits.Hits != nil {
						hits := topHit.Hits.Hits
						if len(hits) > 0 {
							err := json.Unmarshal(*hits[0].Source, &value)
							if err != nil {
								return 0, nil, err
							}
							value["attack_count"] = terms.Buckets[index].DocCount
							es.HandleSearchResult(value, hits[0].Id)
							result = append(result, value)
						}
					}
				}
			}

		}
	}

	return total, result, nil
}

func CreateAlarmEsIndex(appId string) (err error) {
	for _, alarmInfo := range alarmInfos {
		err = es.CreateEsIndex(alarmInfo.EsIndex + "-" + appId,
			alarmInfo.EsAliasIndex + "-" + appId, alarmInfo.EsType + "-template")
		if err != nil {
			return
		}
	}
	return
}

func putStackMd5(alarm map[string]interface{}, paramKey string) {
	var stackValue string
	if alarm[paramKey] != nil {
		if param, ok := alarm[paramKey].(map[string]interface{}); ok && len(param) > 0 && param["stack"] != nil {
			if stack, ok := param["stack"].([]interface{}); ok && len(stack) > 0 {
				for _, item := range stack {
					stackValue += fmt.Sprint(item)
					stackValue += "\n"
				}
				alarm["stack_md5"] = fmt.Sprintf("%x", md5.Sum([]byte(stackValue)))
				return
			}
		}
	}
	if stack, ok := alarm["stack_trace"]; ok && stack != nil && stack != "" {
		_, ok = stack.(string)
		if ok {
			alarm["stack_md5"] = fmt.Sprintf("%x", md5.Sum([]byte(stackValue)))
		}
	}
	if alarm["stack_md5"] == nil {
		stackValue = fmt.Sprint(alarm["rasp_id"]) + fmt.Sprint(alarm["plugin_algorithm"]) + fmt.Sprint(alarm["attack_type"])
		alarm["stack_md5"] = fmt.Sprintf("%x", md5.Sum([]byte(stackValue)))
	}
}
