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
	"time"
	"strconv"
)

type Rasp struct {
	Id                string `json:"id" bson:"_id,omitempty"`
	AppId             string `json:"app_id" bson:"app_id,omitempty"`
	Version           string `json:"version" bson:"version,omitempty"`
	HostName          string `json:"hostname" bson:"hostname,omitempty"`
	RegisterIp        string `json:"register_ip" bson:"register_ip,omitempty"`
	Language          string `json:"language" bson:"language,omitempty"`
	LanguageVersion   string `json:"language_version" bson:"language_version,omitempty"`
	ServerType        string `json:"server_type" bson:"server_type,omitempty"`
	ServerVersion     string `json:"server_version" bson:"server_version,omitempty"`
	RaspHome          string `json:"rasp_home" bson:"rasp_home,omitempty"`
	PluginVersion     string `json:"plugin_version" bson:"plugin_version,omitempty"`
	HeartbeatInterval int64  `json:"heartbeat_interval" bson:"heartbeat_interval,omitempty"`
	Online            *bool  `json:"online" bson:"online,omitempty"`
	LastHeartbeatTime int64  `json:"last_heartbeat_time" bson:"last_heartbeat_time,omitempty"`
}

const (
	raspCollectionName = "rasp"
)

func init() {
	count, err := mongo.Count(raspCollectionName)
	if err != nil {
		tools.Panic("failed to get rasp collection count", err)
	}
	if count <= 0 {
		index := &mgo.Index{
			Key:        []string{"app_id"},
			Unique:     false,
			Background: true,
			Name:       "app_id",
		}
		err = mongo.CreateIndex(raspCollectionName, index)
		if err != nil {
			tools.Panic("failed to create index for rasp collection", err)
		}
	}
}

func UpsertRaspById(id string, rasp *Rasp) (error) {
	return mongo.UpsertId(raspCollectionName, id, rasp)
}

func GetRaspByAppId(id string, page int, perpage int) (count int, result []*Rasp, err error) {
	count, err = mongo.FindAll(raspCollectionName, bson.M{"app_id": id}, &result, perpage*(page-1), perpage)
	if err == nil {
		for _, rasp := range result {
			HandleRasp(rasp)
		}
	}
	return
}

func RemoveRaspByAppId(appId string) (err error) {
	return mongo.RemoveAll(raspCollectionName, bson.M{"app_id": appId})
}

func FindRasp(selector *Rasp, page int, perpage int) (count int, result []*Rasp, err error) {
	if selector.Online != nil {
		var bsonContent []byte
		bsonContent, err = bson.Marshal(selector)
		if err != nil {
			return
		}
		bsonModel := bson.M{}
		err = bson.Unmarshal(bsonContent, &bsonModel)
		if err != nil {
			return
		}
		delete(bsonModel, "online")
		if *selector.Online {
			bsonModel["$where"] = "this.last_heartbeat_time+this.heartbeat_interval+180 >= " +
				strconv.FormatInt(time.Now().Unix(), 10)
		} else {
			bsonModel["$where"] = "this.last_heartbeat_time+this.heartbeat_interval+180 < " +
				strconv.FormatInt(time.Now().Unix(), 10)
		}
		count, err = mongo.FindAll(raspCollectionName, bsonModel, &result, perpage*(page-1), perpage)
		if err == nil {
			for _, rasp := range result {
				rasp.Online = selector.Online
			}
		}
	} else {
		count, err = mongo.FindAllBySort(raspCollectionName, selector, perpage*(page-1), perpage,
			&result, "id")
		if err == nil {
			for _, rasp := range result {
				HandleRasp(rasp)
			}
		}
	}
	return
}

func GetRaspById(id string) (rasp *Rasp, err error) {
	err = mongo.FindId(raspCollectionName, id, &rasp)
	if err == nil {
		HandleRasp(rasp)
	}
	return
}

func HandleRasp(rasp *Rasp) {
	var online bool
	heartbeatInterval := rasp.HeartbeatInterval + 180
	if time.Now().Unix()-rasp.LastHeartbeatTime > heartbeatInterval {
		online = false
	} else {
		online = true
	}
	rasp.Online = &online
}

func RemoveRaspById(id string) (err error) {
	return mongo.RemoveId(raspCollectionName, id)
}
