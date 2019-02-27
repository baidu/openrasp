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

package models

import "rasp-cloud/mongo"

const (
	serverUrlId               = "0"
	serverUrlCollectionName   = "server_url"
	serverAgentCollectionName = "server_agent"
)

type ServerUrl struct {
	PanelUrl  string   `json:"panel_url" bson:"panel_url"`
	AgentUrls []string `json:"agent_urls" bson:"agent_urls"`
}

func GetServerUrl() (serverUrl *ServerUrl, err error) {
	err = mongo.FindId(serverUrlCollectionName, serverUrlId, &serverUrl)
	if err == nil && serverUrl.AgentUrls == nil {
		serverUrl.AgentUrls = []string{}
	}
	return
}

func PutServerUrl(serverUrl *ServerUrl) (error) {
	return mongo.UpsertId(serverUrlCollectionName, serverUrlId, &serverUrl)
}
