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

package api

import (
	"encoding/json"
	"fmt"
	"github.com/astaxie/beego/validation"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"io"
	"io/ioutil"
	"math"
	"net/http"
	"os"
	"rasp-cloud/controllers"
	"rasp-cloud/kafka"
	"rasp-cloud/models"
	"rasp-cloud/tools"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"time"
)

// Operations about app
type AppController struct {
	controllers.BaseController
}

type pageParam struct {
	AppId   string `json:"app_id"`
	Name    string `json:"name"`
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
	o.UnmarshalJson(&data)
	if data.AppId != "" {
		app, err := models.GetAppById(data.AppId)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get app", err)
		}
		o.Serve(app)
	} else if data.Name != "" {
		o.ValidPage(data.Page, data.Perpage)
		var result = make(map[string]interface{})
		count, apps, err := models.GetAppByName(data.Name, data.Page, data.Perpage)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get app by name", err)
		}
		if apps == nil {
			apps = make([]*models.App, 0)
		}
		result["total"] = count
		result["page"] = data.Page
		result["perpage"] = data.Perpage
		result["data"] = apps
		o.Serve(result)
	} else {
		o.ValidPage(data.Page, data.Perpage)
		var result = make(map[string]interface{})
		total, apps, err := models.GetAllApp(data.Page, data.Perpage, true)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to get apps", err)
		}
		if apps == nil {
			apps = make([]*models.App, 0)
		}
		result["total"] = total
		result["total_page"] = math.Ceil(float64(total) / float64(data.Perpage))
		result["page"] = data.Page
		result["perpage"] = data.Perpage
		result["data"] = apps
		o.Serve(result)
	}
}

// @router /rasp/get [post]
func (o *AppController) GetRasps() {
	var param pageParam
	o.UnmarshalJson(&param)
	o.ValidPage(param.Page, param.Perpage)

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
	if rasps == nil {
		rasps = make([]*models.Rasp, 0)
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
	o.UnmarshalJson(&param)
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
	o.UnmarshalJson(&param)
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	secret, err := models.RegenerateSecret(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get secret", err)
	}
	models.AddOperation(param.AppId, models.OperationTypeRegenerateSecret,
		o.Ctx.Input.IP(), "Reset AppSecret of "+param.AppId)
	o.Serve(map[string]string{
		"secret": secret,
	})
}

// @router /general/config [post]
func (o *AppController) UpdateAppGeneralConfig() {
	var param struct {
		StrategyId	string		           `json:"strategy_id,omitempty"`
		AppId       string                 `json:"app_id"`
		Config      map[string]interface{} `json:"config"`
	}
	o.UnmarshalJson(&param)

	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	if param.Config == nil {
		o.ServeError(http.StatusBadRequest, "config can not be empty")
	}

	param.Config = o.validateAppConfig(param.Config)
	app, err := models.UpdateGeneralConfig(param.AppId, param.Config)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app general config", err)
	}
	// clean up rasp config
	if param.Config["offline_hosts.cleanup.interval"].(float64) > 0 {
		models.HasOfflineHosts[app.Id] = param.Config["offline_hosts.cleanup.interval"].(float64)
	}
	//_, err = models.UpdateGeneralConfigForStrategy(param.StrategyId, param.AppId, param.Config)
	//if err != nil {
	//	o.ServeError(http.StatusBadRequest, "failed to update strategy general config", err)
	//}
	models.AddOperation(param.AppId, models.OperationTypeUpdateGenerateConfig,
		o.Ctx.Input.IP(), "Updated general config of "+param.AppId)
	o.Serve(app)
}

