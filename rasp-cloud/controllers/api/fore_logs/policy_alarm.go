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
)

// Operations about policy alarm message
type PolicyAlarmController struct {
	controllers.BaseController
}

// @router /search [post]
func (o *PolicyAlarmController) Search() {
	var param = &logs.SearchLogParam{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if param.AppId != "" {
		_, err := models.GetAppById(param.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "cannot get the app: "+param.AppId)
		}
	} else {
		param.AppId = "*"
	}
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json decode error： "+err.Error())
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
	if param.Page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	if param.Perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}
	total, result, err := logs.SearchLogs(param.StartTime, param.EndTime, param.Data, "event_time",
		param.Page, param.Perpage, false, logs.AliasPolicyIndexName+"-"+param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to search data from es: "+err.Error())
	}
	o.Serve(map[string]interface{}{
		"total": total,
		"data":  result,
	})
}
