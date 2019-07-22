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
	"fmt"
	"crypto/md5"
	"rasp-cloud/mongo"
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
	"bufio"
	"bytes"
	"github.com/robertkrimen/otto"
	"rasp-cloud/conf"
	"strings"
)

type Plugin struct {
	Id                     string                 `json:"id" bson:"_id,omitempty"`
	AppId                  string                 `json:"app_id" bson:"app_id"`
	Name                   string                 `json:"name" bson:"name"`
	UploadTime             int64                  `json:"upload_time" bson:"upload_time"`
	Version                string                 `json:"version" bson:"version"`
	Description            string                 `json:"description" bson:"description"`
	Md5                    string                 `json:"md5" bson:"md5"`
	OriginContent          string                 `json:"origin_content,omitempty" bson:"origin_content"`
	Content                string                 `json:"plugin,omitempty" bson:"content"`
	DefaultAlgorithmConfig map[string]interface{} `json:"-" bson:"default_algorithm_config"`
	AlgorithmConfig        map[string]interface{} `json:"algorithm_config" bson:"algorithm_config"`
}

const (
	pluginCollectionName = "plugin"
)

var (
	mutex sync.Mutex
)

func init() {
	index := &mgo.Index{
		Key:        []string{"app_id"},
		Unique:     false,
		Background: true,
		Name:       "app_id",
	}
	err := mongo.CreateIndex(pluginCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed,
			"failed to create app_id index for plugin collection", err)
	}
	index = &mgo.Index{
		Key:        []string{"upload_time"},
		Unique:     false,
		Background: true,
		Name:       "upload_time",
	}
	err = mongo.CreateIndex(pluginCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed,
			"failed to create the upload_time index for plugin collection", err)
	}
}

func AddPlugin(pluginContent []byte, appId string) (plugin *Plugin, err error) {
	plugin, err = generatePlugin(pluginContent, appId)
	if err != nil {
		return
	}
	err = addPluginToDb(plugin)
	if err != nil {
		return nil, err
	}
	return
}

func generatePlugin(pluginContent []byte, appId string) (plugin *Plugin, err error) {
	pluginReader := bufio.NewReader(bytes.NewReader(pluginContent))
	var newVersion string
	var newPluginName string
	var newPluginDesc string
	for i := 0; i < 3; i++ {
		var lineValue string
		var lineSep []string
		line, err := pluginReader.ReadString('\n')
		if err != nil {
			return nil, errors.New("failed to read the plugin file: " + err.Error())
		}
		if lineValue = regexp.MustCompile(`'.+'|".+"`).FindString(line); lineValue == "" {
			continue
		}
		if lineSep = strings.Split(line, "="); len(lineSep) >= 2 {
			if strings.Contains(lineSep[0], "plugin_version") {
				newVersion = lineValue[1 : len(lineValue)-1]
			} else if strings.Contains(lineSep[0], "plugin_name") {
				newPluginName = lineValue[1 : len(lineValue)-1]
			} else if strings.Contains(lineSep[0], "plugin_desc") {
				newPluginDesc = lineValue[1 : len(lineValue)-1]
			}
		}
	}
	if newVersion == "" || newPluginName == "" {
		return nil, errors.New("the plugin name and plugin version can not be empty")
	}
	algorithmStartMsg := "// BEGIN ALGORITHM CONFIG //"
	algorithmEndMsg := "// END ALGORITHM CONFIG //"
	algorithmStart := bytes.Index(pluginContent, []byte(algorithmStartMsg))
	if algorithmStart < 0 {
		return nil, errors.New("failed to find the start of algorithmConfig variable: " + algorithmStartMsg)
	}
	algorithmStart = algorithmStart + len([]byte(algorithmStartMsg))
	algorithmEnd := bytes.Index(pluginContent, []byte(algorithmEndMsg))
	if algorithmEnd < 0 {
		return nil, errors.New("failed to find the end of algorithmConfig variable: " + algorithmEndMsg)
	}
	jsVm := otto.New()
	_, err = jsVm.Run(string(pluginContent[algorithmStart:algorithmEnd]) +
		"\n algorithmContent=JSON.stringify(algorithmConfig)")
	if err != nil {
		return nil, errors.New("failed to get algorithm config from plugin: " + err.Error())
	}
	algorithmContent, err := jsVm.Get("algorithmContent")
	if err != nil {
		return nil, errors.New("failed to get algorithm config from plugin: " + err.Error())
	}
	var algorithmData map[string]interface{}
	err = json.Unmarshal([]byte(algorithmContent.String()), &algorithmData)
	if err != nil {
		return nil, errors.New("failed to unmarshal algorithm json data: " + err.Error())
	}
	newMd5 := fmt.Sprintf("%x", md5.Sum(pluginContent))
	plugin = &Plugin{
		Id:                     generatePluginId(appId),
		Version:                newVersion,
		Name:                   newPluginName,
		Description:            newPluginDesc,
		Md5:                    newMd5,
		OriginContent:          string(pluginContent),
		Content:                string(pluginContent),
		UploadTime:             time.Now().UnixNano() / 1000000,
		AppId:                  appId,
		DefaultAlgorithmConfig: algorithmData,
		AlgorithmConfig:        algorithmData,
	}
	return plugin, nil
}

