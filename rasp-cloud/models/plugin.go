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
	"time"
	"regexp"
	"encoding/json"
	"errors"
	"strconv"
	"math/rand"
	"crypto/sha1"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2"
)

type Plugin struct {
	Id              string                 `json:"id" bson:"_id,omitempty"`
	AppId           string                 `json:"app_id" bson:"app_id"`
	UploadTime      int64                  `json:"upload_time" bson:"upload_time"`
	Version         string                 `json:"version" bson:"version"`
	Md5             string                 `json:"md5" bson:"md5"`
	Content         string                 `json:"plugin,omitempty" bson:"content"`
	AlgorithmConfig map[string]interface{} `json:"algorithm_config" bson:"algorithm_config"`
}

const (
	pluginCollectionName = "plugin"
)

var (
	mutex      sync.Mutex
	MaxPlugins int
)

func init() {
	if value, err := beego.AppConfig.Int("MaxPlugins"); err != nil || value <= 0 {
		MaxPlugins = 10
	} else {
		MaxPlugins = value
	}
	count, err := mongo.Count(pluginCollectionName)
	if err != nil {
		tools.Panic("failed to get plugin collection count", err)
	}
	if count <= 0 {
		index := &mgo.Index{
			Key:        []string{"app_id"},
			Unique:     false,
			Background: true,
			Name:       "app_id",
		}
		err := mongo.CreateIndex(pluginCollectionName, index)
		if err != nil {
			tools.Panic("failed to create app_id index for plugin collection", err)
		}
		index = &mgo.Index{
			Key:        []string{"upload_time"},
			Unique:     false,
			Background: true,
			Name:       "upload_time",
		}
		err = mongo.CreateIndex(pluginCollectionName, index)
		if err != nil {
			tools.Panic("failed to create upload_time index for plugin collection", err)
		}
	}
}

func AddPlugin(version string, content []byte, appId string,
	defaultAlgorithmConfig map[string]interface{}) (plugin *Plugin, err error) {
	newMd5 := fmt.Sprintf("%x", md5.Sum(content))
	plugin = &Plugin{
		Id:              generatePluginId(appId),
		Version:         version,
		Md5:             newMd5,
		Content:         string(content),
		UploadTime:      time.Now().UnixNano() / 1000000,
		AppId:           appId,
		AlgorithmConfig: defaultAlgorithmConfig,
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

func generatePluginId(appId string) string {
	random := string(bson.NewObjectId()) + appId +
		strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(10000))
	return fmt.Sprintf("%x", sha1.Sum([]byte(random)))
}

func GetSelectedPlugin(appId string, hasContent bool) (plugin *Plugin, err error) {
	var app *App
	if err = mongo.FindId(appCollectionName, appId, &app); err != nil {
		return
	}
	return GetPluginById(app.SelectedPluginId, hasContent)
}

func SetSelectedPlugin(appId string, pluginId string) error {
	_, err := GetPluginById(pluginId, false)
	if err != nil {
		return err
	}
	return mongo.UpdateId(appCollectionName, appId, bson.M{"selected_plugin_id": pluginId})
}

func UpdateAlgorithmConfig(pluginId string, config map[string]interface{}) (appId string, err error) {
	plugin, err := GetPluginById(pluginId, true)
	if err != nil {
		return "", err
	}
	content, err := json.Marshal(config)
	if err != nil {
		return "", err
	}
	regex := `//\s*BEGIN\s*ALGORITHM\s*CONFIG\s*//[\W\w]*?//\s*END\s*ALGORITHM\s*CONFIG\s*//`
	newContent := "// BEGIN ALGORITHM CONFIG //\n\n" +
		"var algorithmConfig = " +
		string(content) + "\n\n// END ALGORITHM CONFIG //"
	if variable := regexp.MustCompile(regex).
		FindString(plugin.Content); len(variable) <= 0 {
		return "", errors.New("failed to find algorithmConfig variable")
	}
	algorithmContent := regexp.MustCompile(regex).ReplaceAllString(plugin.Content, newContent)
	newMd5 := fmt.Sprintf("%x", md5.Sum([]byte(algorithmContent)))
	return plugin.AppId, mongo.UpdateId(pluginCollectionName, plugin.Id, bson.M{"content": algorithmContent,
		"algorithm_config": config, "md5": newMd5})
}

func GetPluginById(id string, hasContent bool) (plugin *Plugin, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	query := newSession.DB(mongo.DbName).C(pluginCollectionName).FindId(id)
	if hasContent {
		err = query.One(&plugin)
	} else {
		err = query.Select(bson.M{"content": 0}).One(&plugin)
	}
	return
}

func GetPluginsByApp(appId string, skip int, limit int) (total int, plugins []Plugin, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	total, err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId}).Count()
	if err != nil {
		return
	}
	err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId}).Select(bson.M{"content": 0}).
		Sort("-upload_time").Skip(skip).Limit(limit).All(&plugins)
	if plugins == nil {
		plugins = make([]Plugin, 0)
	}
	return
}

func DeletePlugin(pluginId string) error {
	mutex.Lock()
	defer mutex.Unlock()
	return mongo.RemoveId(pluginCollectionName, pluginId)
}

func RemovePluginByAppId(appId string) error {
	return mongo.RemoveAll(pluginCollectionName, bson.M{"app_id": appId})
}

func NewPlugin(version string, content []byte, appId string) *Plugin {
	newMd5 := fmt.Sprintf("%x", md5.Sum(content))
	return &Plugin{Version: version, Md5: newMd5, Content: string(content)}
}