// @router /whitelist/config [post]
func (o *AppController) UpdateAppWhiteListConfig() {
	var param struct {
		StrategyId	string					     `json:"strategy_id,omitempty"`
		AppId       string                       `json:"app_id"`
		Config      []models.WhitelistConfigItem `json:"config"`
	}
	o.UnmarshalJson(&param)

	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	if param.Config == nil {
		o.ServeError(http.StatusBadRequest, "config can not be empty")
	}
	o.validateWhiteListConfig(param.Config)
	param.Config = o.RemoveDupWhitelistConfigItem(param.Config)
	app, err := models.UpdateWhiteListConfig(param.AppId, param.Config)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app whitelist config", err)
	}
	//_, err = models.UpdateWhiteListConfigForStrategy(param.StrategyId, param.AppId, param.Config)
	//if err != nil {
	//	o.ServeError(http.StatusBadRequest, "failed to update app general config", err)
	//}
	models.AddOperation(param.AppId, models.OperationTypeUpdateWhitelistConfig,
		o.Ctx.Input.IP(), "Updated whitelist config of "+param.AppId)
	o.Serve(app)
}

// @router / [post]
func (o *AppController) Post() {
	var app = &models.App{}

	o.UnmarshalJson(app)

	if app.Name == "" {
		o.ServeError(http.StatusBadRequest, "app name cannot be empty")
	}
	if len(app.Name) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app name cannot be greater than 64")
	}
	if app.Language == "" {
		o.ServeError(http.StatusBadRequest, "app programming language cannot be empty")
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
		o.ServeError(http.StatusBadRequest, "Unsupported programming language: "+app.Language)
	}
	if len(app.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of the app description can not be greater than 1024")
	}
	if len(app.SelectedPluginId) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of the app selected_plugin_id can not be greater than 1024")
	}

	if app.AttackTypeAlarmConf != nil {
		o.validAttackTypeAlarmConf(app.AttackTypeAlarmConf)
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
		app.GeneralConfig = o.validateAppConfig(app.GeneralConfig)
		configTime := time.Now().UnixNano()
		app.ConfigTime = configTime
	}
	if app.WhitelistConfig != nil {
		o.validateWhiteListConfig(app.WhitelistConfig)
		configTime := time.Now().UnixNano()
		app.ConfigTime = configTime
	} else {
		app.WhitelistConfig = make([]models.WhitelistConfigItem, 0)
	}
	app, err := models.AddApp(app)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "create app failed", err)
	}
	models.AddOperation(app.Id, models.OperationTypeAddApp, o.Ctx.Input.IP(), "New app created with name "+app.Name)
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

	o.UnmarshalJson(&param)
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	_, err := models.GetAppById(param.AppId)
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
	models.AddOperation(app.Id, models.OperationTypeEditApp, o.Ctx.Input.IP(), "Updated app info for "+param.AppId+": "+string(operationData))
	o.Serve(app)
}

// @router /export [get]
func (o * AppController) ExportApp() {
	fileName := "files/app.json"
	pathName := "files"
	apps, err := models.GetAllExportApp()
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get apps", err)
	}
	appBytes, err := json.Marshal(apps)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to convert apps", err)
	}
	if isExist, _ := tools.PathExists(pathName); !isExist {
		err := os.MkdirAll(pathName, os.ModePerm)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "create dir failed", err)
		}
	}
	if ioutil.WriteFile(fileName, appBytes, os.ModePerm) != nil {
		o.ServeError(http.StatusBadRequest, "failed to write file", err)
	}
	file, err := os.Open(fileName)
	if err != nil {
		if strings.Index(err.Error(), "no such file or directory") != -1 {
			o.ServeWithEmptyData()
		} else {
			o.ServeError(http.StatusBadRequest, "open file err", err)
		}
	}
	defer file.Close()
	o.Ctx.Output.Header("Content-Type", "application/json")
	o.Ctx.Output.Header("content-disposition", "attachment; filename=app.json")
	_, err = io.Copy(o.Ctx.ResponseWriter, file)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "download file err:", err)
		return
	}
}

