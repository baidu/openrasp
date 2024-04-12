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
	"bytes"
	"context"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/tls"
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"html/template"
	"io/ioutil"
	"math/rand"
	"net"
	"net/mail"
	"net/smtp"
	"net/url"
	"os"
	"rasp-cloud/conf"
	"rasp-cloud/es"
	"rasp-cloud/kafka"
	"rasp-cloud/models/logs"
	"rasp-cloud/mongo"
	"rasp-cloud/tools"
	"strconv"
	"strings"
	"time"

	"github.com/astaxie/beego"
	"github.com/astaxie/beego/httplib"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
)

type App struct {
	Id               string                 `json:"id" bson:"_id"`
	Name             string                 `json:"name"  bson:"name"`
	Secret           string                 `json:"secret"  bson:"secret"`
	Language         string                 `json:"language"  bson:"language"`
	Description      string                 `json:"description"  bson:"description"`
	CreateTime       int64                  `json:"create_time"  bson:"create_time"`
	ConfigTime       int64                  `json:"config_time"  bson:"config_time"`
	GeneralConfig    map[string]interface{} `json:"general_config"  bson:"general_config"`
	WhitelistConfig  []WhitelistConfigItem  `json:"whitelist_config"  bson:"whitelist_config"`
	SelectedPluginId string                 `json:"selected_plugin_id" bson:"selected_plugin_id"`
	// AttackTypeAlarmConf 该字段为了兼容之前没该字段的问题，该字段为 nil 的情况下代表所有攻击类型都选中所有报警类型
	AttackTypeAlarmConf *map[string][]string   `json:"attack_type_alarm_conf" bson:"attack_type_alarm_conf"`
	EmailAlarmConf      EmailAlarmConf         `json:"email_alarm_conf" bson:"email_alarm_conf"`
	DingAlarmConf       DingAlarmConf          `json:"ding_alarm_conf" bson:"ding_alarm_conf"`
	HttpAlarmConf       HttpAlarmConf          `json:"http_alarm_conf" bson:"http_alarm_conf"`
	AlgorithmConfig     map[string]interface{} `json:"algorithm_config"`
	GeneralAlarmConf    GeneralAlarmConf       `json:"general_alarm_conf" bson:"general_alarm_conf"`
	KafkaConf           *kafka.Kafka           `json:"kafka_alarm_conf" bson:"kafka_alarm_conf"`
}

type ExportAPP struct {
	Id          string `json:"id" bson:"_id"`
	Name        string `json:"name"  bson:"name"`
	Language    string `json:"language"  bson:"language"`
	Secret      string `json:"secret"  bson:"secret"`
	Description string `json:"description"  bson:"description"`
}

type WhitelistConfigItem struct {
	Url         string          `json:"url" bson:"url"`
	Hook        map[string]bool `json:"hook" bson:"hook"`
	Description string          `json:"description" bson:"description"`
}

type GeneralAlarmConf struct {
	AlarmCheckInterval int64 `json:"alarm_check_interval" bson:"alarm_check_interval"`
}

type EmailAlarmConf struct {
	Enable     bool     `json:"enable" bson:"enable"`
	From       string   `json:"from" bson:"from"`
	ServerAddr string   `json:"server_addr" bson:"server_addr"`
	UserName   string   `json:"username" bson:"username"`
	Password   string   `json:"password" bson:"password"`
	Subject    string   `json:"subject" bson:"subject"`
	RecvAddr   []string `json:"recv_addr" bson:"recv_addr"`
	TlsEnable  bool     `json:"tls_enable" bson:"tls_enable"`
}

type DingAlarmConf struct {
	Enable     bool     `json:"enable" bson:"enable"`
	AgentId    string   `json:"agent_id" bson:"agent_id"`
	CorpId     string   `json:"corp_id" bson:"corp_id"`
	CorpSecret string   `json:"corp_secret" bson:"corp_secret"`
	RecvUser   []string `json:"recv_user" bson:"recv_user"`
	RecvParty  []string `json:"recv_party" bson:"recv_party"`
}

type HttpAlarmConf struct {
	Enable   bool     `json:"enable" bson:"enable"`
	RecvAddr []string `json:"recv_addr" bson:"recv_addr"`
}

type emailTemplateParam struct {
	Total        int64
	Alarms       []map[string]interface{}
	DetailedLink string
	AppName      string
	HttpPort     int
}

type dingResponse struct {
	ErrCode     int64  `json:"errcode"`
	ErrMsg      string `json:"errmsg"`
	AccessToken string `json:"access_token"`
}

var AlarmTypes = []string{"email", "ding", "http"}

const (
	appCollectionName    = "app"
	configCollectionName = "config"
	defaultAppName       = "PHP 示例应用"
	SecreteMask          = "************"
	DefalutPluginName    = "plugin.js"
	IastPluginName       = "iast.js"
)

