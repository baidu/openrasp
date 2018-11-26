//Copyright 2017-2018 Baidu Inc.
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
	"github.com/astaxie/beego"
	"rasp-cloud/tools"
	"github.com/astaxie/beego/logs"
	"os"
	"rasp-cloud/es"
	"time"
	"encoding/json"
	"github.com/olivere/elastic"
	"context"
	"path"
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
		AppId        string   `json:"app_id,omitempty"`
		StartTime    int64    `json:"start_time"`
		EndTime      int64    `json:"end_time"`
		RaspId       string   `json:"rasp_id,omitempty"`
		HostName     string   `json:"server_hostname,omitempty"`
		AttackSource string   `json:"attack_source,omitempty"`
		AttackUrl    string   `json:"url,omitempty"`
		AttackType   []string `json:"attack_type,omitempty"`
	} `json:"data"`
}

type SearchPolicyParam struct {
	Page    int `json:"page"`
	Perpage int `json:"perpage"`
	Data *struct {
		AppId     string   `json:"app_id,omitempty"`
		StartTime int64    `json:"start_time"`
		EndTime   int64    `json:"end_time"`
		RaspId    string   `json:"rasp_id,omitempty"`
		HostName  string   `json:"server_hostname,omitempty"`
		PolicyId  []string `json:"policy_id,omitempty"`
	} `json:"data"`
}

var (
	AttackAlarmType     = "attack-alarm"
	PolicyAlarmType     = "policy-alarm"
	AddAlarmFunc        func(string, map[string]interface{}) error
	esAttackAlarmBuffer chan map[string]interface{}
	esPolicyAlarmBuffer chan map[string]interface{}
	alarmFileLoggers    = make(map[string]*logs.BeeLogger)
)

func init() {
	es.RegisterTTL(24*365*time.Hour, AliasAttackIndexName+"-*")
	es.RegisterTTL(24*365*time.Hour, AliasPolicyIndexName+"-*")
	if beego.AppConfig.String("RaspLogMode") == "file" ||
		beego.AppConfig.String("RaspLogMode") == "" {
		AddAlarmFunc = AddLogWithFile
		initRaspLoggers()
	} else if beego.AppConfig.String("RaspLogMode") == "es" {
		startEsAlarmLogPush()
		AddAlarmFunc = AddLogWithES
	} else {
		tools.Panic("Unrecognized the value of RaspLogMode config")
	}
	alarmBufferSize := beego.AppConfig.DefaultInt("AlarmBufferSize", 300)
	esAttackAlarmBuffer = make(chan map[string]interface{}, alarmBufferSize)
	esPolicyAlarmBuffer = make(chan map[string]interface{}, alarmBufferSize)
}

func initRaspLoggers() {
	alarmFileLoggers[AttackAlarmType] = initAlarmFileLogger("/openrasp-logs/attack-alarm", "attack.log")
	alarmFileLoggers[PolicyAlarmType] = initAlarmFileLogger("/openrasp-logs/policy-alarm", "policy.log")
}

func initAlarmFileLogger(dirName string, fileName string) *logs.BeeLogger {
	currentPath, err := tools.GetCurrentPath()
	if err != nil {
		tools.Panic("failed to init alarm logger: " + err.Error())
	}
	dirName = currentPath + dirName
	if isExists, _ := tools.PathExists(dirName); !isExists {
		err := os.MkdirAll(dirName, os.ModePerm)
		if err != nil {
			tools.Panic("failed to init alarm logger: " + err.Error())
		}
	}

	logger := logs.NewLogger()
	logPath := path.Join(dirName, fileName)
	err = logger.SetLogger(tools.AdapterAlarmFile,
		`{"filename":"`+logPath+`", "daily":true, "maxdays":10, "perm":"0777"}`)
	if err != nil {
		tools.Panic("failed to init alarm logger: " + err.Error())
	}
	return logger
}

func startEsAlarmLogPush() {
	go func() {
		defer func() {
			if r := recover(); r != nil {
				beego.Error("failed to push es alarm log: ", r)
			}
		}()
		for {
			select {
			case alarm := <-esAttackAlarmBuffer:
				alarms := make([]map[string]interface{}, 0, 200)
				alarms = append(alarms, alarm)
				for len(esAttackAlarmBuffer) > 0 && len(alarms) < 200 {
					alarm := <-esAttackAlarmBuffer
					alarms = append(alarms, alarm)
				}
				es.BulkInsert(AttackAlarmType, alarms)
			case alarm := <-esPolicyAlarmBuffer:
				alarms := make([]map[string]interface{}, 0, 200)
				alarms = append(alarms, alarm)
				for len(esPolicyAlarmBuffer) > 0 && len(alarms) < 200 {
					alarm := <-esPolicyAlarmBuffer
					alarms = append(alarms, alarm)
				}
				es.BulkInsert(PolicyAlarmType, alarms)
			}
		}
	}()
}

func AddLogWithFile(alarmType string, alarm map[string]interface{}) error {
	if logger, ok := alarmFileLoggers[alarmType]; ok && logger != nil {
		content, err := json.Marshal(alarm)
		if err != nil {
			return err
		}
		_, err = logger.Write(content)
		if err != nil {
			logs.Error("failed to write rasp log: " + err.Error())
			return err
		}
	} else {
		logs.Error("failed to write rasp log ,unrecognized log type: " + alarmType)
	}
	return nil
}

func AddLogWithES(alarmType string, alarm map[string]interface{}) error {
	if alarmType == AttackAlarmType {
		select {
		case esAttackAlarmBuffer <- alarm:
		default:
		}
	} else if alarmType == PolicyAlarmType {
		select {
		case esPolicyAlarmBuffer <- alarm:
		default:
		}
	}
	return nil
}

func SearchLogs(startTime int64, endTime int64, query map[string]interface{}, sortField string, page int,
	perpage int, ascending bool, index ...string) (int64, []map[string]interface{}, error) {
	var total int64
	queries := make([]elastic.Query, 0, len(query)+1)
	if query != nil {
		for key, value := range query {
			if key == "attack_type" {
				if v, ok := value.([]interface{}); ok {
					queries = append(queries, elastic.NewTermsQuery(key, v...))
				} else {
					queries = append(queries, elastic.NewTermQuery(key, value))
				}
			} else if key == "policy_id" {
				if v, ok := value.([]interface{}); ok {
					queries = append(queries, elastic.NewTermsQuery(key, v...))
				} else {
					queries = append(queries, elastic.NewTermQuery(key, value))
				}
			} else {
				queries = append(queries, elastic.NewTermQuery(key, value))
			}
		}
	}
	queries = append(queries, elastic.NewRangeQuery("event_time").Gte(startTime).Lte(endTime))
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
	defer cancel()
	queryResult, err := es.ElasticClient.Search(index...).
		Query(elastic.NewBoolQuery().Must(queries...)).
		Sort(sortField, ascending).
		From((page - 1) * perpage).Size(perpage).Do(ctx)
	if err != nil {
		return 0, nil, err
	}
	result := make([]map[string]interface{}, 0)
	if queryResult != nil && queryResult.Hits != nil && queryResult.Hits.Hits != nil {
		hits := queryResult.Hits.Hits
		total = queryResult.Hits.TotalHits
		result = make([]map[string]interface{}, len(hits))
		for index, item := range hits {
			result[index] = make(map[string]interface{})
			err := json.Unmarshal(*item.Source, &result[index])
			result[index]["id"] = item.Id
			if err != nil {
				return 0, nil, err
			}
		}
	}
	return total, result, nil
}
