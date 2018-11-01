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
	"net/http"
	"rasp-cloud/models"
	"rasp-cloud/controllers"
	"encoding/json"
	"time"
	"strings"
	"github.com/astaxie/beego/validation"
	"strconv"
	"gopkg.in/mgo.v2"
)

// Operations about app
type AppController struct {
	controllers.BaseController
}

// @router / [get]
func (o *AppController) GetApp() {
	id := o.GetString("id")
	if id == "" {
		page, err := o.GetInt("page")
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get page param: "+err.Error())
		}
		if page <= 0 {
			o.ServeError(http.StatusBadRequest, "page must be greater than 0")
		}
		perpage, err := o.GetInt("perpage")
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get perpage param: "+err.Error())
		}
		if perpage <= 0 {
			o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
		}
		var result = make(map[string]interface{})
		total, apps, err := models.GetAllApp(page, perpage)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get apps: "+err.Error())
		}
		if apps == nil {
			apps = make([]models.App, 0)
		}
		result["total"] = total
		result["count"] = len(apps)
		result["data"] = apps
		o.Serve(result)
	} else {
		app, err := models.GetAppById(id)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get app: "+err.Error())
		}
		o.Serve(app)
	}
}

// @router /rasp [get]
func (o *AppController) GetRasps() {
	appId := o.GetString("app_id")
	page, err := o.GetInt("page")
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get page param: "+err.Error())
	}
	if page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	perpage, err := o.GetInt("perpage")
	if err != nil {
		o.ServeError(http.StatusBadRequest, err.Error())
	}
	if perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "failed to get perpage param: "+"perpage must be greater than 0")
	}

	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app: "+err.Error())
	}
	if app == nil {
		o.ServeError(http.StatusBadRequest, "the app doesn't exist")
	}
	var result = make(map[string]interface{})
	total, rasps, err := models.GetRaspByAppId(app.Id, page, perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get apps: "+err.Error())
	}
	result["total"] = total
	result["count"] = len(rasps)
	result["data"] = rasps
	o.Serve(result)
}

// @router /rasp/config [post]
func (o *AppController) ConfigRasp() {
	var param map[string]interface{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error： "+err.Error())
	}
	appIdParam := param["app_id"]
	if appIdParam == nil {
		o.ServeError(http.StatusBadRequest, "the app_id cannot be empty")
	}
	appId, ok := appIdParam.(string)
	if !ok {
		o.ServeError(http.StatusBadRequest, "the app_id must be string")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app from mongodb: "+err.Error())
	}

	configParam := param["config"]
	if configParam == nil {
		o.ServeError(http.StatusBadRequest, "the config cannot be empty")
	}
	config, ok := configParam.(map[string]interface{})
	if !ok {
		o.ServeError(http.StatusBadRequest, "the type of config must be object")
	}
	o.validateAppConfig(config)

	configTime := time.Now().UnixNano()
	app.ConfigTime = configTime
	app.RaspConfig = config
	err = models.UpdateAppById(app.Id, app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app config: "+err.Error())
	}
	o.Serve(app)
}

// @router / [post]
func (o *AppController) Post() {
	var app = &models.App{}

	err := json.Unmarshal(o.Ctx.Input.RequestBody, app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error： "+err.Error())
	}
	if app.Name == "" {
		o.ServeError(http.StatusBadRequest, "app name cannot be empty")
	}
	if len(app.Name) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app name cannot be greater than 64")
	}
	if len(app.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of app description can not be greater than 1024")
	}
	if len(app.SelectedPluginId) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of app selected_plugin_id can not be greater than 1024")
	}
	if app.EmailAlarmConf != nil {
		o.validEmailConf(app.EmailAlarmConf)
	}
	if app.HttpAlarmConf != nil {
		o.validHttpAlarm(app.HttpAlarmConf)
	}
	if app.DingAlarmConf != nil {
		o.validDingConf(app.DingAlarmConf)
	}
	if app.RaspConfig != nil {
		o.validateAppConfig(app.RaspConfig)
		configTime := time.Now().UnixNano()
		app.ConfigTime = configTime
	} else {
		app.RaspConfig = make(map[string]interface{})
	}

	app, err = models.AddApp(app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "create app failed: "+err.Error())
	}
	o.Serve(app)
}

