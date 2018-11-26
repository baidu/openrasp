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
	"encoding/json"
	"github.com/astaxie/beego/validation"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"math"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"strconv"
	"sync"
	"time"
)

// Operations about app
type AppController struct {
	controllers.BaseController
}

type pageParam struct {
	AppId   string `json:"app_id"`
	Page    int    `json:"page"`
	Perpage int    `json:"perpage"`
}

var (
	supportLanguages = []string{"java", "php"}
	mutex            sync.Mutex
)

// @router /get [post]
func (o *AppController) GetApp() {
	var data pageParam
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &data)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if data.AppId == "" {
		if data.Page <= 0 {
			o.ServeError(http.StatusBadRequest, "page must be greater than 0")
		}
		if data.Perpage <= 0 {
			o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
		}
		var result = make(map[string]interface{})
		total, apps, err := models.GetAllApp(data.Page, data.Perpage)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get apps", err)
		}
		if apps == nil {
			apps = make([]models.App, 0)
		}
		result["total"] = total
		result["total_page"] = math.Ceil(float64(total) / float64(data.Perpage))
		result["page"] = data.Page
		result["perpage"] = data.Perpage
		result["data"] = apps
		o.Serve(result)
	} else {
		app, err := models.GetAppById(data.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get app", err)
		}
		o.Serve(app)
	}
}

// @router /rasp/get [post]
func (o *AppController) GetRasps() {
	var param pageParam
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.Page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	if param.Perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}

	app, err := models.GetAppById(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	if app == nil {
		o.ServeError(http.StatusBadRequest, "the app doesn't exist")
	}
	var result = make(map[string]interface{})
	total, rasps, err := models.GetRaspByAppId(app.Id, param.Page, param.Perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get apps", err)
	}
	result["total"] = total
	result["total_page"] = math.Ceil(float64(total) / float64(param.Perpage))
	result["page"] = param.Page
	result["perpage"] = param.Perpage
	result["data"] = rasps
	o.Serve(result)
}

