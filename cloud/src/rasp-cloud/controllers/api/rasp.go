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

package api

import (
	"math"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"github.com/astaxie/beego/validation"
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
	o.UnmarshalJson(&param)
	if param.Data == nil {
		o.ServeError(http.StatusBadRequest, "search data can not be empty")
	}
	o.ValidPage(param.Page, param.Perpage)
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
	var rasp struct {
		Id          string `json:"id"`
		AppId       string `json:"app_id"`
		RegisterIp  string `json:"register_ip"`
		ExpiredTime int64  `json:"expire_time"`
	}
	o.UnmarshalJson(&rasp)
	if rasp.AppId == "" {
		o.ServeError(http.StatusBadRequest, "the app_id can not be empty")
	}

	if rasp.Id != "" {
		err := models.RemoveRaspById(rasp.Id)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to remove rasp by id", err)
		}
		models.AddOperation(rasp.AppId, models.OperationTypeDeleteRasp, o.Ctx.Input.IP(),
			"Deleted RASP agent by id: "+rasp.Id)
		o.Serve(map[string]interface{}{
			"count": 1,
		})
	} else {
		selector := make(map[string]interface{})
		if rasp.ExpiredTime == 0 && rasp.RegisterIp == "" {
			o.ServeError(http.StatusBadRequest,
				"expire_time and register ip can not be empty at the same time")
		}
		if rasp.RegisterIp != "" {
			selector["register_ip"] = rasp.RegisterIp
			valid := validation.Validation{}
			if result := valid.IP(rasp.RegisterIp, "IP"); !result.Ok {
				o.ServeError(http.StatusBadRequest, "rasp register_ip format error"+result.Error.Message)
			}
		}
		if rasp.ExpiredTime != 0 {
			selector["expire_time"] = rasp.ExpiredTime
			if rasp.ExpiredTime < 0 {
				o.ServeError(http.StatusBadRequest, "expire_time must be greater than 0")
			}
		}
		removedCount, err := models.RemoveRaspBySelector(selector, rasp.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to remove rasp by register ip", err)
		}
		models.AddOperation(rasp.AppId, models.OperationTypeDeleteRasp, o.Ctx.Input.IP(),
			"Deleted RASP agent by register ip: "+rasp.RegisterIp)
		o.Serve(map[string]interface{}{
			"count": removedCount,
		})
	}
}
