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
)

type App struct {
	Id               string                 `json:"id" bson:"_id"`
	Name             string                 `json:"name"  bson:"name"`
	Description      string                 `json:"description"  bson:"description"`
	ConfigTime       int64                  `json:"config_time"  bson:"config_time"`
	RaspConfig       map[string]interface{} `json:"rasp_config"  bson:"rasp_config"`
	SelectedPluginId string                 `json:"selected_plugin_id" bson:"selected_plugin_id"`
	EmailAlarmConf   *EmailAlarmConf        `json:"email_alarm_conf" bson:"email_alarm_conf"`
	DingAlarmConf    *DingAlarmConf         `json:"ding_alarm_conf" bson:"ding_alarm_conf"`
	HttpAlarmConf    *HttpAlarmConf         `json:"http_alarm_conf" bson:"http_alarm_conf"`
}

type EmailAlarmConf struct {
	EmailEnable     bool     `json:"email_enable" bson:"email_enable"`
	EmailServerAddr string   `json:"email_server_addr" bson:"email_server_addr"`
	EmailFromAddr   string   `json:"email_from_addr" bson:"email_from_addr"`
	EmailPassword   string   `json:"email_password" bson:"email_password"`
	EmailSubject    string   `json:"email_subject" bson:"email_subject"`
	EmailRecvAddr   []string `json:"email_recv_addr" bson:"email_recv_addr"`
}

type DingAlarmConf struct {
	DingEnable     bool     `json:"ding_enable" bson:"ding_enable"`
	DingAgentId    string   `json:"ding_agent_id" bson:"ding_agent_id"`
	DingCorpId     string   `json:"ding_corp_id" bson:"ding_corp_id"`
	DingCorpSecret string   `json:"ding_corp_secret" bson:"ding_corp_secret"`
	DingRecvUser   []string `json:"ding_recv_user" bson:"ding_recv_user"`
	DingRecvPart   []string `json:"ding_recv_part" bson:"ding_recv_part"`
}

type HttpAlarmConf struct {
	HttpEnable   bool     `json:"http_enable" bson:"http_enable"`
	HttpRecvAddr []string `json:"http_recv_addr" bson:"http_recv_addr"`
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
	lastAlarmTime = time.Now()
)

func init() {
	count, err := mongo.Count(appCollectionName)
	if err != nil {
		tools.Panic("failed to get app collection count")
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
			tools.Panic("failed to create index for app collection")
		}
	}
	alarmDuration := beego.AppConfig.DefaultInt64("AlarmDuration", 120)
	if alarmDuration <= 0 {
		tools.Panic("the 'AlarmDuration' config must be greater than 0")
	}
	go startAlarmTicker(time.Second * time.Duration(alarmDuration))
}

func startAlarmTicker(duration time.Duration) {
	ticker := time.NewTicker(duration)
	for {
		select {
		case <-ticker.C:
			handleAlarm()
		}
	}
}