// @router /secret/get [post]
func (o *AppController) GetAppSecret() {
	var param struct {
		AppId string `json:"app_id"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	secret, err := models.GetSecretByAppId(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get secret", err)
	}
	o.Serve(map[string]string{
		"secret": secret,
	})
}

// @router /secret/regenerate [post]
func (o *AppController) RegenerateAppSecret() {
	var param struct {
		AppId string `json:"app_id"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	secret, err := models.RegenerateSecret(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get secret", err)
	}
	models.AddOperation(param.AppId, models.OperationTypeRegenerateSecret,
		o.Ctx.Input.IP(), "regenerated the secret")
	o.Serve(map[string]string{
		"secret": secret,
	})
}

// @router /general/config [post]
func (o *AppController) UpdateAppGeneralConfig() {
	var param struct {
		AppId  string                 `json:"app_id"`
		Config map[string]interface{} `json:"config"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	if param.Config == nil {
		o.ServeError(http.StatusBadRequest, "config can not be empty")
	}
	o.validateAppConfig(param.Config)
	app, err := models.UpdateGeneralConfig(param.AppId, param.Config)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app general config", err)
	}
	models.AddOperation(param.AppId, models.OperationTypeUpdateGenerateConfig,
		o.Ctx.Input.IP(), "updated the general configuration")
	o.Serve(app)
}

// @router /whitelist/config [post]
func (o *AppController) UpdateAppWhiteListConfig() {
	var param struct {
		AppId  string                       `json:"app_id"`
		Config []models.WhitelistConfigItem `json:"config"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	if param.Config == nil {
		o.ServeError(http.StatusBadRequest, "config can not be empty")
	}
	o.validateWhiteListConfig(param.Config)
	app, err := models.UpdateWhiteListConfig(param.AppId, param.Config)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app whitelist config", err)
	}
	models.AddOperation(param.AppId, models.OperationTypeUpdateWhitelistConfig,
		o.Ctx.Input.IP(), "updated the whitelist configuration")
	o.Serve(app)
}

// @router /algorithm/config [post]
func (o *AppController) UpdateAppAlgorithmConfig() {
	var param struct {
		PluginId string                 `json:"plugin_id"`
		Config   map[string]interface{} `json:"config"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.PluginId == "" {
		o.ServeError(http.StatusBadRequest, "plugin_id can not be empty")
	}
	if param.Config == nil {
		o.ServeError(http.StatusBadRequest, "config can not be empty")
	}
	o.validateAppConfig(param.Config)
	appId, err := models.UpdateAlgorithmConfig(param.PluginId, param.Config)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update algorithm config", err)
	}
	models.AddOperation(appId, models.OperationTypeUpdateAlgorithmConfig,
		o.Ctx.Input.IP(), "updated the whitelist configuration")
	o.ServeWithEmptyData()
}

// @router / [post]
func (o *AppController) Post() {
	var app = &models.App{}

	err := json.Unmarshal(o.Ctx.Input.RequestBody, app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if app.Name == "" {
		o.ServeError(http.StatusBadRequest, "app name cannot be empty")
	}
	if len(app.Name) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app name cannot be greater than 64")
	}
	if app.Language == "" {
		o.ServeError(http.StatusBadRequest, "app language cannot be empty")
	}
	if len(app.Language) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app language name cannot be greater than 64")
	}
	languageSupported := false
	for _, language := range supportLanguages {
		if app.Language == language {
			languageSupported = true
			break
		}
	}
	if !languageSupported {
		o.ServeError(http.StatusBadRequest, "can not support the language: "+app.Language)
	}
	if len(app.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of app description can not be greater than 1024")
	}
	if len(app.SelectedPluginId) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of app selected_plugin_id can not be greater than 1024")
	}
	if app.EmailAlarmConf.Enable {
		o.validEmailConf(&app.EmailAlarmConf)
	}
	if app.HttpAlarmConf.Enable {
		o.validHttpAlarm(&app.HttpAlarmConf)
	}
	if app.DingAlarmConf.Enable {
		o.validDingConf(&app.DingAlarmConf)
	}
	if app.GeneralConfig != nil {
		o.validateAppConfig(app.GeneralConfig)
		configTime := time.Now().UnixNano()
		app.ConfigTime = configTime
	} else {
		app.GeneralConfig = models.DefaultGeneralConfig
	}

	if app.WhitelistConfig != nil {
		o.validateWhiteListConfig(app.WhitelistConfig)
		configTime := time.Now().UnixNano()
		app.ConfigTime = configTime
	} else {
		app.WhitelistConfig = make([]models.WhitelistConfigItem, 0)
	}
	models.HandleApp(app, true)
	app, err = models.AddApp(app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "create app failed", err)
	}
	models.AddOperation(app.Id, models.OperationTypeAddApp, o.Ctx.Input.IP(), "created the app: "+app.Name)
	o.Serve(app)
}

// @router /config [post]
func (o *AppController) ConfigApp() {
	var param struct {
		AppId       string `json:"app_id"`
		Language    string `json:"language,omitempty"`
		Name        string `json:"name,omitempty"`
		Description string `json:"description,omitempty"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	_, err = models.GetAppById(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	if param.Name == "" {
		o.ServeError(http.StatusBadRequest, "app name cannot be empty")
	}
	if len(param.Name) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app name cannot be greater than 64")
	}
	if param.Language == "" {
		o.ServeError(http.StatusBadRequest, "app language cannot be empty")
	}
	if len(param.Language) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app language name cannot be greater than 64")
	}
	languageSupported := false
	for _, language := range supportLanguages {
		if param.Language == language {
			languageSupported = true
			break
		}
	}
	if !languageSupported {
		o.ServeError(http.StatusBadRequest, "can not support the language: "+param.Language)
	}
	if len(param.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of app description can not be greater than 1024")
	}
	updateData := bson.M{"name": param.Name, "language": param.Language, "description": param.Description}
	app, err := models.UpdateAppById(param.AppId, updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app config", err)
	}
	operationData, err := json.Marshal(updateData)
	models.AddOperation(app.Id, models.OperationTypeEditApp, o.Ctx.Input.IP(), "edited the app: "+string(operationData))
	o.Serve(app)
}

func (o *AppController) validEmailConf(conf *models.EmailAlarmConf) {
	var valid = validation.Validation{}
	if conf.ServerAddr == "" {
		o.ServeError(http.StatusBadRequest, "the email server_addr cannot be empty")
	}
	if len(conf.ServerAddr) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email server_addr cannot be greater than 128")
	}
	if len(conf.Subject) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email subject cannot be greater than 256")
	}
	if conf.UserName == "" {
		o.ServeError(http.StatusBadRequest, "the email from_addr cannot be empty")
	}
	if len(conf.UserName) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email from_addr cannot be greater than 256")
	}
	if conf.UserName == "" {
		o.ServeError(http.StatusBadRequest, "the email from_addr cannot be empty")
	}
	if result := valid.Email(conf.UserName, "email"); !result.Ok {
		o.ServeError(http.StatusBadRequest, "the email from_addr format error: "+result.Error.Message)
	}
	if conf.Password == "" {
		o.ServeError(http.StatusBadRequest, "the email password cannot be empty")
	}
	if len(conf.Password) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email password cannot be greater than 256")
	}
	if len(conf.RecvAddr) == 0 {
		o.ServeError(http.StatusBadRequest, "the email recv_addr cannot be empty")
	}
	if len(conf.RecvAddr) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of email recv_addr cannot be greater than 128")
	}
	conf.RecvAddr = o.validAppArrayParam(conf.RecvAddr, "email recv_addr", valid.Email)
}

