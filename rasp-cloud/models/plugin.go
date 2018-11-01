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
	"fmt"
	"crypto/md5"
	"rasp-cloud/mongo"
	"github.com/astaxie/beego"
	"gopkg.in/mgo.v2/bson"
	"sync"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2"
	"time"
)

type Plugin struct {
	Id           string `json:"id,omitempty" bson:"_id"`
	AppId        string `json:"app_id" bson:"app_id"`
	UploadTime   int64  `json:"upload_time" bson:"upload_time"`
	Version      string `json:"version" bson:"version"`
	Md5          string `json:"md5" bson:"md5"`
	Content      string `json:"plugin,omitempty" bson:"content"`
	ConfigFormat string `json:"config_format" bson:"config_format"`
}

const (
	pluginCollectionName = "plugin"
)

var (
	mutex      sync.Mutex
	MaxPlugins int
)

func init() {
	createIndex()
	if value, err := beego.AppConfig.Int("MaxPlugins"); err != nil || value <= 0 {
		MaxPlugins = 10
	} else {
		MaxPlugins = value
	}
}

// create mongo index for plugin collection
func createIndex() {
	count, err := mongo.Count(pluginCollectionName)
	if err != nil {
		tools.Panic("failed to get rasp collection count")
	}
	if count <= 0 {
		index := &mgo.Index{
			Key:        []string{"app_id", "md5"},
			Unique:     true,
			Background: true,
			Name:       "plugin_app_md5",
		}
		err = mongo.CreateIndex(pluginCollectionName, index)
		if err != nil {
			tools.Panic("failed to create index for plugin collection, index name: plugin_app_md5")
		}
		index = &mgo.Index{
			Key:        []string{"upload_time"},
			Background: true,
			Name:       "plugin_upload_time",
		}
		err = mongo.CreateIndex(pluginCollectionName, index)
		if err != nil {
			tools.Panic("failed to create index for plugin collection, index name: plugin_upload_time")
		}
	}
}

func AddPlugin(version string, content []byte, appId string, configFormat string) (plugin *Plugin, err error) {
	newMd5 := fmt.Sprintf("%x", md5.Sum(content))
	plugin = &Plugin{
		Version:      version,
		Md5:          newMd5, Content: string(content),
		UploadTime:   time.Now().UnixNano() / 1000000,
		ConfigFormat: configFormat,
		AppId:        appId,
		Id:           appId + newMd5,
	}
	mutex.Lock()
	defer mutex.Unlock()

	var count int
	_, oldPlugins, err := GetPluginsByApp(appId, MaxPlugins-1, 0)
	if err != nil {
		return
	}
	count = len(oldPlugins)
	if count > 0 {
		for _, oldPlugin := range oldPlugins {
			err = mongo.RemoveId(pluginCollectionName, oldPlugin.Id)
			if err != nil {
				return
			}
		}
	}
	err = mongo.Insert(pluginCollectionName, plugin)
	return
}

func GetSelectedPlugin(appId string) (plugin *Plugin, err error) {
	var app *App
	if err = mongo.FindId(appCollectionName, appId, &app); err != nil {
		return
	}
	return GetPluginById(app.SelectedPluginId)
}

func SetSelectedPlugin(appId string, pluginId string) error {
	var app *App
	if err := mongo.FindId(appCollectionName, appId, &app); err != nil {
		return err
	}
	_, err := GetPluginById(pluginId)
	if err != nil {
		return err
	}
	app.SelectedPluginId = pluginId
	return mongo.UpdateId(appCollectionName, appId, app)
}

func GetPluginById(id string) (plugin *Plugin, err error) {
	err = mongo.FindId(pluginCollectionName, id, &plugin)
	return
}

func GetPluginsByApp(appId string, skip int, limit int) (total int, plugins []Plugin, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	total, err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId}).Count()
	if err != nil {
		return
	}
	err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId}).
		Sort("-upload_time").Skip(skip).Limit(limit).All(&plugins)
	return
}

func GetPluginByMd5(appId string, md5 string) (plugin *Plugin, count int, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId, "md5": md5}).One(&plugin)
	return
}

func DeletePlugin(pluginId string) error {
	mutex.Lock()
	defer mutex.Unlock()
	return mongo.RemoveId(pluginCollectionName, pluginId)
}

func NewPlugin(version string, content []byte, appId string) *Plugin {
	newMd5 := fmt.Sprintf("%x", md5.Sum(content))
	return &Plugin{Version: version, Md5: newMd5, Content: string(content)}
}