var (
	lastAlarmTime        = time.Now().UnixNano() / 1000000
	DefaultGeneralConfig = map[string]interface{}{
		"clientip.header":    "ClientIP",
		"block.status_code":  302,
		"block.redirect_url": "https://rasp.baidu.com/blocked/?request_id=%request_id%",
		"block.content_xml": "<?xml version=\"1.0\"?><doc><error>true</error><reason>Request blocked by OpenRASP" +
			"</reason><request_id>%request_id%</request_id></doc>",
		"block.content_html": "</script><script>" +
			"location.href=\"https://rasp.baidu.com/blocked2/?request_id=%request_id%\"</script>",
		"block.content_json":    `{"error":true,"reason": "Request blocked by OpenRASP","request_id": "%request_id%"}`,
		"plugin.timeout.millis": 100,
		"body.maxbytes":         12288,
		"inject.custom_headers": map[string]interface{}{
			"X-Protected-By": "OpenRASP",
		},
		"plugin.filter":             true,
		"cpu.usage.enable":          false,
		"lru.compare_enable":        false,
		"plugin.maxstack":           100,
		"ognl.expression.minlength": 30,
		"log.maxstack":              50,
		"log.maxburst":              100,
		"log.maxbackup":             30,
		"syslog.tag":                "OpenRASP",
		"syslog.url":                "",
		"syslog.facility":           1,
		"syslog.enable":             false,
		"decompile.enable":          false,
		"security.weak_passwords": []string{
			"111111", "123", "123123", "123456", "123456a",
			"a123456", "admin", "both", "manager", "mysql",
			"root", "rootweblogic", "tomcat", "user",
			"weblogic1", "weblogic123", "welcome1",
		},
		"request.param_encoding":         "",
		"debug.level":                    0,
		"lru.max_size":                   1000,
		"lru.compare_limit":              10240,
		"fileleak_scan.name":             `\.(git|svn|tar|gz|rar|zip|sql|log)$`,
		"fileleak_scan.interval":         21600,
		"fileleak_scan.limit":            100,
		"cpu.usage.interval":             5,
		"cpu.usage.percent":              90,
		"response.sampler_interval":      60,
		"response.sampler_burst":         5,
		"dependency_check.interval":      12 * 3600,
		"offline_hosts.cleanup.interval": 0,
	}
	AlarmCheckInterval    = conf.AppConfig.AlarmCheckInterval
	MinAlarmCheckInterval = conf.AppConfig.AlarmCheckInterval
)

func init() {
	count, err := mongo.Count(appCollectionName)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to get app collection count", err)
	}

	index := &mgo.Index{
		Key:        []string{"name"},
		Unique:     true,
		Background: true,
		Name:       "app_name",
	}
	err = mongo.CreateIndex(appCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to create index for app collection", err)
	}
	if *conf.AppConfig.Flag.Upgrade == "121to122" {
		err := UpdateAppConfig(*conf.AppConfig.Flag.Upgrade)
		if err != nil {
			beego.Error("failed to update App config")
		}
	}
	if *conf.AppConfig.Flag.StartType == conf.StartTypeDefault ||
		*conf.AppConfig.Flag.StartType == conf.StartTypeForeground {
		if count <= 0 {
			createDefaultApp()
		}
		generalConf, _ := getGeneralConfig()
		if generalConf != nil && generalConf.AlarmCheckInterval > MinAlarmCheckInterval {
			AlarmCheckInterval = generalConf.AlarmCheckInterval
		}
		go startAlarmTicker()
	}
	if *conf.AppConfig.Flag.StartType != conf.StartTypeReset {
		initApp()
		TaskCleanUpHosts()
	}
}

func initApp() error {
	var apps []*App
	_, err := mongo.FindAllWithoutLimit(appCollectionName, nil, &apps)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to get all app", err)
	}
	if HasOfflineHosts == nil {
		HasOfflineHosts = make(map[string]float64)
	}
	for _, app := range apps {
		err := createEsIndexWithAppId(app.Id)
		if err != nil {
			tools.Panic(tools.ErrCodeESInitFailed, "failed to init es index for app "+app.Name, err)
		}
		if *conf.AppConfig.Flag.StartType != conf.StartTypeAgent {
			err := initPlugin(app, DefalutPluginName)
			if err != nil {
				beego.Warn(tools.ErrCodeInitDefaultAppFailed, "failed to init plugin.js for app ["+app.Name+"]", err)
			}
			err = initPlugin(app, IastPluginName)
			if err != nil {
				beego.Warn(tools.ErrCodeInitDefaultAppFailed, "failed to init iast.js for app ["+app.Name+"]", err)
			}
		}
		if len(app.GeneralConfig) < len(DefaultGeneralConfig) {
			err = initAppDefaultConfig(app)
		}
		if dayInterval, ok := app.GeneralConfig["offline_hosts.cleanup.interval"].(float64); ok && dayInterval > 0 {
			HasOfflineHosts[app.Id] = dayInterval
		}
	}
	return nil
}