func addPluginToDb(plugin *Plugin) (err error) {
	mutex.Lock()
	defer mutex.Unlock()

	var count int
	if conf.AppConfig.MaxPlugins > 0 {
		_, oldPlugins, err := GetPluginsByApp(plugin.AppId, conf.AppConfig.MaxPlugins-1, 0, "-upload_time")
		if err != nil {
			return err
		}
		count = len(oldPlugins)
		if count > 0 {
			for _, oldPlugin := range oldPlugins {
				app := &App{}
				err = mongo.FindOne(appCollectionName, bson.M{"selected_plugin_id": oldPlugin.Id}, app)
				if err != nil && err != mgo.ErrNotFound {
					return err
				}
				if app.Id != "" {
					continue
				}
				err = mongo.RemoveId(pluginCollectionName, oldPlugin.Id)
				if err != nil {
					return err
				}
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
		return nil, errors.New("can not get app," + err.Error())
	}
	return GetPluginById(app.SelectedPluginId, hasContent)
}

func SetSelectedPlugin(appId string, pluginId string) (plugin *Plugin, err error) {
	plugin, err = GetPluginById(pluginId, false)
	if err != nil {
		return
	}
	return plugin, mongo.UpdateId(appCollectionName, appId, bson.M{"selected_plugin_id": pluginId})
}

func RestoreDefaultConfiguration(pluginId string) (appId string, err error) {
	plugin, err := GetPluginById(pluginId, true)
	if err != nil {
		return "", err
	}
	return handleAlgorithmConfig(plugin, plugin.DefaultAlgorithmConfig)
}

func UpdateAlgorithmConfig(pluginId string, config map[string]interface{}) (appId string, err error) {
	plugin, err := GetPluginById(pluginId, true)
	if err != nil {
		return "", err
	}
	if err := validAlgorithmConfig(plugin, config); err != nil {
		return "", err
	}
	return handleAlgorithmConfig(plugin, config)
}

func validAlgorithmConfig(plugin *Plugin, config map[string]interface{}) error {
	errMsg := "failed to match the new config format to default algorithm config format"
	for key, defaultValue := range plugin.DefaultAlgorithmConfig {
		if c, ok := config[key]; !ok || (c == nil && defaultValue != nil) {
			return errors.New(errMsg + ", " + "can not find the key '" + key + "' in new config")
		}
		if defaultValue != nil {
			if defaultItem, ok := defaultValue.(map[string]interface{}); ok {
				if item, ok := config[key].(map[string]interface{}); ok {
					for subKey := range defaultItem {
						if _, ok := item[subKey]; !ok {
							return errors.New(errMsg + ", " + "can not find the key '" +
								key + "." + subKey + "' in new config")
						}
					}
				} else {
					return errors.New(errMsg + ", " + "the key '" + key + "' must be an object")
				}
			}
		}
	}
	return nil
}

func handleAlgorithmConfig(plugin *Plugin, config map[string]interface{}) (appId string, err error) {
	content, err := json.MarshalIndent(config, "", "\t")
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

func GetPluginsByApp(appId string, skip int, limit int, sortField string) (total int, plugins []Plugin, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	total, err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId}).Count()
	if err != nil {
		return
	}
	err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(bson.M{"app_id": appId}).
		Select(bson.M{"content": 0, "origin_content": 0}).
		Sort(sortField).Skip(skip).Limit(limit).All(&plugins)
	if plugins == nil {
		plugins = make([]Plugin, 0)
	}
	return
}

func SearchPlugins(selector bson.M, skip int, limit int, sortField string) (plugins []Plugin, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	err = newSession.DB(mongo.DbName).C(pluginCollectionName).Find(selector).Select(bson.M{"content": 0, "origin_content": 0}).
		Sort(sortField).Skip(skip).Limit(limit).All(&plugins)
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
	_, err := mongo.RemoveAll(pluginCollectionName, bson.M{"app_id": appId})
	return err
}
