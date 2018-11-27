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
	"rasp-cloud/mongo"
	"fmt"
	"strconv"
	"time"
	"math/rand"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2"
	"crypto/sha1"
	"gopkg.in/mgo.v2/bson"
	"rasp-cloud/es"
	"rasp-cloud/models/logs"
	"github.com/astaxie/beego"
	"net/smtp"
	"os"
	"net/mail"
	"strings"
	"html/template"
	"bytes"
	"github.com/astaxie/beego/httplib"
	"errors"
	"crypto/sha256"
	"encoding/base64"
)

type App struct {
	Id               string                 `json:"id" bson:"_id"`
	Name             string                 `json:"name"  bson:"name"`
	Secret           string                 `json:"secret"  bson:"secret"`
	Language         string                 `json:"language"  bson:"language"`
	Description      string                 `json:"description"  bson:"description"`
	ConfigTime       int64                  `json:"config_time"  bson:"config_time"`
	GeneralConfig    map[string]interface{} `json:"general_config"  bson:"general_config"`
	WhitelistConfig  []WhitelistConfigItem  `json:"whitelist_config"  bson:"whitelist_config"`
	SelectedPluginId string                 `json:"selected_plugin_id" bson:"selected_plugin_id"`
	EmailAlarmConf   EmailAlarmConf         `json:"email_alarm_conf" bson:"email_alarm_conf"`
	DingAlarmConf    DingAlarmConf          `json:"ding_alarm_conf" bson:"ding_alarm_conf"`
	HttpAlarmConf    HttpAlarmConf          `json:"http_alarm_conf" bson:"http_alarm_conf"`
}

type WhitelistConfigItem struct {
	Url  string          `json:"url" bson:"url"`
	Hook map[string]bool `json:"hook" bson:"hook"`
}

type EmailAlarmConf struct {
	Enable     bool     `json:"enable" bson:"enable"`
	ServerAddr string   `json:"server_addr" bson:"server_addr"`
	UserName   string   `json:"username" bson:"username"`
	Password   string   `json:"password" bson:"password"`
	Subject    string   `json:"subject" bson:"subject"`
	RecvAddr   []string `json:"recv_addr" bson:"recv_addr"`
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
}

type dingResponse struct {
	ErrCode     int64  `json:"errcode"`
	ErrMsg      string `json:"errmsg"`
	AccessToken string `json:"access_token"`
}

const (
	appCollectionName = "app"
)

var (
	lastAlarmTime        = time.Now()
	DefaultGeneralConfig = map[string]interface{}{
		"clientip.header":    "ClientIP",
		"block.status_code":  302,
		"block.redirect_url": "https://rasp.baidu.com/blocked/?request_id=%request_id%",
		"block.content_xml": "<?xml version=\"1.0\"?><doc><error>true</error><reason>Request blocked by OpenRASP" +
			"</reason><request_id>%request_id%</request_id></doc>",
		"block.content_html": "</script><script>" +
			"location.href=\"https://rasp.baidu.com/blocked2/?request_id=%request_id%\"</script>",
		"block.content_json":        `{"error":true,"reason": "Request blocked by OpenRASP","request_id": "%request_id%"}`,
		"plugin.timeout.millis":     100,
		"body.maxbytes":             4096,
		"plugin.filter":             true,
		"plugin.maxstack":           100,
		"ognl.expression.minlength": 30,
		"log.maxstack":              10,
	}
)

func init() {
	count, err := mongo.Count(appCollectionName)
	if err != nil {
		tools.Panic("failed to get app collection count", err)
	}
	if count <= 0 {
		index := &mgo.Index{
			Key:        []string{"name"},
			Unique:     true,
			Background: true,
			Name:       "app_name",
		}
		err = mongo.CreateIndex(appCollectionName, index)
		if err != nil {
			tools.Panic("failed to create index for app collection", err)
		}
	}
	alarmDuration := beego.AppConfig.DefaultInt64("AlarmDuration", 120)
	if alarmDuration <= 0 {
		tools.Panic("the 'AlarmDuration' config must be greater than 0", nil)
	}
	go startAlarmTicker(time.Second * time.Duration(alarmDuration))
}

func startAlarmTicker(duration time.Duration) {
	ticker := time.NewTicker(duration)
	for {
		select {
		case <-ticker.C:
			handleAttackAlarm()
			handleRaspExpiredAlarm()
		}
	}
}

