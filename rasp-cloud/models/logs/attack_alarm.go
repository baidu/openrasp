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
)

type AttackAlarm struct {
	content string
}

var (
	AttackIndexName      = "openrasp-attack-alarm"
	AliasAttackIndexName = "real-openrasp-attack-alarm"
	AttackEsMapping      = `
	{
		"mappings": {
			"attack-alarm": {
				"_all": {
					"enabled": false
				},
				"properties": {
					"@timestamp":{
                   		"type":"date"
         			},
					"request_method": {
						"type": "keyword",
						"ignore_above": 50
					},
					"target": {
						"type": "keyword",
						"ignore_above": 256
					},
					"server_ip": {
						"type": "keyword",
						"ignore_above": 256
					},
					"client_ip": {
						"type": "keyword",
						"ignore_above": 256
					},
					"referer": {
						"type": "keyword",
						"ignore_above": 256
					},
					"user_agent": {
						"type": "keyword",
						"ignore_above": 512
					},
					"attack_source": {
						"type": "keyword",
						"ignore_above": 256
					},
					"path": {
						"type": "keyword",
						"ignore_above": 256
					},
					"url": {
						"type": "keyword",
						"ignore_above": 256
					},
					"event_type": {
						"type": "keyword",
						"ignore_above": 256
					},
					"server_hostname": {
						"type": "keyword",
						"ignore_above": 256
					},
					"stack_md5": {
						"type": "keyword",
						"ignore_above": 64
					},
					"server_type": {
						"type": "keyword",
						"ignore_above": 256
					},
					"server_version": {
						"type": "keyword",
						"ignore_above": 256
					},
					"request_id": {
						"type": "keyword",
						"ignore_above": 256
					},
					"body": {
						"type": "keyword"
					},
					"app_id": {
						"type": "keyword",
						"ignore_above": 256
					},
					"rasp_id": {
						"type": "keyword",
						"ignore_above": 256
					},
					"local_ip": {
						"type": "keyword",
						"ignore_above": 256
					},
					"event_time": {
						"type": "date"
					},
					"stack_trace": {
						"type": "keyword"
					},
					"intercept_state": {
						"type": "keyword",
						"ignore_above": 64
					},
					"attack_type": {
						"type": "keyword",
						"ignore_above": 256
					},
					"attack_location": {
						"type": "object",
						"properties": {
							"location_zh_cn":{
								"type": "keyword",
								"ignore_above": 256
							},
							"location_en":{
								"type": "keyword",
								"ignore_above": 256
							},
							"longitude":{
								"type": "double"
							},
							"latitude":{
								"type": "double"
							}
						}
					},
					"plugin_algorithm":{
						"type": "keyword",
						"ignore_above": 256
					},
					"plugin_name": {
						"type": "keyword",
						"ignore_above": 256
					},
					"plugin_confidence": {
						"type": "short"
					},
					"attack_params": {
						"type": "object",
						"enabled":"false"
					},
					"plugin_message": {
						"type": "keyword"
					},
					"server_nic": {
						"type": "nested",
						"properties": {
							"name": {
								"type": "keyword",
								"ignore_above": 256
							},
							"ip": {
								"type": "keyword",
								"ignore_above": 256
							}
						}
					}
				}
			}
		}
	}
	`
	geoIpDbPath string
)

func init() {
	currentPath, err := tools.GetCurrentPath()
	if err != nil {
		tools.Panic("failed to get current directory path", err)
	}
	geoIpDbPath = currentPath + "/geoip/GeoLite2-City.mmdb"
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
	return AddAlarmFunc(AttackAlarmType, alarm)
}

func setAlarmLocation(alarm map[string]interface{}) {
	if attackSource, ok := alarm["attack_source"]; ok && attackSource != nil {
		_, ok = attackSource.(string)
		if ok {
			db, err := geoip2.Open(geoIpDbPath)
			if err != nil {
				beego.Error("failed to open geoip database: " + err.Error())
			}
			defer db.Close()
			attackIp := net.ParseIP(attackSource.(string))
			record, err := db.City(attackIp)
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
	aggrResult, err := es.ElasticClient.Search(AliasAttackIndexName + "-" + appId).
		Query(elastic.NewBoolQuery().Must(timeQuery)).
		Aggregation(timeAggrName, timeAggr).
		Size(0).
		Do(ctx)
	if err != nil {
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
						} else if item.Key == "info" {
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
	uaAggr := elastic.NewTermsAggregation().Field("user_agent").Size(size).OrderByCount(false)
	timeQuery := elastic.NewRangeQuery("event_time").Gte(startTime).Lte(endTime)
	aggrName := "aggr_ua"
	aggrResult, err := es.ElasticClient.Search(AliasAttackIndexName + "-" + appId).
		Query(timeQuery).
		Aggregation(aggrName, uaAggr).
		Size(0).
		Do(ctx)
	if err != nil {
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
	aggrResult, err := es.ElasticClient.Search(AliasAttackIndexName + "-" + appId).
		Query(timeQuery).
		Aggregation(aggrName, typeAggr).
		Size(0).
		Do(ctx)
	if err != nil {
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
