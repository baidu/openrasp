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

package fore_logs

import (
	"rasp-cloud/controllers"
	"encoding/json"
	"net/http"
	"rasp-cloud/models"
	"rasp-cloud/models/logs"
	"math"
	"time"
	"fmt"
)

// Operations about attack alarm message
type AttackAlarmController struct {
	controllers.BaseController
}

// @router /aggr/time [post]
func (o *AttackAlarmController) AggregationWithTime() {
	var param = &logs.AggrTimeParam{}
	o.UnmarshalJson(&param)
	if param.AppId != "" {
		_, err := models.GetAppById(param.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get the app: "+param.AppId)
		}
	} else {
		param.AppId = "*"
	}
	if param.StartTime <= 0 {
		o.ServeError(http.StatusBadRequest, "start_time must be greater than 0")
	}
	if param.EndTime <= 0 {
		o.ServeError(http.StatusBadRequest, "end_time must be greater than 0")
	}
	if param.StartTime > param.EndTime {
		o.ServeError(http.StatusBadRequest, "start_time cannot be greater than end_time")
	}
	duration := time.Duration(param.EndTime-param.StartTime) * time.Millisecond
	if duration > 366*24*time.Hour {
		o.ServeError(http.StatusBadRequest, "time duration can not be greater than 366 days")
	}
	if param.Interval == "" {
		o.ServeError(http.StatusBadRequest, "interval cannot be empty")
	}
	if param.TimeZone == "" {
		o.ServeError(http.StatusBadRequest, "time_zone cannot be empty")
	}
	if len(param.Interval) > 32 {
		o.ServeError(http.StatusBadRequest, "the length of interval cannot be greater than 32")
	}
	intervals := [...]string{"hour", "day", "month"}
	isValidInterval := false
	for index := range intervals {
		if param.Interval == intervals[index] {
			isValidInterval = true
		}
	}
	if !isValidInterval {
		o.ServeError(http.StatusBadRequest, "the interval must be in "+fmt.Sprintf("%v", intervals))
	}
	if len(param.TimeZone) > 32 {
		o.ServeError(http.StatusBadRequest, "the length of time_zone cannot be greater than 32")
	}
	result, err :=
		logs.AggregationAttackWithTime(param.StartTime, param.EndTime, param.Interval, param.TimeZone, param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get aggregation from es", err)
	}
	o.Serve(result)
}

// @router /aggr/type [post]
func (o *AttackAlarmController) AggregationWithType() {
	var param = &logs.AggrFieldParam{}
	o.UnmarshalJson(&param)
	o.validFieldAggrParam(param)
	result, err :=
		logs.AggregationAttackWithType(param.StartTime, param.EndTime, param.Size, param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get aggregation from es", err)
	}
	o.Serve(result)
}

// @router /aggr/ua [post]
func (o *AttackAlarmController) AggregationWithUserAgent() {
	var param = &logs.AggrFieldParam{}
	o.UnmarshalJson(&param)
	o.validFieldAggrParam(param)
	result, err :=
		logs.AggregationAttackWithUserAgent(param.StartTime, param.EndTime, param.Size, param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get aggregation from es", err)
	}
	o.Serve(result)
}

// @router /search [post]
func (o *AttackAlarmController) Search() {
	param, searchData := o.handleAttackSearchParam()
	total, result, err := logs.SearchLogs(param.Data.StartTime, param.Data.EndTime,
		false, searchData, "event_time", param.Page,
		param.Perpage, false, logs.AttackAlarmInfo.EsAliasIndex+"-"+param.Data.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to search data from es", err)
	}
	o.Serve(map[string]interface{}{
		"total":      total,
		"total_page": math.Ceil(float64(total) / float64(param.Perpage)),
		"page":       param.Page,
		"perpage":    param.Perpage,
		"data":       result,
	})
}

// @router /aggr/vuln [post]
func (o *AttackAlarmController) AggregationVuln() {
	param, searchData := o.handleAttackSearchParam()
	total, result, err := logs.SearchLogs(param.Data.StartTime, param.Data.EndTime,
		true, searchData, "event_time", param.Page,
		param.Perpage, false, logs.AttackAlarmInfo.EsAliasIndex+"-"+param.Data.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to search data from es", err)
	}
	o.Serve(map[string]interface{}{
		"total":      total,
		"total_page": math.Ceil(float64(total) / float64(param.Perpage)),
		"page":       param.Page,
		"perpage":    param.Perpage,
		"data":       result,
	})
}

func (o *AttackAlarmController) handleAttackSearchParam() (param *logs.SearchAttackParam,
	searchData map[string]interface{}) {
	param = &logs.SearchAttackParam{}
	o.UnmarshalJson(&param)
	if param.Data == nil {
		o.ServeError(http.StatusBadRequest, "search data can not be empty")
	}
	if param.Data.AppId != "" {
		_, err := models.GetAppById(param.Data.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "cannot get the app: "+param.Data.AppId, err)
		}
	} else {
		param.Data.AppId = "*"
	}
	if param.Data.StartTime <= 0 {
		o.ServeError(http.StatusBadRequest, "start_time must be greater than 0")
	}
	if param.Data.EndTime <= 0 {
		o.ServeError(http.StatusBadRequest, "end_time must be greater than 0")
	}
	if param.Data.StartTime > param.Data.EndTime {
		o.ServeError(http.StatusBadRequest, "start_time cannot be greater than end_time")
	}
	o.ValidPage(param.Page, param.Perpage)
	content, err := json.Marshal(param.Data)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to encode search data", err)
	}
	err = json.Unmarshal(content, &searchData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to decode search data", err)
	}
	delete(searchData, "start_time")
	delete(searchData, "end_time")
	delete(searchData, "app_id")
	return
}

func (o *AttackAlarmController) validFieldAggrParam(param *logs.AggrFieldParam) {
	if param.AppId != "" {
		_, err := models.GetAppById(param.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "cannot get the app: "+param.AppId, err)
		}
	} else {
		param.AppId = "*"
	}
	if param.StartTime <= 0 {
		o.ServeError(http.StatusBadRequest, "start_time must be greater than 0")
	}
	if param.EndTime <= 0 {
		o.ServeError(http.StatusBadRequest, "end_time must be greater than 0")
	}
	if param.StartTime > param.EndTime {
		o.ServeError(http.StatusBadRequest, "start_time cannot be greater than end_time")
	}
	duration := time.Duration(param.EndTime-param.StartTime) * time.Millisecond
	if duration > 366*24*time.Hour {
		o.ServeError(http.StatusBadRequest, "time duration can not be greater than 366 days")
	}
	if param.Size <= 0 {
		o.ServeError(http.StatusBadRequest, "size must be greater than 0")
	}

	if param.Size > 1024 {
		o.ServeError(http.StatusBadRequest, "size can not be greater than 1024")
	}
}