func (o *AppController) validEmailConf(conf *models.EmailAlarmConf) {
	var valid = validation.Validation{}
	if conf.EmailServerAddr == "" {
		o.ServeError(http.StatusBadRequest, "the email_server_addr cannot be empty")
	}
	if len(conf.EmailServerAddr) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email_server_addr cannot be greater than 128")
	}
	if len(conf.EmailSubject) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email_subject cannot be greater than 256")
	}
	if conf.EmailFromAddr == "" {
		o.ServeError(http.StatusBadRequest, "the email_from_addr cannot be empty")
	}
	if len(conf.EmailFromAddr) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email_from_addr cannot be greater than 256")
	}
	if conf.EmailFromAddr == "" {
		o.ServeError(http.StatusBadRequest, "the email_from_addr cannot be empty")
	}
	if result := valid.Email(conf.EmailFromAddr, "email"); !result.Ok {
		o.ServeError(http.StatusBadRequest, "the email_from_addr format error: "+result.Error.Message)
	}
	if conf.EmailPassword == "" {
		o.ServeError(http.StatusBadRequest, "the email_password cannot be empty")
	}
	if len(conf.EmailPassword) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email_password cannot be greater than 256")
	}
	if len(conf.EmailRecvAddr) == 0 {
		o.ServeError(http.StatusBadRequest, "the email_recv_addr cannot be empty")
	}
	if len(conf.EmailRecvAddr) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of email_recv_addr cannot be greater than 128")
	}
	conf.EmailRecvAddr = o.validAppArrayParam(conf.EmailRecvAddr, "email_recv_addr", valid.Email)
}

func (o *AppController) validDingConf(conf *models.DingAlarmConf) {
	if conf.DingCorpId == "" {
		o.ServeError(http.StatusBadRequest, "the ding_corp_id cannot be empty")
	}
	if len(conf.DingCorpId) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of ding_corp_id cannot be greater than 128")
	}
	if conf.DingCorpSecret == "" {
		o.ServeError(http.StatusBadRequest, "the ding_corp_secret cannot be empty")
	}
	if len(conf.DingCorpSecret) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of ding_corp_secret cannot be greater than 128")
	}
	if len(conf.DingRecvPart) == 0 && len(conf.DingRecvUser) == 0 {
		o.ServeError(http.StatusBadRequest, "ding_recv_part and ding_recv_user cannot be empty at the same time")
	}
	if len(conf.DingRecvPart) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of ding_recv_part cannot be greater than 128")
	}
	if len(conf.DingRecvUser) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of ding_recv_user cannot be greater than 128")
	}
	if conf.DingAgentId == "" {
		o.ServeError(http.StatusBadRequest, "the ding_agent_id cannot be empty")
	}
	if len(conf.DingAgentId) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of ding_agent_id cannot be greater than 128")
	}
	conf.DingRecvUser = o.validAppArrayParam(conf.DingRecvUser, "ding_recv_user", nil)
	conf.DingRecvPart = o.validAppArrayParam(conf.DingRecvPart, "ding_recv_part", nil)
}

func (o *AppController) validHttpAlarm(conf *models.HttpAlarmConf) {
	if len(conf.HttpRecvAddr) == 0 {
		o.ServeError(http.StatusBadRequest, "the http_recv_addr cannot be empty")
	}
	if len(conf.HttpRecvAddr) > 128 {
		o.ServeError(http.StatusBadRequest, "the count of http_recv_addr cannot be greater than 128")
	}
	conf.HttpRecvAddr = o.validAppArrayParam(conf.HttpRecvAddr, "http_recv_addr", nil)
}

