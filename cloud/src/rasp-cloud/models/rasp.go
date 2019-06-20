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

import (
	"rasp-cloud/mongo"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"time"
	"strconv"
	"errors"
	"github.com/astaxie/beego/httplib"
	"strings"
	"fmt"
)

type Rasp struct {
	Id                string            `json:"id" bson:"_id,omitempty"`
	AppId             string            `json:"app_id" bson:"app_id,omitempty"`
	Version           string            `json:"version" bson:"version,omitempty"`
	HostName          string            `json:"hostname" bson:"hostname,omitempty"`
	RegisterIp        string            `json:"register_ip" bson:"register_ip,omitempty"`
	Language          string            `json:"language" bson:"language,omitempty"`
	LanguageVersion   string            `json:"language_version" bson:"language_version,omitempty"`
	ServerType        string            `json:"server_type" bson:"server_type,omitempty"`
	ServerVersion     string            `json:"server_version" bson:"server_version,omitempty"`
	RaspHome          string            `json:"rasp_home" bson:"rasp_home,omitempty"`
	PluginVersion     string            `json:"plugin_version" bson:"plugin_version,omitempty"`
	HostType          string            `json:"host_type" bson:"host_type,omitempty"`
	HeartbeatInterval int64             `json:"heartbeat_interval" bson:"heartbeat_interval,omitempty"`
	Online            *bool             `json:"online" bson:"online,omitempty"`
	LastHeartbeatTime int64             `json:"last_heartbeat_time" bson:"last_heartbeat_time,omitempty"`
	RegisterTime      int64             `json:"register_time" bson:"register_time,omitempty"`
	Environ           map[string]string `json:"environ" bson:"environ,omitempty"`
}

const (
	raspCollectionName = "rasp"
)

func init() {
	index := &mgo.Index{
		Key:        []string{"app_id"},
		Unique:     false,
		Background: true,
		Name:       "app_id",
	}
	err := mongo.CreateIndex(raspCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed,
			"failed to create app_id index for rasp collection", err)
	}
	index = &mgo.Index{
		Key:        []string{"register_time"},
		Unique:     false,
		Background: true,
		Name:       "register_time",
	}
	err = mongo.CreateIndex(raspCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed,
			"failed to create register_time index for rasp collection", err)
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
	_, err = mongo.RemoveAll(raspCollectionName, bson.M{"app_id": appId})
	return
}

func FindRasp(selector *Rasp, page int, perpage int) (count int, result []*Rasp, err error) {
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
	if bsonModel["hostname"] != nil {
		realHostname := strings.TrimSpace(fmt.Sprint(bsonModel["hostname"]))
		bsonModel["$or"] = []bson.M{
			{
				"hostname": bson.M{
					"$regex":   realHostname,
					"$options": "$i",
				},
			},
			{
				"register_ip": bson.M{
					"$regex":   realHostname,
					"$options": "$i",
				},
			},
		}
		delete(bsonModel, "hostname")
	}
	if selector.Online != nil {
		delete(bsonModel, "online")
		if *selector.Online {
			bsonModel["$where"] = "this.last_heartbeat_time+this.heartbeat_interval+180 >= " +
				strconv.FormatInt(time.Now().Unix(), 10)
		} else {
			bsonModel["$where"] = "this.last_heartbeat_time+this.heartbeat_interval+180 < " +
				strconv.FormatInt(time.Now().Unix(), 10)
		}
	}
	count, err = mongo.FindAllBySort(raspCollectionName, bsonModel, perpage*(page-1), perpage,
		&result, "-register_time")
	if err == nil {
		for _, rasp := range result {
			if selector.Online != nil {
				rasp.Online = selector.Online
			} else {
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
	if rasp.Environ == nil {
		rasp.Environ = map[string]string{}
	}
}

func RemoveRaspById(id string) (err error) {
	rasp, err := GetRaspById(id)
	if err != nil {
		return err
	}
	if *rasp.Online {
		return errors.New("unable to delete online rasp")
	}
	return mongo.RemoveId(raspCollectionName, id)
}

func RemoveRaspByIds(appId string, ids []string) (int, error) {
	selector := bson.M{
		"_id":    bson.M{"$in": ids},
		"app_id": appId,
		"$where": "this.last_heartbeat_time+this.heartbeat_interval+180 < " +
			strconv.FormatInt(time.Now().Unix(), 10),
	}
	info, err := mongo.RemoveAll(raspCollectionName, selector)
	if err != nil {
		return 0, err
	}
	return info.Removed, nil
}

func RemoveRaspBySelector(selector map[string]interface{}, appId string) (int, error) {
	offlineWhere := ""
	if _, ok := selector["expire_time"]; ok {
		expireTime := strconv.FormatInt(int64(selector["expire_time"].(float64)), 10)
		offlineWhere = "this.last_heartbeat_time+this.heartbeat_interval+180+" + expireTime + "<" +
			strconv.FormatInt(time.Now().Unix(), 10)
	} else {
		offlineWhere = "this.last_heartbeat_time+this.heartbeat_interval+180 < " +
			strconv.FormatInt(time.Now().Unix(), 10)
	}
	param := bson.M{"app_id": appId, "$where": offlineWhere}
	if selector["register_ip"] != nil && selector["register_ip"] != "" {
		param["register_ip"] = selector["register_ip"]
	}
	if selector["host_type"] != nil && selector["host_type"] != "" {
		param["host_type"] = selector["host_type"]
	}
	info, err := mongo.RemoveAll(raspCollectionName, param)
	if err != nil {
		return 0, err
	}
	return info.Removed, nil
}

func RegisterCallback(url string, token string, rasp *Rasp) error {
	var resBody struct {
		Msg string `json:"message"`
	}
	request, err := httplib.Post(url).
		JSONBody(rasp)
	if err != nil {
		return err
	}
	response, err := request.Header("openrasp-token", token).
		SetTimeout(10*time.Second, 10*time.Second).
		Response()
	if err != nil {
		return err
	}
	if response.StatusCode != 200 {
		return errors.New("the status code is error: " + response.Status)
	}
	err = request.ToJSON(&resBody)
	if err != nil {
		return errors.New("response body is invalid: " + err.Error())
	}
	if resBody.Msg != "ok" {
		return errors.New("the message of response body is not ok: " + resBody.Msg)
	}
	return nil
}
