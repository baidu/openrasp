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

package api

import (
	"rasp-cloud/models"
	"path"
	"net/http"
	"rasp-cloud/controllers"
	"io/ioutil"
	"archive/zip"
	"encoding/json"
	"bufio"
	"regexp"
	"bytes"
)

// Operations about plugin
type PluginController struct {
	controllers.BaseController
}

// @router /upload [post]
func (o *PluginController) Upload() {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	uploadFile, info, err := o.GetFile("plugin")
	if uploadFile == nil {
		o.ServeError(http.StatusBadRequest, "must have the plugin parameter")
	}
	defer uploadFile.Close()
	if err != nil {
		o.ServeError(http.StatusBadRequest, "parse uploadFile error: "+err.Error())
	}
	if info.Size == 0 {
		o.ServeError(http.StatusBadRequest, "upload uploadFile cannot be empty")
	}
	fileName := info.Filename
	if len(fileName) <= 0 || len(fileName) > 50 {
		o.ServeError(http.StatusBadRequest, "the length of upload uploadFile name must be (0,50]")
	}
	if path.Ext(fileName) != ".zip" {
		o.ServeError(http.StatusBadRequest, "the uploadFile name suffix must be .zip")
	}
	reader, err := zip.NewReader(uploadFile, info.Size)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to read zip uploadFile: "+err.Error())
	}
	var configFormat string
	var pluginContent []byte
	var newVersion string
	for _, file := range reader.File {
		if file.Name == "plugin/plugin.js" {
			fileReader, err := file.Open()
			if err != nil {
				o.ServeError(http.StatusBadRequest, "failed to read plugin.js in the zip file: "+err.Error())
			}
			content, err := ioutil.ReadAll(fileReader)
			if err != nil {
				o.ServeError(http.StatusBadRequest, "failed to read plugin.js in the zip file: "+err.Error())
			}
			// parse plugin version
			firstLine, err := bufio.NewReader(bytes.NewReader(content)).ReadString('\n')
			if err != nil {
				o.ServeError(http.StatusBadRequest, "failed to read the plugin.js in the zip file: "+err.Error())
			}
			if versionArr := regexp.MustCompile(`'.+'|".+"`).FindAllString(firstLine, -1); len(versionArr) > 0 {
				newVersion = versionArr[0][1 : len(versionArr[0])-1]
			} else {
				o.ServeError(http.StatusBadRequest, "failed to find the plugin version: "+err.Error())
			}
			pluginContent = content
			fileReader.Close()
		} else if file.Name == "plugin/config.json" {
			fileReader, err := file.Open()
			if err != nil {
				o.ServeError(http.StatusBadRequest, "failed to read config.json in the zip file: "+err.Error())
			}
			content, err := ioutil.ReadAll(fileReader)
			if err != nil {
				o.ServeError(http.StatusBadRequest, "failed to read config.json in the zip file: "+err.Error())
			}
			if !json.Valid(content) {
				o.ServeError(http.StatusBadRequest, "the format of config.json is error")
			}
			configFormat = string(content)
			fileReader.Close()
		}
	}
	if configFormat == "" {
		o.ServeError(http.StatusBadRequest, "plugin config format can not be empty")
	}
	if len(pluginContent) == 0 {
		o.ServeError(http.StatusBadRequest, "plugin content can not be empty")
	}
	if len(newVersion) == 0 {
		o.ServeError(http.StatusBadRequest, "failed to get plugin version")
	}
	latestPlugin, err := models.AddPlugin(newVersion, pluginContent, appId, configFormat)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to add plugin to mongodb: "+err.Error())
	}
	o.Serve(latestPlugin)
}

// @router / [get]
func (o *PluginController) Get() {
	pluginId := o.GetString("id")
	if pluginId == "" {
		o.ServeError(http.StatusBadRequest, "plugin id cannot be empty")
	}
	plugin, err := models.GetPluginById(pluginId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get plugin: "+err.Error())
	}
	o.Serve(plugin)
}

// @router /delete [get]
func (o *PluginController) Delete() {
	pluginId := o.GetString("id")
	if pluginId == "" {
		o.ServeError(http.StatusBadRequest, "plugin_id cannot be empty")
	}
	err := models.DeletePlugin(pluginId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to delete plugin: "+err.Error())
	}
	o.ServeWithoutData()
}
