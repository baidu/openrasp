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
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"net/http"
	"gopkg.in/mgo.v2"
	"strings"
)

type ServerController struct {
	controllers.BaseController
}

// @router /url/get [post]
func (o *ServerController) GetUrl() {
	serverUrl, err := models.GetServerUrl()
	if err != nil {
		if mgo.ErrNotFound == err {
			o.Serve(models.ServerUrl{AgentUrls: []string{}})
			return
		}
		o.ServeError(http.StatusBadRequest, "failed to get serverUrl", err)
	}
	o.Serve(serverUrl)
}

// @router /url [post]
func (o *ServerController) PutUrl() {
	var serverUrl = &models.ServerUrl{}
	o.UnmarshalJson(&serverUrl)

	if !validHttpUrl(serverUrl.PanelUrl) {
		o.ServeError(http.StatusBadRequest, "Invalid panel url: "+serverUrl.PanelUrl)
	}

	if len(serverUrl.PanelUrl) > 512 {
		o.ServeError(http.StatusBadRequest, "the length of panel url cannot be greater than 512")
	}

	if len(serverUrl.AgentUrls) > 1024 {
		o.ServeError(http.StatusBadRequest, "the count of agent url cannot be greater than 1024")
	}

	if serverUrl.AgentUrls != nil {
		for _, agentUrl := range serverUrl.AgentUrls {
			if len(agentUrl) > 512 {
				o.ServeError(http.StatusBadRequest, "the length of agent url cannot be greater than 512")
			}
			if !validHttpUrl(agentUrl) {
				o.ServeError(http.StatusBadRequest, "Invalid agent url: "+agentUrl)
			}
		}
	}

	err := models.PutServerUrl(serverUrl)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to put server url", err)
	}

	o.Serve(serverUrl)
}

func validHttpUrl(url string) bool {
	return strings.HasPrefix(url, "http://") || strings.HasPrefix(url, "https://")
}