func (o *AppController) validDingConf(conf *models.DingAlarmConf) {
	if conf.CorpId == "" {
		o.ServeError(http.StatusBadRequest, "the ding ding corp_id cannot be empty")
	}
	if len(conf.CorpId) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of ding ding corp_id cannot be greater than 128")
	}
	if conf.CorpSecret == "" {
		o.ServeError(http.StatusBadRequest, "the ding ding corp_secret cannot be empty")
	}
	if len(conf.CorpSecret) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of ding ding corp_secret cannot be greater than 128")
	}
	if len(conf.RecvParty) == 0 && len(conf.RecvUser) == 0 {
		o.ServeError(http.StatusBadRequest, "ding ding recv_party and ding ding recv_user cannot be empty at the same time")
	}
	if len(conf.RecvParty) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of ding ding recv_party cannot be greater than 128")
	}
	if len(conf.RecvUser) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of ding ding recv_user cannot be greater than 128")
	}
	if conf.AgentId == "" {
		o.ServeError(http.StatusBadRequest, "the ding ding agent_id cannot be empty")
	}
	if len(conf.AgentId) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of ding agent_id cannot be greater than 128")
	}
	conf.RecvUser = o.validAppArrayParam(conf.RecvUser, "ding recv_user", nil)
	conf.RecvParty = o.validAppArrayParam(conf.RecvParty, "ding recv_party", nil)
}

func (o *AppController) validHttpAlarm(conf *models.HttpAlarmConf) {
	if len(conf.RecvAddr) == 0 {
		o.ServeError(http.StatusBadRequest, "the http recv_addr cannot be empty")
	}
	if len(conf.RecvAddr) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of http recv_addr cannot be greater than 128")
	}
	conf.RecvAddr = o.validAppArrayParam(conf.RecvAddr, "http recv_addr", nil)
}

// @router /delete [post]
func (o *AppController) Delete() {
	var app = &models.App{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if app.Id == "" {
		o.ServeError(http.StatusBadRequest, "the id cannot be empty")
	}
	mutex.Lock()
	defer mutex.Unlock()
	count, err := models.GetAppCount()
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app count", err)
	}
	if count <= 1 {
		o.ServeError(http.StatusBadRequest, "failed to remove app: keep at least one app")
	}
	err = models.RemoveAppById(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove app", err)
	}
	err = models.RemoveRaspByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove rasp by app_id", err)
	}
	err = models.RemovePluginByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove plugin by app_id", err)
	}
	err = models.RemovePluginByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove plugin by app_id", err)
	}
	err = models.RemoveOperationByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove operation log by app_id", err)
	}
	models.AddOperation(app.Id, models.OperationTypeDeleteApp, o.Ctx.Input.IP(), "deleted the app")
	o.ServeWithEmptyData()
}

func (o *AppController) validAppArrayParam(param []string, paramName string,
	valid func(interface{}, string) *validation.Result) []string {
	if param != nil {
		if len(param) > 128 {
			o.ServeError(http.StatusBadRequest,
				"the count of "+paramName+" cannot be greater than 128")
		}
		for i, v := range param {
			if len(v) > 256 {
				o.ServeError(http.StatusBadRequest,
					"the element's length of "+paramName+" cannot be greater than 256")
			}
			if valid != nil {
				if result := valid(v, "valid"); !result.Ok {
					o.ServeError(http.StatusBadRequest,
						"the "+strconv.Itoa(i)+"th element's format of "+paramName+" is error: "+result.Error.Message)
				}
			}
		}
	} else {
		param = make([]string, 0)
	}
	return param
}