func handleAttackAlarm() {
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
	now := time.Now()
	for _, app := range apps {
		total, result, err := logs.SearchLogs(lastAlarmTime.Unix()*1000, now.Unix()*1000, nil, "event_time",
			1, 3, false, logs.AliasAttackIndexName+"-"+app.Id)
		if err != nil {
			beego.Error("failed to get alarm from es: " + err.Error())
			continue
		}
		if total > 0 {
			PushAttackAlarm(&app, total, result, false)
		}
	}
	lastAlarmTime = now
}

func handleRaspExpiredAlarm() {
	//defer func() {
	//	if r := recover(); r != nil {
	//		beego.Error("failed to handle alarm: ", r)
	//	}
	//}()
	//Rasp
}

func AddApp(app *App) (result *App, err error) {
	app.Id = generateAppId(app)
	app.Secret = generateSecret(app)
	if mongo.FindOne(appCollectionName, bson.M{"name": app.Name}, &App{}) != mgo.ErrNotFound {
		return nil, errors.New("duplicate app name")
	}
	err = es.CreateEsIndex(logs.PolicyIndexName+"-"+app.Id, logs.AliasPolicyIndexName+"-"+app.Id, logs.PolicyEsMapping)
	if err != nil {
		return
	}
	err = es.CreateEsIndex(logs.AttackIndexName+"-"+app.Id, logs.AliasAttackIndexName+"-"+app.Id, logs.AttackEsMapping)
	if err != nil {
		return
	}
	err = es.CreateEsIndex(ReportIndexName+"-"+app.Id, AliasReportIndexName+"-"+app.Id, ReportEsMapping)
	if err != nil {
		return
	}
	// ES must be created before mongo
	err = mongo.Insert(appCollectionName, app)
	if err != nil {
		return
	}
	result = app
	return
}

func generateAppId(app *App) string {
	random := "openrasp_app" + app.Name + strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(10000))
	return fmt.Sprintf("%x", sha1.Sum([]byte(random)))
}

func generateSecret(app *App) string {
	random := "openrasp_app" + app.Name + app.Id +
		strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(10000))
	sha256Data := sha256.Sum256([]byte(random))
	base64Data := base64.URLEncoding.EncodeToString(sha256Data[0:])
	return base64Data[0 : len(base64Data)-1]
}

func GetAllApp(page int, perpage int) (count int, result []App, err error) {
	count, err = mongo.FindAll(appCollectionName, nil, &result, perpage*(page-1), perpage, "name")
	if err == nil && result != nil {
		for _, app := range result {
			HandleApp(&app, false)
		}
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

func RegenerateSecret(appId string) (secret string, err error) {
	var app *App
	err = mongo.FindId(appCollectionName, appId, &app)
	if err == nil {
		return
	}
	newSecret := generateSecret(app)
	err = mongo.UpdateId(appCollectionName, appId, bson.M{"secret": newSecret})
	return
}

func HandleApp(app *App, isCreate bool) {
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
			app.EmailAlarmConf.Password = "************"
		}
		if app.DingAlarmConf.CorpSecret != "" {
			app.DingAlarmConf.CorpSecret = "************"
		}
	}
	if app.WhitelistConfig == nil {
		app.WhitelistConfig = make([]WhitelistConfigItem, 0)
	}
	if app.GeneralConfig == nil {
		app.GeneralConfig = make(map[string]interface{})
	}
}

func UpdateAppById(id string, doc interface{}) (app *App, err error) {
	err = mongo.UpdateId(appCollectionName, id, doc)
	if err != nil {
		return
	}
	return GetAppById(id)
}

func UpdateGeneralConfig(appId string, config map[string]interface{}) (*App, error) {
	return UpdateAppById(appId, bson.M{"general_config": config, "config_time": time.Now().UnixNano()})
}

func UpdateWhiteListConfig(appId string, config []WhitelistConfigItem) (app *App, err error) {
	return UpdateAppById(appId, bson.M{"whitelist_config": config, "config_time": time.Now().UnixNano()})
}

func RemoveAppById(id string) (err error) {
	return mongo.RemoveId(appCollectionName, id)
}

func GetAppCount() (count int, err error) {
	return mongo.Count(appCollectionName)
}

func PushAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) {
	if app != nil {
		if app.DingAlarmConf.Enable {
			PushDingAttackAlarm(app, total, alarms, isTest)
		}
		if app.EmailAlarmConf.Enable {
			PushEmailAttackAlarm(app, total, alarms, isTest)
		}
		if app.HttpAlarmConf.Enable {
			PushHttpAttackAlarm(app, total, alarms, isTest)
		}
	}
}

func PushEmailAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var emailConf = app.EmailAlarmConf
	if emailConf.UserName != "" && len(emailConf.RecvAddr) > 0 && emailConf.ServerAddr != "" {
		auth := smtp.PlainAuth("", emailConf.UserName, emailConf.Password, emailConf.ServerAddr)
		var subject string
		var msg string
		var body string
		var emailAddr = &mail.Address{}
		var head = make(map[string]string)
		hostName, err := os.Hostname()
		emailAddr.Address = emailConf.UserName
		if err == nil {
			emailAddr.Name = hostName
		} else {
			emailAddr.Name = "OpenRASP"
		}
		head["From"] = emailAddr.String()
		head["To"] = strings.Join(emailConf.RecvAddr, ",")
		head["Content-Type"] = "text/html; charset=UTF-8"
		if isTest {
			subject = "OpenRASP ALARM TEST"
			body = "OpenRASP test message from app: " + app.Name
		} else {
			if emailConf.Subject == "" {
				subject = "OpenRASP ALARM"
			} else {
				subject = emailConf.Subject
			}
			t, err := template.ParseFiles("views/email.tpl")
			if err != nil {
				beego.Error("failed to render email template: " + err.Error())
				return err
			}
			var alarmData = new(bytes.Buffer)
			err = t.Execute(alarmData, &emailTemplateParam{
				Total:        total - int64(len(alarms)),
				Alarms:       alarms,
				AppName:      app.Name,
				DetailedLink: beego.AppConfig.String("Domain") + "/#/events/" + app.Id,
			})
			if err != nil {
				beego.Error("failed to execute email template: " + err.Error())
				return err
			}
			body = alarmData.String()
		}
		head["Subject"] = subject
		for k, v := range head {
			msg += fmt.Sprintf("%s: %s\r\n", k, v)
		}
		msg += "\r\n" + body
		err = smtp.SendMail(emailConf.ServerAddr, auth, emailConf.UserName, emailConf.RecvAddr, []byte(msg))
		if err != nil {
			beego.Error("failed to push email alarms: " + err.Error())
			return err
		}
	}
	beego.Debug("succeed in pushing email alarm for app: " + app.Name)
	return nil
}

func PushHttpAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var httpConf = app.HttpAlarmConf
	if len(httpConf.RecvAddr) != 0 {
		body := make(map[string]interface{})
		body["app_id"] = app.Id
		if isTest {
			body["data"] = map[string]interface{}{"test": "test"}
		} else {
			body["data"] = alarms
		}
		for _, addr := range httpConf.RecvAddr {
			request := httplib.Post(addr)
			request.JSONBody(body)
			request.SetTimeout(10*time.Second, 10*time.Second)
			response, err := request.Response()
			if err != nil {
				beego.Error("failed to push http alarms to: " + addr + ", with error: " + err.Error())
				return err
			}
			if response.StatusCode > 299 || response.StatusCode < 200 {
				err := errors.New("failed to push http alarms to: " + addr + ", with status code: " +
					strconv.Itoa(response.StatusCode))
				beego.Error(err.Error())
				return err
			}
		}
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
			beego.Error(errMsg + ", with error: " + err.Error())
			return err
		}
		if response.StatusCode != 200 {
			err := errors.New(errMsg + ", with status code: " + strconv.Itoa(response.StatusCode))
			beego.Error(err.Error())
			return err
		}
		var result dingResponse
		err = request.ToJSON(&result)
		if err != nil {
			beego.Error(errMsg + ", with error: " + err.Error())
			return err
		}
		if result.ErrCode != 0 {
			err := errors.New(errMsg + ", with errmsg: " + result.ErrMsg)
			beego.Error(err.Error())
			return err
		}
		token := result.AccessToken
		body := make(map[string]interface{})
		dingText := ""
		if isTest {
			dingText = "OpenRASP test message from app: " + app.Name
		} else {
			dingText = "来自 OpenRAS 的报警\n共有 " + strconv.FormatInt(total, 10) + " 条报警信息来自 APP：" + app.Name + "，详细信息：" +
				beego.AppConfig.String("Domain") + "/#/events/" + app.Id
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
			beego.Error(errMsg + ", with error: " + err.Error())
			return err
		}
		if response.StatusCode != 200 {
			err := errors.New(errMsg + ", with status code: " + strconv.Itoa(response.StatusCode))
			beego.Error(err.Error())
			return err
		}
		err = request.ToJSON(&result)
		if err != nil {
			beego.Error(errMsg + ", with error: " + err.Error())
			return err
		}
		if result.ErrCode != 0 {
			err := errors.New(errMsg + ", with errmsg: " + result.ErrMsg)
			beego.Error(err.Error())
			return err
		}
	}
	beego.Debug("succeed in pushing ding ding alarm for app: " + app.Name + " ,with corp id: " + dingCong.CorpId)
	return nil
}
