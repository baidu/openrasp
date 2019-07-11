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
	"time"
	"rasp-cloud/conf"
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/validation"
)

type RaspController struct {
	controllers.BaseController
}

// @router / [post]
func (o *RaspController) Post() {
	var rasp = &models.Rasp{}
	rasp.AppId = o.Ctx.Input.Header("X-OpenRASP-AppID")
	o.UnmarshalJson(rasp)
	if rasp.Id == "" {
		o.ServeError(http.StatusBadRequest, "rasp id cannot be empty")
	}
	if len(rasp.Id) < 16 || len(rasp.Id) > 512 {
		o.ServeError(http.StatusBadRequest, "the length of rasp id must be between 16~512")
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
	if rasp.ServerType == "" {
		o.ServeError(http.StatusBadRequest, "the server_type can not be empty")
	}
	if len(rasp.ServerType) >= 256 {
		o.ServeError(http.StatusBadRequest, "the length of rasp server type must be less than 256")
	}
	if rasp.ServerVersion == "" {
		o.ServeError(http.StatusBadRequest, "the server version can not be empty")
	}
	if len(rasp.ServerVersion) >= 50 {
		o.ServeError(http.StatusBadRequest, "the length of rasp server version must be less than 50")
	}
	if len(rasp.HostType) >= 256 {
		o.ServeError(http.StatusBadRequest, "the length of host type must be less than 256")
	}
	if rasp.RegisterIp == "" {
		o.ServeError(http.StatusBadRequest, "the register ip can not be empty")
	}
	valid := validation.Validation{}
	if result := valid.IP(rasp.RegisterIp, "IP"); !result.Ok {
		o.ServeError(http.StatusBadRequest, "rasp register_ip format error: "+result.Error.Message)
	}
	if rasp.HeartbeatInterval <= 0 {
		o.ServeError(http.StatusBadRequest, "heartbeat_interval must be greater than 0")
	}

	if rasp.Environ == nil {
		rasp.Environ = map[string]string{}
	}

	envTotal := 0
	environ := map[string]string{}
	isWarnEnv := false
	for k, v := range rasp.Environ {
		envTotal += len(k)
		envTotal += len(v)
		if envTotal > 100000 {
			if !isWarnEnv {
				beego.Warn("the total length of environment variable is greater than 100000, " +
					"can not store all environment variable for rasp: " + rasp.HostName + " (" + rasp.RaspHome + ")")
				isWarnEnv = true
			}
			envTotal -= len(k)
			envTotal -= len(v)
			continue
		}
		environ[k] = v
	}
	online := true
	rasp.Online = &online
	rasp.Environ = environ
	rasp.LastHeartbeatTime = time.Now().Unix()
	rasp.RegisterTime = time.Now().Unix()
	err := models.UpsertRaspById(rasp.Id, rasp)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to add rasp", err)
	}
	if len(conf.AppConfig.RegisterCallbackUrl) > 0 {
		go func() {
			err = models.RegisterCallback(conf.AppConfig.RegisterCallbackUrl, conf.AppConfig.RegisterCallbackToken, rasp)
			if err != nil {
				beego.Error("failed to send register callback to url: " +
					conf.AppConfig.RegisterCallbackUrl + ", " + err.Error())
			}
		}()
	}
	models.AddOperation(rasp.AppId, models.OperationTypeRegisterRasp, o.Ctx.Input.IP(),
		"New RASP agent registered from "+rasp.HostName+": "+rasp.Id, "")
	o.Serve(rasp)
}