// @router /delete [post]
func (o *AppController) Delete() {
	var app = &models.App{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error： "+err.Error())
	}
	if app.Id == "" {
		o.ServeError(http.StatusBadRequest, "the id cannot be empty")
	}
	err = models.RemoveAppById(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove app： "+err.Error())
	}
	err = models.RemoveRaspByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove rasp by app_id： "+err.Error())
	}

	o.ServeWithoutData()
}

func (o *AppController) validAppArrayParam(param []string, paramName string,
	valid func(interface{}, string) (*validation.Result)) ([]string) {
	if param != nil {
		if len(param) > 128 {
			o.ServeError(http.StatusBadRequest,
				"the count of "+paramName+" http_recv_addr cannot be greater than 128")
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
		o.ServeError(http.StatusBadRequest, "the config of app cannot be nil")
	}
	for key, value := range config {
		if value == nil {
			o.ServeError(http.StatusBadRequest, "the value of "+key+" config cannot be nil")
		}
		if strings.HasPrefix(key, "hook.white.") {
			whiteUrls, ok := value.([]string)
			if !ok {
				o.ServeError(http.StatusBadRequest,
					"the type of "+key+" config must be string array")
			}

			if len(whiteUrls) == 0 || len(whiteUrls) > 10 {
				o.ServeError(http.StatusBadRequest,
					"the count of hook.white's url array must be between (0,10]")
			}
			for _, url := range whiteUrls {
				if len(url) > 200 || len(url) < 1 {
					o.ServeError(http.StatusBadRequest,
						"the length of hook.white's url must be between [1,200]")
				}
			}
		} else {
			if v, ok := value.(string); ok {
				if len(v) >= 512 {
					o.ServeError(http.StatusBadRequest,
						"the length of config key "+key+" must less tha 1024")
				}
			}
		}
	}
}

// @router /alarm/config [post]
func (o *AppController) ConfigAlarm() {

}

// @router /plugins [get]
func (o *AppController) GetPlugins() {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id param can not be empty")
	}
	page, err := o.GetInt("page")
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get page param: "+err.Error())
	}
	if page <= 0 {
		o.ServeError(http.StatusBadRequest, "page param must be greater than 0")
	}
	perpage, err := o.GetInt("perpage")
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get perpage param: "+err.Error())
	}
	if perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage param must be greater than 0")
	}
	total, plugins, err := models.GetPluginsByApp(appId, (page-1)*perpage, perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get plugins: "+err.Error())
	}
	result := make(map[string]interface{})
	result["total"] = total
	result["data"] = plugins
	o.Serve(result)
}

// @router /plugin/selected [get]
func (o *AppController) GetSelectedPlugin() {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	plugin, err := models.GetSelectedPlugin(appId)
	if mgo.ErrNotFound == err {
		o.ServeWithoutData()
		return
	}
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get selected plugin: "+err.Error())
	}
	o.Serve(plugin)
}

// @router /plugin/select [get]
func (o *AppController) SetSelectedPlugin() {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	pluginId := o.GetString("plugin_id")
	if pluginId == "" {
		o.ServeError(http.StatusBadRequest, "plugin_id cannot be empty")
	}
	err := models.SetSelectedPlugin(appId, pluginId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to set selected plugin: "+err.Error())
	}
	o.ServeWithoutData()
}

// @router /email/test [get]
func (o *AppController) TestEmail() {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id param can not be empty")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app: "+err.Error())
	}
	err = models.PushEmailAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test email alarm: "+err.Error())
	}
}

// @router /ding/test [get]
func (o *AppController) TestDing(config map[string]interface{}) {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id param can not be empty")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app: "+err.Error())
	}
	err = models.PushDingAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test ding ding alarm: "+err.Error())
	}
}

// @router /http/test [get]
func (o *AppController) TestHttp(config map[string]interface{}) {
	appId := o.GetString("app_id")
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id param can not be empty")
	}
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app: "+err.Error())
	}
	err = models.PushHttpAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test http alarm: "+err.Error())
	}
}
