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

package models

import (
	"rasp-cloud/conf"
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
	"github.com/astaxie/beego"
)

type Rasp struct {
	Id                string            `json:"id" bson:"_id,omitempty"`
	AppId             string            `json:"app_id" bson:"app_id,omitempty"`
	StrategyId		  string			`json:"strategy_id" bson:"strategy_id,omitempty"`
	Version           string            `json:"version" bson:"version,omitempty"`
	Os                string            `json:"os" bson:"os,omitempty"`
	HostName          string            `json:"hostname" bson:"hostname,omitempty"`
	RegisterIp        string            `json:"register_ip" bson:"register_ip,omitempty"`
	Language          string            `json:"language" bson:"language,omitempty"`
	LanguageVersion   string            `json:"language_version" bson:"language_version,omitempty"`
	ServerType        string            `json:"server_type" bson:"server_type,omitempty"`
	ServerVersion     string            `json:"server_version" bson:"server_version,omitempty"`
	RaspHome          string            `json:"rasp_home" bson:"rasp_home,omitempty"`
	PluginVersion     string            `json:"plugin_version" bson:"plugin_version,omitempty"`
	PluginName        string            `json:"plugin_name" bson:"plugin_name,omitempty"`
	PluginMd5         string            `json:"plugin_md5" bson:"plugin_md5,omitempty"`
	HostType          string            `json:"host_type" bson:"host_type,omitempty"`
	HeartbeatInterval int64             `json:"heartbeat_interval" bson:"heartbeat_interval,omitempty"`
	Online            *bool             `json:"online" bson:"online,omitempty"`
	LastHeartbeatTime int64             `json:"last_heartbeat_time" bson:"last_heartbeat_time,omitempty"`
	RegisterTime      int64             `json:"register_time" bson:"register_time,omitempty"`
	Environ           map[string]string `json:"environ" bson:"environ,omitempty"`
	Description       string            `json:"description" bson:"description,omitempty"`
	HostNameList      []string          `json:"hostname_list" bson:"hostname_list,omitempty"`
}

type RecordCount struct {
	Id                string            `json:"version" bson:"_id"`
	Count             int64    			`json:"count" bson:"count"`
}

const (
	raspCollectionName = "rasp"
	defaultOfflineInterval = 180
)

var (
	HasOfflineHosts       map[string]float64
	OfflineInterval       int64
	OfflineIntervalString string
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
	// read offline interval
	OfflineInterval = conf.AppConfig.OffLineInterval
	// check offline Interval valid
	if OfflineInterval < 30 || OfflineInterval > 3600 {
		beego.Warning("OfflineInterval must between 30 and 3600, set OfflineInterval to default value")
		OfflineInterval = int64(defaultOfflineInterval)
	}
	OfflineIntervalString = strconv.FormatInt(OfflineInterval, 10)
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
				"rasp_home": bson.M{
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
			{
				"_id": realHostname,
			},
			{
				"version": bson.M{
					"$regex":   realHostname,
					"$options": "$i",
				},
			},
			{
				"description": bson.M{
					"$regex":   realHostname,
					"$options": "$i",
				},
			},
			{
				"os": bson.M{
					"$regex":   realHostname,
					"$options": "$i",
				},
			},
		}
		delete(bsonModel, "hostname")
	}
	if bsonModel["hostname_list"] != nil {
		delete(bsonModel, "hostname")
		realHostnameList := selector.HostNameList
		bsonModel["hostname"] = bson.M{
			"$in":   realHostnameList,
		}
		delete(bsonModel, "hostname_list")
	}

	if selector.Online != nil {
		delete(bsonModel, "online")
		if *selector.Online {
			bsonModel["$where"] = "this.last_heartbeat_time+this.heartbeat_interval+" + OfflineIntervalString + " >= " +
				strconv.FormatInt(time.Now().Unix(), 10)
		} else {
			bsonModel["$where"] = "this.last_heartbeat_time+this.heartbeat_interval+" + OfflineIntervalString + " < " +
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

func FindRaspVersion(selector *Rasp) (result []*RecordCount, err error) {
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
	if bsonModel["app_id"] != nil {
		app_id := strings.TrimSpace(fmt.Sprint(bsonModel["app_id"]))
		version := strings.TrimSpace(fmt.Sprint(bsonModel["version"]))
		language := strings.TrimSpace(fmt.Sprint(bsonModel["language"]))
		onlineFlag := bson.M{"$gt": 0}
		if selector.Online != nil {
			if *selector.Online {
				onlineFlag = bson.M{"$gt": time.Now().Unix() - OfflineInterval}
			} else {
				onlineFlag = bson.M{"$lt": time.Now().Unix() - OfflineInterval}
			}
		}
		var matchCase bson.M
		matchCase = bson.M{"$and": []bson.M{
			{"app_id": app_id},
			{"onlineTime": onlineFlag},
		}}
		if version != "<nil>" {
			and := matchCase["$and"].([]bson.M)
			and = append(and, bson.M{"version": version})
			matchCase["$and"] = and
		}
		if language != "<nil>" {
			and := matchCase["$and"].([]bson.M)
			and = append(and, bson.M{"language": language})
			matchCase["$and"] = and
		}
		Operations := []bson.M {
			{
				"$project": bson.M {
					"app_id": 1,
					"version": 1,
					"language": 1,
					"last_heartbeat_time": 1,
					"heartbeat_interval": 1,
					"onlineTime": bson.M {
						"$add": []string{"$last_heartbeat_time", "$heartbeat_interval"}}},
			},
			{
				"$match": matchCase,
			},
			{
				"$group": bson.M{
					"_id":   "$version",
					"count": bson.M{"$sum": 1},
				},
			},
			{
				"$match": bson.M{
					"count": bson.M{"$gt": 0 }},
			},
			{
				"$sort": bson.M{
					"_id": -1,
				},
			},
		}
		err = mongo.FindSelectWithAggregation(raspCollectionName, Operations, &result)
		return
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
	heartbeatInterval := rasp.HeartbeatInterval + OfflineInterval
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
	err = mongo.RemoveId(raspCollectionName, id)
	if err != nil {
		return err
	}
	return RemoveDependencyByRasp(rasp.AppId, rasp.Id)
}

func RemoveRaspByIds(appId string, ids []string) (int, error) {
	selector := bson.M{
		"_id":    bson.M{"$in": ids},
		"app_id": appId,
		"$where": "this.last_heartbeat_time+this.heartbeat_interval+" + OfflineIntervalString + "< " +
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
		offlineWhere = "this.last_heartbeat_time+this.heartbeat_interval+" + OfflineIntervalString+ "+" + expireTime + "<" +
			strconv.FormatInt(time.Now().Unix(), 10)
	} else {
		offlineWhere = "this.last_heartbeat_time+this.heartbeat_interval+" + OfflineIntervalString + "< " +
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

func UpdateRaspDescription(raspId string, description string) error {
	return mongo.UpdateId(raspCollectionName, raspId, bson.M{"description": description})
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

func CleanOfflineHosts() {
	for appId, interval := range HasOfflineHosts {
		selector := map[string]interface{}{
			"expire_time": interval * 24 * 3600,
		}
		removedCount, err := RemoveRaspBySelector(selector, appId)
		if err != nil {
			beego.Error("clear offline err:", err)
		}
		beego.Info("remove rasps for app:", appId, " remove counts:", removedCount)
	}
}
