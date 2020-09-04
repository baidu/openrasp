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

package es

import (
	"rasp-cloud/conf"
	"rasp-cloud/tools"
)

var attackAlarmTemplate = `
		{
			"template":"openrasp-attack-alarm-*",
			"aliases" : {
        		"real-{index}" : {} 
    		},
			"settings": {
				"analysis": {
					"normalizer": {
						"lowercase_normalizer": {
							"type": "custom",
							"filter": ["lowercase","asciifolding"]
						}
					}     
				}
			},
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
							"ignore_above": 1024,
							"normalizer": "lowercase_normalizer"
						},
						"event_type": {
							"type": "keyword",
							"ignore_above": 256
						},
						"server_hostname": {
							"type": "keyword",
							"ignore_above": 256,
							"normalizer": "lowercase_normalizer"
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
							"type": "keyword",
							"index": false
						},
						"app_id": {
							"type": "keyword",
							"ignore_above": 256
						},
						"rasp_id": {
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
							"type": "keyword",
							"normalizer": "lowercase_normalizer"
						},
						"parameter": {
							"type": "object",
							"properties": {
								"form":{
									"type": "keyword",
									"index":false
								},
								"json":{
									"type": "keyword",
									"index":false
								},
								"multipart":{
									"type": "keyword",
									"index":false
								}
							}
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
var policyAlarmTemplate = `
		{
			"template":"openrasp-policy-alarm-*",
			"aliases" : {
        		"real-{index}" : {} 
    		},
			"settings": {
				"analysis": {
					"normalizer": {
						"lowercase_normalizer": {
							"type": "custom",
							"filter": ["lowercase","asciifolding"]
						}
					}
				}
			},
			"mappings": {
				"policy-alarm": {
					"_all": {
						"enabled": false
					},
					"properties": {
						"@timestamp":{
							"type":"date"
						},
						"event_type": {
							"type": "keyword",
							"ignore_above": 256
						},
						"server_hostname": {
							"type": "keyword",
							"ignore_above": 256,
							"normalizer": "lowercase_normalizer"
						},
						"server_type": {
							"type": "keyword",
							"ignore_above": 64
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
						},
						"app_id": {
							"type": "keyword",
							"ignore_above": 256
						},
						"rasp_id": {
							"type": "keyword",
							"ignore_above": 256
						},
						"event_time": {
							"type": "date"
						},
						"stack_trace": {
							"type": "keyword"
						},
						"policy_id": {
							"type": "long"
						},
						"message": {
							"type": "keyword",
							"normalizer": "lowercase_normalizer"
						},
						"stack_md5": {
							"type": "keyword",
							"ignore_above": 64
						},
						"policy_params": {
							"type": "object",
							"enabled":"false"
						}
					}
				}
			}
		}
		`
var errorAlarmTemplate = `{
			"template":"openrasp-error-alarm-*",
			"aliases" : {
        		"real-{index}" : {} 
    		},
			"settings": {
				"analysis": {
					"normalizer": {
						"lowercase_normalizer": {
							"type": "custom",
							"filter": ["lowercase","asciifolding"]
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
							"type": "keyword",
							"normalizer": "lowercase_normalizer"
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
	`
var reportDataTemplate = `
		{
			"template":"openrasp-report-data-*",
			"aliases" : {
        		"real-{index}" : {} 
    		},
			"mappings": {
				"report-data": {
					"_all": {
						"enabled": false
					},
					"properties": {
						"@timestamp":{
							"type":"date"
         				},
						"time": {
							"type": "date"
						},
						"request_sum": {
							"type": "long"
						},
						"rasp_id": {
							"type": "keyword",
							"ignore_above" : 256
						}
					}
				}
			}
		}
	`

var dependencyDataTemplate = `
		{
			"template":"openrasp-dependency-data-*",
			"aliases" : {
        		"real-{index}" : {} 
    		},
			"settings": {
				"analysis": {
					"normalizer": {
						"lowercase_normalizer": {
							"type": "custom",
							"filter": ["lowercase","asciifolding"]
						}
					}     
				}
			},
			"mappings": {
				"dependency": {
					"_all": {
						"enabled": false
					},
					"properties": {
						"@timestamp":{
							"type":"date"
         				},
						"app_id": {
							"type": "keyword",
							"ignore_above" : 256
						},
						"rasp_id": {
							"type": "keyword",
							"ignore_above" : 256
						},
						"register_ip": {
							"type": "keyword",
							"ignore_above": 128
						},
						"hostname": {
							"type": "keyword",
							"ignore_above" : 256,
							"normalizer": "lowercase_normalizer"
						},
						"path": {
							"type": "keyword",
							"ignore_above" : 1024
						},
						"vendor": {
							"type": "keyword",
							"ignore_above" : 256,
							"normalizer": "lowercase_normalizer"
						},
						"product": {
							"type": "keyword",
							"ignore_above" : 256,
							"normalizer": "lowercase_normalizer"
						},
						"version": {
							"type": "keyword",
							"ignore_above" : 256
						},
						"tag": {
							"type": "keyword",
							"ignore_above" : 1024,
							"normalizer": "lowercase_normalizer"
						},
						"search_string": {
							"type": "keyword",
							"ignore_above" : 1024,
							"normalizer": "lowercase_normalizer"
						},
						"source": {
							"type": "keyword",
							"ignore_above" : 1024,
							"normalizer": "lowercase_normalizer"
						}
					}
				}
			}
		}
	`

var crashDataTemplate = `
		{
			"template":"openrasp-crash-alarm-*",
			"aliases" : {
        		"real-{index}" : {} 
    		},
			"settings": {
				"analysis": {
					"normalizer": {
						"lowercase_normalizer": {
							"type": "custom",
							"filter": ["lowercase","asciifolding"]
						}
					}     
				}
			},
			"mappings": {
				"crash-alarm": {
					"_all": {
						"enabled": false
					},
					"properties": {
						"@timestamp":{
							"type":"date"
         				},
						"app_id": {
							"type": "keyword",
							"ignore_above" : 256
						},
						"rasp_id": {
							"type": "keyword",
							"ignore_above" : 256
						},
						"language": {
							"type": "keyword",
							"ignore_above" : 64
						},
						"relativity": {
							"type": "boolean"
						},
						"event_time": {
							"type": "date"
						},
						"hostname": {
							"type": "keyword",
							"ignore_above": 256,
							"normalizer": "lowercase_normalizer"
						},
						"plugin_name": {
							"type": "keyword",
							"ignore_above": 256
						},
						"register_ip": {
							"type": "keyword",
							"ignore_above": 128
						},
						"crash_message": {
							"type": "keyword"
						},
						"crash_log": {
							"type": "binary"
						}
					}
				}
			}
		}
	`

func init() {
	if *conf.AppConfig.Flag.StartType != conf.StartTypeReset {

		templates := map[string]string{
			"report-data-template":     reportDataTemplate,
			"error-alarm-template":     errorAlarmTemplate,
			"attack-alarm-template":    attackAlarmTemplate,
			"policy-alarm-template":    policyAlarmTemplate,
			"dependency-data-template": dependencyDataTemplate,
			"crash-alarm-template":     crashDataTemplate,
		}
		for name, template := range templates {
			err := CreateTemplate(name, template)
			if err != nil {
				tools.Panic(tools.ErrCodeESInitFailed, "failed to create es template: "+name, err)
			}
		}

	}
}