func (o *AppController) validEmailConf(conf *models.EmailAlarmConf) {
	var valid = validation.Validation{}
	if conf.ServerAddr == "" {
		o.ServeError(http.StatusBadRequest, "the email server_addr cannot be empty")
	}
	if len(conf.ServerAddr) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email server_addr cannot be greater than 256")
	}
	if len(conf.From) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of from cannot be greater than 256")
	}
	if len(conf.Subject) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email subject cannot be greater than 256")
	}
	if len(conf.UserName) > 256 {
		o.ServeError(http.StatusBadRequest, "the length of email username cannot be greater than 256")
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
		o.ServeError(http.StatusBadRequest, "the length of ding agent_id cannot be greater than 256")
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

func (o *AppController) validAttackTypeAlarmConf(conf *map[string][]string) {
	if conf != nil {
		for k, v := range *conf {
			if k == "" {
				o.ServeError(http.StatusBadRequest, "the attack type can not be empty")
			}
			if len(k) > 128 {
				o.ServeError(http.StatusBadRequest, "the length of attack type can not be greater than 128")
			}
			if len(v) > 0 {
				if len(v) > 64 {
					o.ServeError(http.StatusBadRequest,
						"the length of alarm array can not be greater than 64")
				}
				for _, item := range v {
					if item == "" {
						o.ServeError(http.StatusBadRequest, "the alarm type can not be empty")
					}
					found := false
					for _, alarmType := range models.AlarmTypes {
						if item == alarmType {
							found = true
						}
					}
					if !found {
						o.ServeError(http.StatusBadRequest, "the alarm type must be in: "+
							fmt.Sprintf("%v", models.AlarmTypes))
					}
				}
			}

		}
	}
}

func (o *AppController) validKafkaConf(conf *kafka.Kafka) {
	if conf.KafkaAddr == "" {
		o.ServeError(http.StatusBadRequest, "the kafka addr cannot be empty")
	}
	if conf.KafkaTopic == "" {
		o.ServeError(http.StatusBadRequest, "the kafka topic cannot be empty")
	}
}

func (o *AppController) validGeneralAlarmConf(conf *models.GeneralAlarmConf) {
	if conf.AlarmCheckInterval < models.MinAlarmCheckInterval {
		o.ServeError(http.StatusBadRequest, "the minimum interval may exceed min interval")
	}
}

// @router /delete [post]
func (o *AppController) Delete() {
	var app = &models.App{}
	o.UnmarshalJson(app)

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
	online := true
	raspCount, _, err := models.FindRasp(&models.Rasp{AppId: app.Id, Online: &online}, 1, 1)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to find rasps for this app", err)
	}
	if raspCount > 0 {
		o.ServeError(http.StatusBadRequest, "failed to remove this app, it also has online rasps")
	}
	app, err = models.RemoveAppById(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove this app", err)
	}
	err = models.RemoveRaspByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove rasps by app_id", err)
	}
	err = models.RemovePluginByAppId(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove plugins by app_id", err)
	}
	err = models.RemoveDependencyByApp(app.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove dependency data by app_id", err)
	}
	models.AddOperation(app.Id, models.OperationTypeDeleteApp, o.Ctx.Input.IP(), "Deleted app with name "+app.Name)
	o.ServeWithEmptyData()
}