func handleAlarm() {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to handle alarm: ", r)
		}
	}()
	var apps []App
	_, err := mongo.FindAllWithSelect(appCollectionName, nil, &apps, bson.M{"plugin": -1}, 0, 0)
	if err != nil {
		beego.Error("failed to get apps from mongodb for the alarm: " + err.Error())
		return
	}
	now := time.Now()
	for _, app := range apps {
		total, result, err := logs.SearchLogs(lastAlarmTime.Unix()*1000, now.Unix()*1000, nil, "event_time",
			1, 3, false, AliasReportIndexName+"-"+app.Id)
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

func AddApp(app *App) (result *App, err error) {
	if app.Id == "" {
		app.Id = generateAppId(app)
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
	err = mongo.UpsertId(appCollectionName, app.Id, app)
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

func GetAllApp(page int, perpage int) (count int, result []App, err error) {
	count, err = mongo.FindAllWithSelect(appCollectionName, nil, &result, bson.M{"plugin.content": -1}, perpage*(page-1), perpage)
	return
}

func GetAppByName(name string) (app *App, err error) {
	err = mongo.FindOne(appCollectionName, bson.M{"name": name}, &app)
	return
}

func GetAppById(id string) (app *App, err error) {
	err = mongo.FindOne(appCollectionName, bson.M{"_id": id}, &app)
	return
}

func UpdateAppById(id string, app *App) (err error) {
	return mongo.UpdateId(appCollectionName, id, app)
}

func RemoveAppById(id string) (err error) {
	return mongo.RemoveId(appCollectionName, id)
}

func PushAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) {
	if app != nil {
		if app.DingAlarmConf != nil && app.DingAlarmConf.DingEnable {
			PushDingAttackAlarm(app, total, alarms, isTest)
		}
		if app.EmailAlarmConf != nil && app.EmailAlarmConf.EmailEnable {
			PushEmailAttackAlarm(app, total, alarms, isTest)
		}
		if app.HttpAlarmConf != nil && app.HttpAlarmConf.HttpEnable {
			PushHttpAttackAlarm(app, total, alarms, isTest)
		}
	}
}

func PushEmailAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var emailConf = app.EmailAlarmConf
	if emailConf.EmailFromAddr != "" && len(emailConf.EmailRecvAddr) > 0 && emailConf.EmailServerAddr != "" {
		auth := smtp.PlainAuth("", emailConf.EmailFromAddr, emailConf.EmailPassword, emailConf.EmailServerAddr)
		var subject string
		var msg string
		var body string
		var emailAddr = &mail.Address{}
		var head = make(map[string]string)
		hostName, err := os.Hostname()
		emailAddr.Address = emailConf.EmailFromAddr
		if err == nil {
			emailAddr.Name = hostName
		} else {
			emailAddr.Name = "OpenRASP"
		}
		head["From"] = emailAddr.String()
		head["To"] = strings.Join(emailConf.EmailRecvAddr, ",")
		head["Content-Type"] = "text/html; charset=UTF-8"
		if isTest {
			subject = "OpenRASP ALARM TEST"
			body = "OpenRASP test message from app: " + app.Name
		} else {
			if emailConf.EmailSubject == "" {
				subject = "OpenRASP ALARM"
			} else {
				subject = emailConf.EmailSubject
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
				DetailedLink: beego.AppConfig.String("Domain") + "/logs/search?page=1&perpage=15&app_id=" + app.Id,
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
		fmt.Println(msg)
		err = smtp.SendMail(emailConf.EmailServerAddr, auth, emailConf.EmailFromAddr, emailConf.EmailRecvAddr, []byte(msg))
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
	if len(httpConf.HttpRecvAddr) != 0 {
		body := make(map[string]interface{})
		body["app_id"] = app.Id
		if isTest {
			body["data"] = map[string]interface{}{"test": "test"}
		} else {
			body["data"] = alarms
		}
		for _, addr := range httpConf.HttpRecvAddr {
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
		fmt.Sprintf("%v", httpConf.HttpRecvAddr))
	return nil
}

func PushDingAttackAlarm(app *App, total int64, alarms []map[string]interface{}, isTest bool) error {
	var dingCong = app.DingAlarmConf
	if dingCong.DingCorpId != "" && dingCong.DingCorpSecret != "" && dingCong.DingAgentId != "" &&
		!(len(dingCong.DingRecvPart) == 0 && len(dingCong.DingRecvUser) == 0) {

		request := httplib.Get("https://oapi.dingtalk.com/gettoken")
		request.SetTimeout(10*time.Second, 10*time.Second)
		request.Param("corpid", dingCong.DingCorpId)
		request.Param("corpsecret", dingCong.DingCorpSecret)
		response, err := request.Response()
		errMsg := "failed to get ding ding token with corp id: " + dingCong.DingCorpId
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
				beego.AppConfig.String("Domain") + "/logs/search?page=1&perpage=15&app_id=" + app.Id
		}
		if len(dingCong.DingRecvUser) > 0 {
			body["touser"] = strings.Join(dingCong.DingRecvUser, "|")
		}
		if len(dingCong.DingRecvPart) > 0 {
			body["toparty"] = strings.Join(dingCong.DingRecvPart, "|")
		}
		body["agentid"] = dingCong.DingAgentId
		body["msgtype"] = "text"
		body["text"] = map[string]string{"content": dingText}
		request = httplib.Post("https://oapi.dingtalk.com/message/send?access_token=" + token)
		request.JSONBody(body)
		request.SetTimeout(10*time.Second, 10*time.Second)
		response, err = request.Response()
		errMsg = "failed to push ding ding alarms with corp id: " + dingCong.DingCorpId
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
	beego.Debug("succeed in pushing ding ding alarm for app: " + app.Name + " ,with corp id: " + dingCong.DingCorpId)
	return nil
}
