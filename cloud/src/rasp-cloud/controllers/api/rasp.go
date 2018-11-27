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

package api

import (
	"encoding/json"
	"math"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
)

type RaspController struct {
	controllers.BaseController
}

// @router /search [post]
func (o *RaspController) Search() {
	var param struct {
		Data    *models.Rasp `json:"data" `
		Page    int          `json:"page"`
		Perpage int          `json:"perpage"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.Data == nil {
		o.ServeError(http.StatusBadRequest, "search data can not be empty")
	}
	if param.Page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	if param.Perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}
	total, rasps, err := models.FindRasp(param.Data, param.Page, param.Perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get rasp", err)
	}
	if rasps == nil {
		rasps = make([]*models.Rasp, 0)
	}
	var result = make(map[string]interface{})
	result["total"] = total
	result["total_page"] = math.Ceil(float64(total) / float64(param.Perpage))
	result["page"] = param.Page
	result["perpage"] = param.Perpage
	result["data"] = rasps
	o.Serve(result)
}

// @router /delete [post]
func (o *RaspController) Delete() {
	var rasp = &models.Rasp{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, rasp)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if rasp.Id == "" {
		o.ServeError(http.StatusBadRequest, "the id cannot be empty")
	}
	rasp, err = models.GetRaspById(rasp.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get rasp by id", err)
	}
	if *rasp.Online {
		o.ServeError(http.StatusBadRequest, "can not delete online rasp")
	}
	err = models.RemoveRaspById(rasp.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove rasp", err)
	}
	models.AddOperation(rasp.AppId, models.OperationTypeDeleteRasp, o.Ctx.Input.IP(), "deleted the rasp: "+rasp.Id)
	o.ServeWithEmptyData()
}
