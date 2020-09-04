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

package agent

import (
	"gopkg.in/mgo.v2"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"time"
	"rasp-cloud/tools"
)

// Operations about plugin
type HeartbeatController struct {
	controllers.BaseController
}

type heartbeatParam struct {
	RaspId        string `json:"rasp_id"`
	PluginName    string `json:"plugin_name"`
	PluginVersion string `json:"plugin_version"`
	PluginMd5     string `json:"plugin_md5"`
	ConfigTime    int64  `json:"config_time"`
	HostName      string `json:"hostname"`
}

// @router / [post]
func (o *HeartbeatController) Post() {
	var heartbeat heartbeatParam
	o.UnmarshalJson(&heartbeat)
	rasp, err := models.GetRaspById(heartbeat.RaspId)
	if err != nil {
		if err == mgo.ErrNotFound {
			o.ServeErrorWithStatusCode(http.StatusBadRequest,
				tools.ErrRaspNotFound, "can not found the rasp", err)
		} else {
			o.ServeError(http.StatusBadRequest, "failed to get rasp", err)
		}
	}
	rasp.LastHeartbeatTime = time.Now().Unix()
	rasp.PluginVersion = heartbeat.PluginVersion
	rasp.PluginName = heartbeat.PluginName
	rasp.PluginMd5 = heartbeat.PluginMd5
	if heartbeat.HostName != "" {
		if len(heartbeat.HostName) >= 1024 {
			o.ServeError(http.StatusBadRequest, "the length of rasp hostname must be less than 1024")
		}
		rasp.HostName = heartbeat.HostName
	}
	err = models.UpsertRaspById(heartbeat.RaspId, rasp)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update rasp", err)
	}
	pluginMd5 := heartbeat.PluginMd5
	configTime := heartbeat.ConfigTime
	appId := o.Ctx.Input.Header("X-OpenRASP-AppID")
	app, err := models.GetAppById(appId)
	if err != nil || app == nil {
		o.ServeError(http.StatusBadRequest, "cannot get the app", err)
	}

	result := make(map[string]interface{})
	isUpdate := false
	// handle plugin
	selectedPlugin, err := models.GetSelectedPlugin(appId, true)
	if err != nil && err != mgo.ErrNotFound {
		o.ServeError(http.StatusBadRequest, "failed to get selected plugin", err)
	}
	if selectedPlugin != nil {
		if pluginMd5 != selectedPlugin.Md5 {
			isUpdate = true
		}
		if app.ConfigTime > 0 && app.ConfigTime > int64(configTime) {
			isUpdate = true
		}
	}
	if isUpdate {
		whitelistConfig := make(map[string]interface{})
		for _, configItem := range app.WhitelistConfig {
			whiteHookTypes := make([]string, 0, len(configItem.Hook))
			for hookType, isWhite := range configItem.Hook {
				if isWhite {
					whiteHookTypes = append(whiteHookTypes, hookType)
				}
			}
			whitelistConfig[configItem.Url] = whiteHookTypes
		}
		//app.GeneralConfig["algorithm.config"] = selectedPlugin.AlgorithmConfig
		app.GeneralConfig["hook.white"] = whitelistConfig
		result["plugin"] = selectedPlugin
		result["config_time"] = app.ConfigTime
		result["config"] = app.GeneralConfig
	}
	o.Serve(result)
}
