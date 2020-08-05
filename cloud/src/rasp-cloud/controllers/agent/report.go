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

package agent

import (
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
)

type ReportController struct {
	controllers.BaseController
}

// @router / [post]
func (o *ReportController) Post() {
	var reportData *models.ReportData
	o.UnmarshalJson(&reportData)
	if reportData.RaspId == "" {
		o.ServeError(http.StatusBadRequest, "rasp_id cannot be empty")
	}
	rasp, err := models.GetRaspById(reportData.RaspId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get rasp", err)
	}
	if reportData.Time <= 0 {
		o.ServeError(http.StatusBadRequest, "time param must be greater than 0")
	}
	if reportData.RequestSum < 0 {
		o.ServeError(http.StatusBadRequest, "request_sum param cannot be less than 0")
	}
	err = models.AddReportData(reportData, rasp.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to insert report data", err)
	}
	o.Serve(reportData)
}