func initAppDefaultConfig(app *App) error {
	var err error
	beego.Info("Detected new app config for this app:" + app.Name)
	// 新增默认配置，在这里初始化。省去刷库的步骤（只初始化简单类型）
	for key, value := range DefaultGeneralConfig {
		if key != "inject.custom_headers" && app.GeneralConfig[key] == nil {
			app.GeneralConfig[key] = value
			err = mongo.UpsertId(appCollectionName, app.Id, app)
		}
	}
	return err
}

func initPlugin(app *App, pluginName string) error {
	content, err := getDefaultPluginContent(pluginName)
	if err != nil {
		return err
	}
	plugin, err := generatePlugin(content, app.Id)
	if err != nil {
		return err
	}
	err = mongo.FindOne(pluginCollectionName, bson.M{
		"name":    plugin.Name,
		"version": plugin.Version,
		"app_id":  plugin.AppId}, plugin)
	if err != nil {
		if err == mgo.ErrNotFound {
			err = addPluginToDb(plugin)
			if err != nil {
				return err
			}
		} else {
			return err
		}
	}
	return nil
}

func createEsIndexWithAppId(appId string) error {
	err := logs.CreateAlarmEsIndex(appId)
	if err != nil {
		return errors.New("failed to create alarm es index, " + err.Error())
	}
	err = CreateReportDataEsIndex(appId)
	if err != nil {
		return errors.New("failed to create report data es index, " + err.Error())
	}
	err = CreateDependencyEsIndex(appId)
	if err != nil {
		return errors.New("failed to create dependency data es index, " + err.Error())
	}
	return nil
}

func createDefaultApp() {
	_, err := AddApp(&App{
		Name:        defaultAppName,
		Description: "default app",
		Language:    "php",
	})
	if err != nil {
		tools.Panic(tools.ErrCodeInitDefaultAppFailed, "failed to create default app", err)
	}
	// 初始化全局配置
	var config GeneralAlarmConf
	config.AlarmCheckInterval = MinAlarmCheckInterval
	err = UpdateGeneralAlarmConfig(&config)
	if err != nil {
		tools.Panic(tools.ErrCodeInitDefaultAppFailed, "failed to init general config", err)
	}
}

func startAlarmTicker() {
	timer := time.NewTimer(time.Second * time.Duration(AlarmCheckInterval))
	for {
		select {
		case <-timer.C:
			HandleAttackAlarm()
		}
		timer.Reset(time.Second * time.Duration(AlarmCheckInterval))
	}
}

func HandleAttackAlarm() {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to handle alarm: ", r)
		}
	}()
	var apps []App
	_, err := mongo.FindAllWithSelect(appCollectionName, nil, &apps, bson.M{"plugin": 0}, 0, 0)
	if err != nil {
		beego.Error("failed to get apps for the alarm: " + err.Error())
		return
	}
	now := time.Now().UnixNano() / 1000000
	for _, app := range apps {
		if app.AttackTypeAlarmConf != nil {
			attackConf := map[string][]interface{}{}
			for k, v := range *app.AttackTypeAlarmConf {
				for _, item := range v {
					if attackConf[item] == nil {
						attackConf[item] = make([]interface{}, 0)
					}
					attackConf[item] = append(attackConf[item], k)
				}
			}
			for k, v := range attackConf {
				query := map[string]interface{}{"attack_type": v}
				total, result, err := logs.SearchLogs(lastAlarmTime, now, false, query, "@timestamp",
					1, 10, false, logs.AttackAlarmInfo.EsAliasIndex+"-"+app.Id)
				if err != nil {
					beego.Error("failed to get alarm from es for alarm type " + k + ": " + err.Error())
					continue
				}
				if total > 0 {
					PushAttackAlarm(&app, total, result, false, k)
				}
			}
		} else {
			total, result, err := logs.SearchLogs(lastAlarmTime, now, false, nil, "@timestamp",
				1, 10, false, logs.AttackAlarmInfo.EsAliasIndex+"-"+app.Id)
			if err != nil {
				beego.Error("failed to get alarm from es: " + err.Error())
				continue
			}
			if total > 0 {
				PushAttackAlarm(&app, total, result, false)
			}
		}
	}
	lastAlarmTime = now + 1
}

func AddApp(app *App) (result *App, err error) {
	app.Id = generateAppId(app)
	app.Secret = generateSecret(app)
	app.CreateTime = time.Now().Unix()
	if err := mongo.FindOne(appCollectionName, bson.M{"name": app.Name}, &App{}); err != mgo.ErrNotFound {
		if err != nil {
			return nil, err
		}
		return nil, errors.New("duplicate app name")
	}
	HandleApp(app, true)
	err = createEsIndexWithAppId(app.Id)
	if err != nil {
		return nil, errors.New("failed to create es index for app " + app.Name + ", " + err.Error())
	}
	// ES must be created before mongo
	err = mongo.Insert(appCollectionName, app)
	if err != nil {
		return nil, errors.New("failed to insert app to db: " + err.Error())
	}
	result = app
	beego.Info("Succeed to create app, name: " + app.Name)
	selectDefaultPlugin(app)
	return
}

