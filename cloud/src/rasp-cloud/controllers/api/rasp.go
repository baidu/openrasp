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
	"fmt"
	"encoding/json"
	"github.com/astaxie/beego/validation"
	"encoding/csv"
	"bytes"
	"time"
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

// @router /csv [get]
func (o *RaspController) GeneralCsv() {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "the app_id can not be empty")
	}
	_, rasps, err := models.FindRasp(&models.Rasp{AppId: appId}, 0, 0)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get rasp", err)
	}
	o.Ctx.Output.Header("Content-Type", "text/plain")
	o.Ctx.Output.Header("Content-Disposition", "attachment;filename=rasp.csv")
	writer := &bytes.Buffer{}
	csvWriter := csv.NewWriter(writer)
	csvWriter.Write([]string{"hostname", "register ip", "version", "rasp home", "last heartbeat time", "status"})
	for _, rasp := range rasps {
		onlineMsg := "online"
		if !*rasp.Online {
			onlineMsg = "offline"
		}
		lastTime := time.Unix(rasp.LastHeartbeatTime, 0).Format(time.RFC3339)
		csvWriter.Write([]string{rasp.HostName, rasp.RegisterIp, rasp.Version, rasp.RaspHome, lastTime, onlineMsg})
	}
	csvWriter.Flush()
	o.Ctx.Output.Body(writer.Bytes())
}

// @router /delete [post]
func (o *RaspController) Delete() {
	var rasp struct {
		Id          string `json:"id,omitempty"`
		AppId       string `json:"app_id,omitempty"`
		RegisterIp  string `json:"register_ip,omitempty"`
		ExpiredTime int64  `json:"expire_time,omitempty"`
		HostType    string `json:"host_type,omitempty"`
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
		if rasp.ExpiredTime < 0 {
			o.ServeError(http.StatusBadRequest, "expire_time must be greater than 0")
		}
		if rasp.RegisterIp != "" {
			selector["register_ip"] = rasp.RegisterIp
			valid := validation.Validation{}
			if result := valid.IP(rasp.RegisterIp, "IP"); !result.Ok {
				o.ServeError(http.StatusBadRequest, "rasp register_ip format error"+result.Error.Message)
			}
		}
		data, err := json.Marshal(rasp)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "marshal search param error", err)
		}
		err = json.Unmarshal(data, &selector)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "unmarshal search param error", err)
		}
		removedCount, err := models.RemoveRaspBySelector(selector, rasp.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to remove rasp by register ip", err)
		}
		models.AddOperation(rasp.AppId, models.OperationTypeDeleteRasp, o.Ctx.Input.IP(),
			"Deleted RASP agent by selector: "+fmt.Sprintf("%+v", selector))
		o.Serve(map[string]interface{}{
			"count": removedCount,
		})
	}
}

// @router /batch_delete [post]
func (o *RaspController) BatchDelete() {
	var param struct {
		AppId string   `json:"app_id"`
		Ids   []string `json:"ids"`
	}
	o.UnmarshalJson(&param)

	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "the app_id can not be empty")
	}
	if len(param.Ids) == 0 {
		o.ServeError(http.StatusBadRequest, "the id array can not be empty")
	}
	if len(param.Ids) > 512 {
		o.ServeError(http.StatusBadRequest, "the length of ids array can not be greater than 512")
	}

	for _, id := range param.Ids {
		if len(id) > 512 {
			o.ServeError(http.StatusBadRequest, "the length of id can not be greater than 512")
		}
	}

	count, err := models.RemoveRaspByIds(param.AppId, param.Ids)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove rasp", err)
	}

	models.AddOperation(param.AppId, models.OperationTypeDeleteRasp, o.Ctx.Input.IP(),
		"Batch deleted RASP agent by rasp id: "+fmt.Sprintf("%v", param.Ids))
	o.Serve(map[string]interface{}{
		"count": count,
	})
}
