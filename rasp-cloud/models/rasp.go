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

package models

import (
	"rasp-cloud/mongo"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
)

type Rasp struct {
	Id                string `json:"id" bson:"_id"`
	AppId             string `json:"app_id" bson:"app_id"`
	Version           string `json:"version" bson:"version"`
	HostName          string `json:"host_name" bson:"host_name"`
	LocalIp           string `json:"local_ip" bson:"local_ip"`
	Language          string `json:"language" bson:"language"`
	LanguageVersion   string `json:"language_version" bson:"language_version"`
	ServerType        string `json:"server_type" bson:"server_type"`
	ServerVersion     string `json:"server_version" bson:"server_version"`
	RaspHome          string `json:"rasp_home" bson:"rasp_home"`
	PluginVersion     string `json:"plugin_version" bson:"plugin_version"`
	LastHeartbeatTime int64  `json:"last_heartbeat_time" bson:"last_heartbeat_time"`
}

const (
	raspCollectionName = "rasp"
)

func init() {
	count, err := mongo.Count(raspCollectionName)
	if err != nil {
		tools.Panic("failed to get rasp collection count")
	}
	if count <= 0 {
		index := &mgo.Index{
			Key:        []string{"app_id"},
			Unique:     false,
			Background: true,
			Name:       "app_id",
		}
		mongo.CreateIndex(raspCollectionName, index)
		if err != nil {
			tools.Panic("failed to create index for rasp collection")
		}
	}
}

func UpsertRaspById(id string, rasp *Rasp) (error) {
	return mongo.UpsertId(raspCollectionName, id, rasp)
}

func GetRaspByAppId(id string, page int, perpage int) (count int, result []*Rasp, err error) {
	count, err = mongo.FindAll(raspCollectionName, bson.M{"app_id": id}, &result, perpage*(page-1), perpage)
	return
}

func RemoveRaspByAppId(appId string) (err error) {
	return mongo.RemoveId(raspCollectionName, appId)
}

func FindRasp(selector map[string]interface{}, page int, perpage int) (count int, result []*Rasp, err error) {
	count, err = mongo.FindAll(raspCollectionName, bson.M(selector), &result, perpage*(page-1), perpage)
	return
}

func GetRaspById(id string) (rasp *Rasp, err error) {
	err = mongo.FindOne(raspCollectionName, bson.M{"_id": id}, &rasp)
	return
}

func RemoveRaspById(id string) (err error) {
	return mongo.RemoveId(raspCollectionName, id)
}