func getDefaultPluginContent(pluginName string) ([]byte, error) {
	// if setting default plugin fails, continue to initialize
	pluginPath := "resources/" + pluginName
	content, err := ioutil.ReadFile(pluginPath)
	if err != nil {
		return nil, errors.New("failed to read " + pluginPath + ": " + err.Error())
	}
	return content, err
}

func selectDefaultPlugin(app *App) {
	// if setting default plugin fails, continue to initialize
	content, err := getDefaultPluginContent(DefalutPluginName)
	if err != nil {
		beego.Warn(tools.ErrCodeInitDefaultAppFailed, "failed to get default plugin: "+err.Error())
		return
	}
	plugin, err := AddPlugin(content, app.Id)
	if err != nil {
		beego.Warn(tools.ErrCodeInitDefaultAppFailed, "failed to insert default plugin: "+err.Error())
		return
	}
	_, err = SetSelectedPlugin(app.Id, plugin.Id, "")
	if err != nil {
		beego.Warn(tools.ErrCodeInitDefaultAppFailed, "failed to select default plugin for app: "+err.Error()+
			", app_id: "+app.Id+", plugin_id: "+plugin.Id)
		return
	}
	err = initPlugin(app, IastPluginName)
	if err != nil {
		beego.Warn(tools.ErrCodeInitDefaultAppFailed, "failed to init iast plugin: "+err.Error()+
			", app_id: "+app.Id+", plugin_id: "+plugin.Id)
		return
	}
	beego.Info("Succeed to set up default plugin for app, version: " + plugin.Version)
}

func generateAppId(app *App) string {
	random := "openrasp_app" + app.Name + strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(10000))
	return fmt.Sprintf("%x", sha1.Sum([]byte(random)))
}

func generateSecret(app *App) string {
	random := "openrasp_app" + app.Name + app.Id +
		strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(10000))
	sha256Data := sha256.Sum256([]byte(random))
	base64Data := base64.StdEncoding.EncodeToString(sha256Data[0:])
	return base64Data[0 : len(base64Data)-1]
}

func GetAllApp(page int, perpage int, mask bool) (count int, result []*App, err error) {
	count, err = mongo.FindAll(appCollectionName, nil, &result, perpage*(page-1), perpage, "name")
	if err == nil && result != nil {
		for _, app := range result {
			if mask {
				HandleApp(app, false)
			}
		}
	}
	return
}

func GetAppByIdWithoutMask(id string) (app *App, err error) {
	err = mongo.FindId(appCollectionName, id, &app)
	generalConf, _ := getGeneralConfig()
	if generalConf != nil {
		app.GeneralAlarmConf.AlarmCheckInterval = generalConf.AlarmCheckInterval
	}
	return
}

func GetAppById(id string) (app *App, err error) {
	err = mongo.FindId(appCollectionName, id, &app)
	if err == nil && app != nil {
		HandleApp(app, false)
	}
	return
}

func GetAppByName(name string, page int, perpage int) (count int, result []*App, err error) {
	// 支持模糊查询
	selector := bson.M{"name": bson.M{
		"$regex":   name,
		"$options": "$i",
	}}
	count, err = mongo.FindAll(appCollectionName, selector, &result, perpage*(page-1), perpage)
	if err == nil && result != nil {
		for _, app := range result {
			HandleApp(app, false)
		}
	}
	return
}

func GetSecretByAppId(appId string) (secret string, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	var result *App
	err = newSession.DB(mongo.DbName).C(appCollectionName).FindId(appId).Select(bson.M{"secret": 1}).One(&result)
	if err != nil {
		return
	}
	if result != nil {
		secret = result.Secret
	}
	return
}

func GetEmailConfByAppId(appId string) (e EmailAlarmConf, err error) {
	newSession := mongo.NewSession()
	defer newSession.Close()
	var result *App
	err = newSession.DB(mongo.DbName).C(appCollectionName).FindId(appId).Select(bson.M{"email_alarm_conf": 1}).One(&result)
	if err != nil {
		return
	}
	if result != nil {
		e = result.EmailAlarmConf
	}
	return
}

func getGeneralConfig() (conf *GeneralAlarmConf, err error) {
	var result struct {
		GeneralAlarmConf GeneralAlarmConf `json:"general_alarm_conf" bson:"general_alarm_conf"`
	}
	err = mongo.FindId(configCollectionName, "0", &result)
	if err != nil && &result != nil {
		return
	}
	conf = &result.GeneralAlarmConf
	return
}

func RegenerateSecret(appId string) (secret string, err error) {
	var app *App
	err = mongo.FindId(appCollectionName, appId, &app)
	if err != nil {
		return
	}
	newSecret := generateSecret(app)
	err = mongo.UpdateId(appCollectionName, appId, bson.M{"secret": newSecret})
	return
}

