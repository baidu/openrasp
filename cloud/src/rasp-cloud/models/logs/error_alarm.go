package logs

import (
	"github.com/astaxie/beego"
	"time"
	"rasp-cloud/conf"
)

var (
	ErrorAlarmInfo = AlarmLogInfo{
		EsType:       "error-alarm",
		EsIndex:      "openrasp-error-alarm",
		EsAliasIndex: "real-openrasp-error-alarm",
		TtlTime:      24 * 365 * time.Hour,
		AlarmBuffer:  make(chan map[string]interface{}, conf.AppConfig.AlarmBufferSize),
		FileLogger:   initAlarmFileLogger("/openrasp-logs/error-alarm", "error.log"),
		EsMapping:
		`{
			"settings": {
				"analysis": {
					"normalizer": {
						"lowercase_normalizer": {
							"type": "custom",
							"filter": ["lowercase"]
						}
					}     
				}
			},
			"mappings": {
				"error-alarm": {
					"_all": {
						"enabled": false
					},
					"properties": {
						"@timestamp":{
							"type":"date"
						},
						"app_id": {
							"type": "keyword",
							"ignore_above": 256
						},
						"rasp_id": {
							"type": "keyword",
							"ignore_above": 256
						},
						"message": {
							"type": "keyword"
						},
						"level": {
							"type": "keyword",
							"ignore_above": 64
						},
						"err_code":{
							"type": "long"
						},
						"stack_trace":{
							"type": "keyword"
						},
						"pid":{
							"type": "long"
						},
						"event_time": {
							"type": "date"
						},
						"server_hostname": {
							"type": "keyword",
							"ignore_above": 256,
							"normalizer": "lowercase_normalizer"
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
	`,
	}
)

func init() {
	registerAlarmInfo(&ErrorAlarmInfo)
}

func AddErrorAlarm(alarm map[string]interface{}) error {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to add error alarm: ", r)
		}
	}()
	return AddAlarmFunc(ErrorAlarmInfo.EsType, alarm)
}