func (o *AppController) validAppArrayParam(param []string, paramName string,
	valid func(interface{}, string) *validation.Result) []string {
	if param != nil {
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

func (o *AppController) validateAppConfig(config map[string]interface{}) map[string]interface{} {
	generalConfigTemplate := models.DefaultGeneralConfig
	returnConfig := make(map[string]interface{})
	for key, v := range generalConfigTemplate {
		value := config[key]
		if value == nil || value == "" {
			config[key] = v
		}
		returnConfig[key] = config[key]
		if key == "" {
			o.ServeError(http.StatusBadRequest,
				"the config key can not be empty")
		}
		if len(key) > 512 {
			o.ServeError(http.StatusBadRequest,
				"the length of config key '"+key+"' must be less than 512")
		}
		if v, ok := value.(string); ok {
			if key == "body.maxbytes" {
				maxBytes := generalConfigTemplate[key]
				if len(v) >= maxBytes.(int) {
					o.ServeError(http.StatusBadRequest,
						"the value's length of config item '"+key+"' must be less than" + v)
				}
			}
		}

		// 对类型和值进行检验
		if generalConfigTemplate[key] != nil && value != nil {
			if key == "dependency_check.interval" || key == "fileleak_scan.interval" {
				if interval, ok := value.(float64); ok {
					if interval < 60 || interval > 12 * 3600 {
						o.ServeError(http.StatusBadRequest,
							"the value's length of config item '"+key+"' must between 60 and 86400")
					}
				} else if interval, ok := value.(int); ok {
					if interval < 60 || interval > 12 * 3600 {
						o.ServeError(http.StatusBadRequest,
							"the value's length of config item '"+key+"' must between 60 and 86400")
					}
				}
			}

			if key != "security.weak_passwords" {
				switch reflect.TypeOf(generalConfigTemplate[key]).String(){
				case "string":
					valueStr, ok := value.(string)
					if !ok {
						o.ServeError(http.StatusBadRequest,
							"the type of config key: "+key+"'s value must be send a string")
					}
					if key == "block.content_html" || key == "block.content_xml" ||
						key == "block.content_json" || key == "fileleak_scan.name"{
						if strings.TrimSpace(valueStr) == "" {
							returnConfig[key] = v
						}
					}
				case "int64":
					if _, ok := value.(int64); !ok {
						o.ServeError(http.StatusBadRequest,
							"the type of config key: "+key+"'s value must be send a int64")
					}
				case "bool":
					if _, ok := value.(bool); !ok {
						o.ServeError(http.StatusBadRequest,
							"the type of config key: "+key+"'s value must be send a bool")
					}
				case "int", "float64":
					if _, ok := value.(float64); !ok {
						if value == "" || reflect.TypeOf(value).String() == "int" {
							config[key] = v
						} else {
							o.ServeError(http.StatusBadRequest,
								"the type of config key: "+key+"'s value must be send a int/float64")
						}
					}
				}
			} else {
				if valueArray, ok := value.([]interface{}); ok {
					tmpArray := []interface{}{}
					for _, ele := range valueArray {
						if strings.TrimSpace(ele.(string)) != "" {
							tmpArray = append(tmpArray, ele)
						}
					}
					if len(tmpArray) == 0 {
						returnConfig[key] = generalConfigTemplate["security.weak_passwords"]
					} else {
						for idx, v := range value.([]interface{}) {
							if len(v.(string)) > 16 {
								o.ServeError(http.StatusBadRequest,
									"the length of value:" + v.(string) + " exceeds max_len 16!")
							}
							if idx >= 200 {
								o.ServeError(http.StatusBadRequest,
									"the count of weak_password exceed 200!")
							}
						}
					}
				}
			}
			if key == "inject.custom_headers" {
				for hk, hv := range value.(map[string]interface{}) {
					if len(hk) >= 200 {
						o.ServeError(http.StatusBadRequest,
							"the value's length of config item '"+hk+"' must be less than 200")
					}
					if hv, ok := hv.(string); ok {
						if len(hv) >= 200 {
							o.ServeError(http.StatusBadRequest,
								"the value's length of config item '"+hv+"' must be less than 200")
						}
					} else {
						o.ServeError(http.StatusBadRequest,
							"the inject.custom_headers's value cannot convert to type string")
					}
				}
			}
			if v, ok := value.(float64); ok {
				if v < 0 {
					o.ServeError(http.StatusBadRequest,
						"the value of config item '"+key+"' can not be less than 0")
				} else if key == "plugin.timeout.millis" || key == "body.maxbytes" ||
					key == "syslog.reconnect_interval" || key == "ognl.expression.minlength" {
					if v == 0 {
						o.ServeError(http.StatusBadRequest,
							"the value of config item '"+key+"' must be greater than 0")
					}
				}
			}
		}
	}
	return returnConfig
}

func (o *AppController) RemoveDupWhitelistConfigItem(a []models.WhitelistConfigItem) (ret []models.WhitelistConfigItem){
	n := len(a)
	for i:=0; i < n; i++{
		state := false
		for j := i+1 ; j < n; j++{
			if (j > 0 && reflect.DeepEqual(a[i],a[j])){
				state = true
				break
			}
		}
		if !state {
			ret = append(ret, a[i])
		}
	}
	return
}

func (o *AppController) validateWhiteListConfig(config []models.WhitelistConfigItem) {
	if len(config) > 200 {
		o.ServeError(http.StatusBadRequest,
			"the count of whitelist config items must be between (0,200]")
	}
	for _, value := range config {
		if len(value.Url) > 200 || len(value.Url) == 0 {
			o.ServeError(http.StatusBadRequest,
				"the length of whitelist config url must be between [1,200]")
		}
		if len(value.Description) > 200 {
			o.ServeError(http.StatusBadRequest,
				"the length of whitelist config desc can not be greater than 200")
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
		AppId               string                   `json:"app_id"`
		AttackTypeAlarmConf *map[string][]string     `json:"attack_type_alarm_conf,omitempty"`
		EmailAlarmConf      *models.EmailAlarmConf   `json:"email_alarm_conf,omitempty"`
		DingAlarmConf       *models.DingAlarmConf    `json:"ding_alarm_conf,omitempty"`
		HttpAlarmConf       *models.HttpAlarmConf    `json:"http_alarm_conf,omitempty"`
		KafkaConf           *kafka.Kafka             `json:"kafka_alarm_conf,omitempty"`
		GeneralAlarmConf    *models.GeneralAlarmConf `json:"general_alarm_conf"`
	}
	o.UnmarshalJson(&param)

	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	app, err := models.GetAppByIdWithoutMask(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	var updateData bson.M
	if param.EmailAlarmConf != nil {
		if param.EmailAlarmConf.Password == models.SecreteMask {
			param.EmailAlarmConf.Password = app.EmailAlarmConf.Password
		}
		o.validEmailConf(param.EmailAlarmConf)
	}
	if param.HttpAlarmConf != nil {
		o.validHttpAlarm(param.HttpAlarmConf)
	}
	if param.DingAlarmConf != nil {
		if param.DingAlarmConf.CorpSecret == models.SecreteMask {
			param.DingAlarmConf.CorpSecret = app.DingAlarmConf.CorpSecret
		}
		o.validDingConf(param.DingAlarmConf)
	}
	if param.AttackTypeAlarmConf != nil {
		o.validAttackTypeAlarmConf(param.AttackTypeAlarmConf)
	}
	if param.KafkaConf != nil {
		o.validKafkaConf(param.KafkaConf)
		err = kafka.PutKafkaConfig(param.AppId, param.KafkaConf)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to put kafka config", err)
		}
	}
	if param.GeneralAlarmConf != nil {
		models.AlarmCheckInterval = param.GeneralAlarmConf.AlarmCheckInterval
		o.validGeneralAlarmConf(param.GeneralAlarmConf)
		err = models.UpdateGeneralAlarmConfig(param.GeneralAlarmConf)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to update alarm config", err)
		}
	}
	content, err := json.Marshal(param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to encode param to json", err)
	}
	err = json.Unmarshal(content, &updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to decode param json", err)
	}
	app, err = models.UpdateAppById(param.AppId, updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update app", err)
	}
	models.AddOperation(app.Id, models.OperationTypeUpdateAlarmConfig, o.Ctx.Input.IP(),
		"Alarm configuration updated for "+param.AppId)
	o.Serve(app)
}

// @router /plugin/get [post]
func (o *AppController) GetPlugins() {
	var param pageParam
	o.UnmarshalJson(&param)
	o.ValidPage(param.Page, param.Perpage)

	app, err := models.GetAppById(param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	if app == nil {
		o.ServeError(http.StatusBadRequest, "the app doesn't exist")
	}
	var result = make(map[string]interface{})
	total, plugins, err := models.GetPluginsByApp(param.AppId, (param.Page-1)*param.Perpage,
		param.Perpage, "-upload_time")
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
	o.UnmarshalJson(&param)

	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	plugin, err := models.GetSelectedPlugin(appId, false)

	if err != nil {
		if mgo.ErrNotFound == err {
			o.ServeWithEmptyData()
			return
		}
		o.ServeError(http.StatusBadRequest, "failed to get selected plugin", err)
	}

	o.Serve(plugin)
}

// @router /plugin/select [post]
func (o *AppController) SetSelectedPlugin() {
	var param map[string]string
	o.UnmarshalJson(&param)
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	pluginId := param["plugin_id"]
	if pluginId == "" {
		o.ServeError(http.StatusBadRequest, "plugin_id cannot be empty")
	}
	plugin, err := models.SetSelectedPlugin(appId, pluginId, "")
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to set selected plugin", err)
	}
	models.AddOperation(appId, models.OperationTypeSetSelectedPlugin, o.Ctx.Input.IP(),
		"Deployed plugin "+plugin.Name+": "+plugin.Version+" ["+plugin.Id+"]")
	o.ServeWithEmptyData()
}

// @router /email/test [post]
func (o *AppController) TestEmail() {
	var param map[string]string
	o.UnmarshalJson(&param)
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppByIdWithoutMask(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	if !app.EmailAlarmConf.Enable {
		o.ServeError(http.StatusBadRequest, "please enable the email alarm first")
	}
	err = models.PushEmailAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test email alarm", err)
	}
	o.ServeWithEmptyData()
}

// @router /ding/test [post]
func (o *AppController) TestDing() {
	var param map[string]string
	o.UnmarshalJson(&param)
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppByIdWithoutMask(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	if !app.DingAlarmConf.Enable {
		o.ServeError(http.StatusBadRequest, "please enable the ding ding alarm first")
	}
	err = models.PushDingAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test ding ding alarm", err)
	}
	o.ServeWithEmptyData()
}

// @router /http/test [post]
func (o *AppController) TestHttp() {
	var param map[string]string
	o.UnmarshalJson(&param)
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppByIdWithoutMask(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	if !app.HttpAlarmConf.Enable {
		o.ServeError(http.StatusBadRequest, "please enable the http alarm first")
	}
	err = models.PushHttpAttackAlarm(app, 0, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test http alarm", err)
	}
	o.ServeWithEmptyData()
}

// @router /kafka/test [post]
func (o *AppController) TestKafka() {
	var param map[string]string
	o.UnmarshalJson(&param)
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	app, err := models.GetAppByIdWithoutMask(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "can not find the app", err)
	}
	if !app.KafkaConf.KafkaEnable {
		o.ServeError(http.StatusBadRequest, "please enable the kafka first")
	}
	err = models.PushKafkaAttackAlarm(app, nil, true)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to test kafka alarm", err)
	}
	o.ServeWithEmptyData()
}

// @router /plugin/latest [post]
func (o *AppController) CheckPluginLatest() {
	var param map[string]string
	o.UnmarshalJson(&param)
	appId := param["app_id"]
	if appId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	latestVersion := ""
	currentVersion := ""
	app, err := models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get the app", err)
	}
	selectedPlugin, err := models.GetPluginById(app.SelectedPluginId, false)
	if err != nil && err != mgo.ErrNotFound {
		o.ServeError(http.StatusBadRequest, "failed to get the app", err)
	}
	if selectedPlugin != nil {
		if selectedPlugin.Name != "official" {
			o.Serve(map[string]interface{}{
				"is_latest": true,
			})
			return
		}
		currentVersion = selectedPlugin.Version
	}

	latestPlugins, err := models.SearchPlugins(bson.M{"app_id": appId, "name": "official"},
		0, 1, "-version")
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get plugins for app: "+appId, err)
	}
	if len(latestPlugins) > 0 {
		latestVersion = latestPlugins[0].Version
		if selectedPlugin == nil || strings.Compare(selectedPlugin.Version, latestPlugins[0].Version) < 0 {
			o.Serve(map[string]interface{}{
				"is_latest":        false,
				"selected_version": currentVersion,
				"latest_version":   latestVersion,
			})
			return
		}
	}

	o.Serve(map[string]interface{}{
		"is_latest": true,
	})
}