func HandleApp(app *App, isCreate bool) error {
	generalConf, _ := getGeneralConfig()
	if generalConf != nil {
		app.GeneralAlarmConf.AlarmCheckInterval = generalConf.AlarmCheckInterval
	}
	if app.EmailAlarmConf.RecvAddr == nil {
		app.EmailAlarmConf.RecvAddr = make([]string, 0)
	}
	if app.DingAlarmConf.RecvParty == nil {
		app.DingAlarmConf.RecvParty = make([]string, 0)
	}
	if app.DingAlarmConf.RecvUser == nil {
		app.DingAlarmConf.RecvUser = make([]string, 0)
	}
	if app.HttpAlarmConf.RecvAddr == nil {
		app.HttpAlarmConf.RecvAddr = make([]string, 0)
	}
	if !isCreate {
		if app.EmailAlarmConf.Password != "" {
			app.EmailAlarmConf.Password = SecreteMask
		}
		if app.DingAlarmConf.CorpSecret != "" {
			app.DingAlarmConf.CorpSecret = SecreteMask
		}
	} else {
		if app.GeneralConfig == nil {
			app.GeneralConfig = DefaultGeneralConfig
		}
	}
	if app.WhitelistConfig == nil {
		app.WhitelistConfig = make([]WhitelistConfigItem, 0)
	}
	if app.GeneralConfig == nil {
		app.GeneralConfig = make(map[string]interface{})
	}
	if app.KafkaConf != nil {
		if app.KafkaConf.KafkaPwd != "" {
			app.KafkaConf.KafkaPwd = SecreteMask
		}
	}
	app.AlgorithmConfig = make(map[string]interface{})
	plugin, err := GetSelectedPlugin(app.Id, false)
	if err != nil {
		if err != mgo.ErrNotFound {
			return err
		}
		return nil
	}
	if plugin != nil && plugin.AlgorithmConfig != nil {
		app.AlgorithmConfig = plugin.AlgorithmConfig
	}
	return nil
}

func UpdateAppById(id string, doc interface{}) (app *App, err error) {
	err = mongo.UpdateId(appCollectionName, id, doc)
	if err != nil {
		return
	}
	return GetAppById(id)
}

func UpdateConfigById(collection string, id string, doc interface{}) (err error) {
	err = mongo.UpsertId(collection, id, doc)
	if err != nil {
		return
	}
	return err
}

func UpdateGeneralConfig(appId string, config map[string]interface{}) (*App, error) {
	return UpdateAppById(appId, bson.M{"general_config": config, "config_time": time.Now().UnixNano()})
}

func UpdateWhiteListConfig(appId string, config []WhitelistConfigItem) (app *App, err error) {
	return UpdateAppById(appId, bson.M{"whitelist_config": config, "config_time": time.Now().UnixNano()})
}

func UpdateGeneralAlarmConfig(config *GeneralAlarmConf) (err error) {
	return UpdateConfigById(configCollectionName, "0", bson.M{"general_alarm_conf": config})
}

func UpdateAppConfig(version string) error {
	if version == "121to122" {
		page := 1
		perPage := 10
		for {
			_, apps, err := GetAllApp(page, perPage, true)
			if err != nil {
				beego.Error("failed to update App config", err)
				return err
			}
			if apps == nil {
				apps = make([]*App, 0)
			}
			for _, app := range apps {
				appId := app.Id
				maxBytes := app.GeneralConfig["body.maxbytes"]
				switch maxBytes.(type) {
				case float64:
					if uint64(maxBytes.(float64)) == 4096 {
						maxBytes = 12288
					}
				case int:
					if maxBytes == 4096 {
						maxBytes = 12288
					}
				}
				app.GeneralConfig["inject.custom_headers"] = map[string]interface{}{
					"X-Protected-By": "OpenRASP",
				}
				_, err := UpdateGeneralConfig(appId, app.GeneralConfig)
				if err != nil {
					beego.Error("failed to update app general config after update", err)
					return err
				}
			}
			if len(apps) < perPage {
				break
			}
			page++
		}
		beego.Info("update 121to122 success!")
	}
	return nil
}

func RemoveAppById(id string) (app *App, err error) {
	err = mongo.FindId(appCollectionName, id, &app)
	if err != nil {
		return
	}
	return app, mongo.RemoveId(appCollectionName, id)
}

func GetAppCount() (count int, err error) {
	return mongo.Count(appCollectionName)
}

func PushAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool, alarmType ...string) {
	if app != nil {
		AddAppNameToAlarms(alarms)
		if (len(alarmType) == 0 || alarmType[0] == "ding") && app.DingAlarmConf.Enable {
			PushDingAttackAlarm(app, total, alarms, isTest)
		}
		if (len(alarmType) == 0 || alarmType[0] == "email") && app.EmailAlarmConf.Enable {
			PushEmailAttackAlarm(app, total, alarms, isTest)
		}
		if (len(alarmType) == 0 || alarmType[0] == "http") && app.HttpAlarmConf.Enable {
			PushHttpAttackAlarm(app, total, alarms, isTest)
		}
	}
}