func (o *AppController) validateAppConfig(config map[string]interface{}) {
	if config == nil {
		o.ServeError(http.StatusBadRequest, "the config cannot be nil")
	}
	for key, value := range config {
		if value == nil {
			o.ServeError(http.StatusBadRequest, "the value of "+key+" config cannot be nil")
		}
		if v, ok := value.(string); ok {
			if len(v) >= 512 {
				o.ServeError(http.StatusBadRequest,
					"the length of config key "+key+" must less tha 1024")
			}
		}
	}
}

func (o *AppController) validateWhiteListConfig(config []models.WhitelistConfigItem) {
	if config == nil {
		o.ServeError(http.StatusBadRequest, "the config cannot be nil")
	}
	if len(config) > 200 {
		o.ServeError(http.StatusBadRequest,
			"the count of whitelist config items must be between (0,200]")
	}
	for _, value := range config {
		if len(value.Url) > 200 || len(value.Url) == 0 {
			o.ServeError(http.StatusBadRequest,
				"the length of whitelist config url must be between [1,200]")
		}
		for key := range value.Hook {
			if len(key) > 128 {
				o.ServeError(http.StatusBadRequest,
					"the length of hook's type can not be greater 128")
			}
		}
	}
}

// @router /alarm/config [post]
func (o *AppController) ConfigAlarm() {
	var param struct {
		AppId          string                 `json:"app_id"`
		EmailAlarmConf *models.EmailAlarmConf `json:"email_alarm_conf,omitempty"`
		DingAlarmConf  *models.DingAlarmConf  `json:"ding_alarm_conf,omitempty"`
		HttpAlarmConf  *models.HttpAlarmConf  `json:"http_alarm_conf,omitempty"`
	}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	_, err = models.GetAppById(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	var updateData bson.M
	if param.EmailAlarmConf != nil {
		o.validEmailConf(param.EmailAlarmConf)
	}
	if param.HttpAlarmConf != nil {
		o.validHttpAlarm(param.HttpAlarmConf)
	}
	if param.DingAlarmConf != nil {
		o.validDingConf(param.DingAlarmConf)
	}
	content, err := json.Marshal(param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to encode param to json", err)
	}
	err = json.Unmarshal(content, &updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to decode param json", err)
	}
	app, err := models.UpdateAppById(param.AppId, updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update alarm config", err)
	}
	models.AddOperation(app.Id, models.OperationTypeUpdateAlarmConfig, o.Ctx.Input.IP(),
		"updated the alarm configuration")
	o.Serve(app)
}

// @router /plugin/get [post]
func (o *AppController) GetPlugins() {
	var param pageParam
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	if param.Page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	if param.Perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}

	app, err := models.GetAppById(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	if app == nil {
		o.ServeError(http.StatusBadRequest, "the app doesn't exist")
	}
	var result = make(map[string]interface{})
	total, plugins, err := models.GetPluginsByApp(param.AppId, (param.Page-1)*param.Perpage, param.Perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get plugins", err)
	}
	result["total"] = total
	result["total_page"] = math.Ceil(float64(total) / float64(param.Perpage))
	result["page"] = param.Page
	result["perpage"] = param.Perpage
	result["data"] = plugins
	o.Serve(result)
}

// @router /plugin/select/get [post]
func (o *AppController) GetSelectedPlugin() {
	var param map[string]string
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	plugin, err := models.GetSelectedPlugin(appId, false)
	if mgo.ErrNotFound == err || plugin == nil {
		o.ServeWithEmptyData()
		return
	}
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get selected plugin", err)
	}
	o.Serve(plugin)
}

// @router /plugin/select [post]
func (o *AppController) SetSelectedPlugin() {
	var param map[string]string
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	pluginId := param["plugin_id"]
	if pluginId == "" {
		o.ServeError(http.StatusBadRequest, "plugin_id cannot be empty")
	}
	err = models.SetSelectedPlugin(appId, pluginId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to set selected plugin", err)
	}
	models.AddOperation(appId, models.OperationTypeSetSelectedPlugin, o.Ctx.Input.IP(),
		"set up selected plugin: "+pluginId)
	o.ServeWithEmptyData()
}

// @router /email/test [post]
func (o *AppController) TestEmail() {
	var param map[string]string
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	err = models.PushEmailAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test email alarm", err)
	}
}

// @router /ding/test [post]
func (o *AppController) TestDing(config map[string]interface{}) {
	var param map[string]string
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	err = models.PushDingAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test ding ding alarm", err)
	}
}

// @router /http/test [post]
func (o *AppController) TestHttp(config map[string]interface{}) {
	var param map[string]string
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	err = models.PushHttpAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test http alarm", err)
	}
}
