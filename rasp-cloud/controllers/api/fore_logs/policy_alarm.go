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

package fore_logs

import (
	"rasp-cloud/controllers"
	"rasp-cloud/models/logs"
	"encoding/json"
	"rasp-cloud/models"
	"net/http"
	"math"
)

// Operations about policy alarm message
type PolicyAlarmController struct {
	controllers.BaseController
}

// @router /search [post]
func (o *PolicyAlarmController) Search() {
	var param = &logs.SearchPolicyParam{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json decode error", err)
	}
	if param.Data == nil {
		o.ServeError(http.StatusBadRequest, "search data can not be empty")
	}
	if param.Data.AppId != "" {
		_, err := models.GetAppById(param.Data.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "cannot get the app: "+param.Data.AppId)
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
	if param.Page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	if param.Perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}
	content, err := json.Marshal(param.Data)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to encode search data", err)
	}
	var searchData map[string]interface{}
	err = json.Unmarshal(content, &searchData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to decode search data", err)
	}
	delete(searchData, "start_time")
	delete(searchData, "end_time")
	delete(searchData, "app_id")
	total, result, err := logs.SearchLogs(param.Data.StartTime, param.Data.EndTime, searchData, "event_time",
		param.Page, param.Perpage, false, logs.AliasPolicyIndexName+"-"+param.Data.AppId)
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