func getTestAlarmData(app *App) []map[string]interface{} {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
	defer cancel()

	indexName := fmt.Sprintf("openrasp-attack-alarm-%s", app.Id)
	queryResult, err := es.ElasticClient.Search(indexName).
		Sort("event_time", false).
		From(0).
		Size(2).
		Do(ctx)
	if err != nil {
		beego.Error(err)
		return nil
	}

	if queryResult == nil || queryResult.Hits == nil && queryResult.Hits.Hits == nil {
		return nil
	}

	hits := queryResult.Hits.Hits
	if len(hits) == 0 {
		beego.Error("No data available, please create at least one alarm to continue")
		return nil
	}

	result := make([]map[string]interface{}, len(hits))
	for index, item := range hits {
		result[index] = make(map[string]interface{})
		if err := json.Unmarshal(*item.Source, &result[index]); err != nil {
			beego.Error(err)
		}
	}

	AddAppNameToAlarms(result)
	return result
}

func PushEmailAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var emailConf = app.EmailAlarmConf
	if len(emailConf.RecvAddr) > 0 && emailConf.ServerAddr != "" {
		var (
			subject   string
			msg       string
			emailAddr = &mail.Address{Address: emailConf.UserName}
		)
		if emailConf.From != "" {
			emailAddr.Name = emailConf.From
		} else {
			hostName, err := os.Hostname()
			if err == nil {
				emailAddr.Name = hostName
			} else {
				emailAddr.Name = "OpenRASP"
			}
		}
		if emailConf.Subject == "" {
			subject = "OpenRASP alarm"
		} else {
			subject = emailConf.Subject
		}
		if isTest {
			subject = "【测试邮件】" + subject
			alarms = getTestAlarmData(app)
			total = int64(len(alarms))
		}
		head := map[string]string{
			"From":              emailAddr.String(),
			"To":                strings.Join(emailConf.RecvAddr, ","),
			"Subject":           subject,
			"Content-Type":      "text/html; charset=UTF-8",
			"X-Priority":        "3",
			"X-MSMail-Priority": "Normal",
			"X-Mailer":          "Microsoft Outlook Express 6.00.2900.2869",
			"X-MimeOLE":         "Produced By Microsoft MimeOLE V6.00.2900.2869",
			"ReturnReceipt":     "1",
		}
		t, err := template.ParseFiles("views/email.tpl")
		if err != nil {
			beego.Error("failed to render email template: " + err.Error())
			return err
		}
		alarmData := new(bytes.Buffer)
		panelUrl, port := getPanelServerUrl()
		handleAlarms(alarms)
		err = t.Execute(alarmData, &emailTemplateParam{
			Total:        total - int64(len(alarms)),
			Alarms:       alarms,
			AppName:      app.Name,
			DetailedLink: panelUrl + "/#/events/" + app.Id,
			HttpPort:     port,
		})
		if err != nil {
			beego.Error("failed to execute email template: " + err.Error())
			return err
		}
		for k, v := range head {
			msg += fmt.Sprintf("%s: %s\r\n", k, v)
		}
		msg += "\r\n" + alarmData.String()
		if !strings.Contains(emailConf.ServerAddr, ":") {
			if emailConf.TlsEnable {
				emailConf.ServerAddr += ":465"
			} else {
				emailConf.ServerAddr += ":25"
			}
		}
		//host, _, err := net.SplitHostPort(emailConf.ServerAddr)
		//if err != nil {
		//	return handleError("failed to get email serve host: " + err.Error())
		//}
		auth := tools.LoginAuth(emailConf.UserName, emailConf.Password)
		//auth := smtp.PlainAuth("", emailConf.UserName, emailConf.Password, host)
		//auth := smtp.CRAMMD5Auth(emailConf.UserName, emailConf.Password)
		if emailConf.Password == "" {
			auth = nil
		}
		if emailConf.TlsEnable {
			return SendEmailWithTls(emailConf, auth, []byte(msg))
		}
		return SendNormalEmail(emailConf, auth, []byte(msg))
	} else {
		beego.Error(
			"failed to send email alarm: the email receiving address and email server address can not be empty", emailConf)
		return errors.New("the email receiving address and email server address can not be empty")
	}
}

func AddAppNameToAlarms(alarms []map[string]interface{}) {
	var (
		app     *App
		appName = ""
	)

	if len(alarms) == 0 {
		return
	}

	// 一个RASP只能绑定一个app_id，因此只需要查询一次; app查询失败，也应该发出报警
	if appId, ok := alarms[0]["app_id"]; ok {
		appIdStr, ok := appId.(string)

		if ok {
			if err := mongo.FindId(appCollectionName, appIdStr, &app); err == nil {
				appName = app.Name
			} else {
				// beego.Error(err)
			}
		}
	}

	for _, alarm := range alarms {
		alarm["app_name"] = appName
	}
}

