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

package agent

import (
	"rasp-cloud/models"
	"encoding/json"
	"net/http"
	"rasp-cloud/controllers"
	"time"
	"github.com/astaxie/beego/validation"
)

type RaspController struct {
	controllers.BaseController
}

// @router / [post]
func (o *RaspController) Post() {
	var rasp = &models.Rasp{}
	rasp.AppId = o.Ctx.Input.Header("X-OpenRASP-AppID")
	err := json.Unmarshal(o.Ctx.Input.RequestBody, rasp)

	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error", err)
	}
	if rasp.Id == "" {
		o.ServeError(http.StatusBadRequest, "rasp id cannot be empty")
	}
	if len(rasp.Id) < 16 {
		o.ServeError(http.StatusBadRequest, "rasp id cannot be less than 16")
	}
	if len(rasp.Id) >= 512 {
		o.ServeError(http.StatusBadRequest, "the length of rasp id must be less than 512")
	}
	if rasp.Version == "" {
		o.ServeError(http.StatusBadRequest, "rasp_version cannot be empty")
	}
	if len(rasp.Version) >= 50 {
		o.ServeError(http.StatusBadRequest, "the length of rasp version must be less than 50")
	}
	if rasp.HostName == "" {
		o.ServeError(http.StatusBadRequest, "rasp hostname cannot be empty")
	}
	if len(rasp.HostName) >= 1024 {
		o.ServeError(http.StatusBadRequest, "the length of rasp hostname must be less than 1024")
	}
	if rasp.LanguageVersion == "" {
		o.ServeError(http.StatusBadRequest, "rasp language_version cannot be empty")
	}
	if len(rasp.LanguageVersion) >= 50 {
		o.ServeError(http.StatusBadRequest, "the length of rasp language version must be less than 50")
	}
	if rasp.Language == "" {
		o.ServeError(http.StatusBadRequest, "rasp language cannot be empty")
	}
	if len(rasp.Language) >= 50 {
		o.ServeError(http.StatusBadRequest, "the length of rasp language must be less than 50")
	}
	if len(rasp.ServerType) >= 256 {
		o.ServeError(http.StatusBadRequest, "the length of rasp server type must be less than 256")
	}
	if len(rasp.ServerVersion) >= 50 {
		o.ServeError(http.StatusBadRequest, "the length of rasp server version must be less than 50")
	}
	if rasp.RegisterIp != "" {
		valid := validation.Validation{}
		if result := valid.IP(rasp.RegisterIp, "IP"); !result.Ok {
			o.ServeError(http.StatusBadRequest, "rasp primary_ip format error: "+result.Error.Message)
		}
	}
	if rasp.HeartbeatInterval <= 0 {
		o.ServeError(http.StatusBadRequest, "heartbeat_interval must be greater than 0")
	}

	rasp.LastHeartbeatTime = time.Now().Unix()
	err = models.UpsertRaspById(rasp.Id, rasp)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to add rasp", err)
	}
	models.AddOperation(rasp.AppId, models.OperationTypeRegisterRasp, o.Ctx.Input.IP(),
		"registered the rasp: " + rasp.Id + ",hostname is: " + rasp.HostName)
	o.Serve(rasp)
}