func handleAlarms(alarms []map[string]interface{}) {
	for index, alarm := range alarms {
		alarm["index"] = index + 1
		if intercept, ok := logs.AttackInterceptMap[alarm["intercept_state"]]; ok {
			alarm["intercept_state"] = intercept
		}

		if attackType, ok := logs.AttackTypeMap[alarm["attack_type"]]; ok {
			alarm["attack_type"] = attackType
		}

		if alarm["url"] != nil {
			if attackUrl, ok := alarm["url"].(string); ok && attackUrl != "" {
				attackUrl, err := url.Parse(attackUrl)
				if err == nil {
					alarm["domain"] = attackUrl.Host
				}
			}
		}

		if alarm["client_ip"] != nil {
			if clientIp, ok := alarm["client_ip"].(string); ok && clientIp != "" {
				alarm["attack_source"] = clientIp
			}
		}
	}
}

func getPanelServerUrl() (string, int) {
	serverUrl, err := GetServerUrl()

	if err != nil && err != mgo.ErrNotFound {
		beego.Error("failed to get panel url for alarm: " + err.Error())
	}

	port := beego.AppConfig.DefaultInt("httpport", 8080)
	if serverUrl == nil || len(serverUrl.PanelUrl) == 0 {
		return "", port
	}

	return serverUrl.PanelUrl, port
}

func SendNormalEmail(emailConf EmailAlarmConf, auth smtp.Auth, msg []byte) (err error) {
	err = smtp.SendMail(emailConf.ServerAddr, auth, emailConf.UserName, emailConf.RecvAddr, msg)
	if err != nil {
		beego.Error("failed to push email alarms: " + err.Error())
		return
	}
	return
}

func SendEmailWithTls(emailConf EmailAlarmConf, auth smtp.Auth, msg []byte) error {
	client, err := smtpTlsDial(emailConf.ServerAddr)
	if err != nil && !strings.Contains(err.Error(), "timeout") {
		conn, err := net.DialTimeout("tcp", emailConf.ServerAddr, time.Second*5)
		if err != nil {
			return handleError("failed to dial with email server: " + err.Error())
		}
		host, _, _ := net.SplitHostPort(emailConf.ServerAddr)
		client, err = smtp.NewClient(conn, host)
		if err != nil {
			return handleError("failed to dial with email server: " + err.Error())
		}
	}
	if err != nil {
		return handleError("failed to start tls dial: " + err.Error())
	}
	if client.Text != nil {
		defer client.Close()
	}
	if auth != nil {
		if ok, _ := client.Extension("AUTH"); ok {
			if err = client.Auth(auth); err != nil {
				return handleError("failed to auth with tls: " + err.Error())
			}
		}
	}
	if err = client.Mail(emailConf.UserName); err != nil {
		return handleError("failed to mail from 'emailConf.UserName': " + err.Error())
	}

	for _, addr := range emailConf.RecvAddr {
		if err = client.Rcpt(addr); err != nil {
			return handleError("failed to push email to " + addr + " with tls: " + err.Error())
		}
	}

	writer, err := client.Data()
	if err != nil {
		return handleError("failed to get writer for email with tls: " + err.Error())
	}
	defer writer.Close()

	_, err = writer.Write(msg)
	if err != nil {
		return handleError("failed to write msg with tls: " + err.Error())
	}

	if client.Text != nil {
		client.Quit()
	}

	return nil
}

func handleError(msg string) error {
	beego.Error(msg)
	return errors.New(msg)
}

func smtpTlsDial(addr string) (*smtp.Client, error) {
	host, _, err := net.SplitHostPort(addr)
	if err != nil {
		return nil, handleError("failed to get email serve host: " + err.Error())
	}
	conn, err := tls.DialWithDialer(&net.Dialer{Timeout: time.Second * 5}, "tcp",
		addr, &tls.Config{InsecureSkipVerify: true})
	if err != nil {
		return nil, handleError("smtp dialing error: " + err.Error())
	}
	return smtp.NewClient(conn, host)
}

func PushHttpAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var httpConf = app.HttpAlarmConf
	if len(httpConf.RecvAddr) != 0 {
		body := make(map[string]interface{})
		body["app_id"] = app.Id
		if isTest {
			body["data"] = getTestAlarmData(app)
		} else {
			body["data"] = alarms
		}
		for _, addr := range httpConf.RecvAddr {
			request := httplib.Post(addr)
			request.JSONBody(body)
			request.SetTimeout(10*time.Second, 10*time.Second)
			response, err := request.Response()
			if err != nil {
				return handleError("failed to push http alarms to: " + addr + ", with error: " + err.Error())
			}
			if response.StatusCode > 299 || response.StatusCode < 200 {
				return handleError("failed to push http alarms to: " + addr + ", with status code: " +
					strconv.Itoa(response.StatusCode))
			}
		}
	} else {
		return handleError("failed to send http alarm: the http receiving address can not be empty")
	}
	beego.Debug("succeed in pushing http alarm for app: " + app.Name + " ,with urls: " +
		fmt.Sprintf("%v", httpConf.RecvAddr))
	return nil
}

func PushDingAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var dingCong = app.DingAlarmConf
	if dingCong.CorpId != "" && dingCong.CorpSecret != "" && dingCong.AgentId != "" &&
		!(len(dingCong.RecvParty) == 0 && len(dingCong.RecvUser) == 0) {

		request := httplib.Get("https://oapi.dingtalk.com/gettoken")
		request.SetTimeout(10*time.Second, 10*time.Second)
		request.Param("corpid", dingCong.CorpId)
		request.Param("corpsecret", dingCong.CorpSecret)
		response, err := request.Response()
		errMsg := "failed to get ding ding token with corp id: " + dingCong.CorpId
		if err != nil {
			return handleError(errMsg + ", with error: " + err.Error())
		}
		if response.StatusCode != 200 {
			return handleError(errMsg + ", with status code: " + strconv.Itoa(response.StatusCode))
		}
		var result dingResponse
		err = request.ToJSON(&result)
		if err != nil {
			return handleError(errMsg + ", with error: " + err.Error())
		}
		if result.ErrCode != 0 {
			return handleError(errMsg + ", with errmsg: " + result.ErrMsg)
		}
		token := result.AccessToken
		body := make(map[string]interface{})
		dingText := ""
		if isTest {
			dingText = "OpenRASP test message from app: " + app.Name + ", time: " + time.Now().Format(time.RFC3339)
		} else {
			panelUrl, _ := getPanelServerUrl()
			if len(panelUrl) == 0 {
				panelUrl = "http://127.0.0.1"
			}
			dingText = "时间：" + time.Now().Format(time.RFC3339) + "， 来自 OpenRAS 的报警\n共有 " +
				strconv.FormatInt(total, 10) + " 条报警信息来自 APP：" + app.Name + "，详细信息：" +
				panelUrl + "/#/events/" + app.Id
		}
		if len(dingCong.RecvUser) > 0 {
			body["touser"] = strings.Join(dingCong.RecvUser, "|")
		}
		if len(dingCong.RecvParty) > 0 {
			body["toparty"] = strings.Join(dingCong.RecvParty, "|")
		}
		body["agentid"] = dingCong.AgentId
		body["msgtype"] = "text"
		body["text"] = map[string]string{"content": dingText}
		request = httplib.Post("https://oapi.dingtalk.com/message/send?access_token=" + token)
		request.JSONBody(body)
		request.SetTimeout(10*time.Second, 10*time.Second)
		response, err = request.Response()
		errMsg = "failed to push ding ding alarms with corp id: " + dingCong.CorpId
		if err != nil {
			return handleError(errMsg + ", with error: " + err.Error())
		}
		if response.StatusCode != 200 {
			return handleError(errMsg + ", with status code: " + strconv.Itoa(response.StatusCode))
		}
		err = request.ToJSON(&result)
		if err != nil {
			return handleError(errMsg + ", with error: " + err.Error())
		}
		if result.ErrCode != 0 {
			return handleError(errMsg + ", with errmsg: " + result.ErrMsg)
		}
	} else {
		return handleError("failed to send ding ding alarm: invalid ding ding alarm conf")
	}
	beego.Debug("succeed in pushing ding ding alarm for app: " + app.Name + " ,with corp id: " + dingCong.CorpId)
	return nil
}

func PushKafkaAttackAlarm(app *App, alarms []map[string]interface{}, isTest bool) error {
	var kafkaConf = app.KafkaConf
	addrs := strings.Split(kafkaConf.KafkaAddr, ",")
	if len(addrs) != 0 {
		body := make(map[string]interface{})
		body["app_id"] = app.Id
		if isTest {
			body["data"] = getTestAlarmData(app)
		} else {
			body["data"] = alarms
		}

		for _, addr := range addrs {
			err := kafka.SendMessage(body["app_id"].(string), "kafka-test", body["data"].([]map[string]interface{})[0])
			if err != nil {
				return handleError("failed to push kafka alarms to: " + addr + ", with error: " + err.Error())
			}
		}
	} else {
		return handleError("failed to send kafka alarm: the http receiving address can not be empty")
	}
	beego.Debug("succeed in pushing kafka alarm for app: " + app.Name + " ,with urls: " +
		fmt.Sprintf("%v", addrs))
	return nil
}

func GetAllExportApp() (apps []*ExportAPP, err error) {
	_, err = mongo.FindAllWithoutLimit(appCollectionName, nil, &apps)
	if err != nil {
		return nil, err
	}
	return
}
