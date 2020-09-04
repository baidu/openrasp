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

package agent

import (
	"rasp-cloud/controllers"
	"net/http"
	"rasp-cloud/models"
	"io/ioutil"
	"net/mail"
	"os"
	"rasp-cloud/models/logs"
	"strings"
	"github.com/astaxie/beego"
	"bytes"
	"fmt"
	"rasp-cloud/tools"
	"html/template"
	"encoding/base64"
	"errors"
	"time"
)

type CrashController struct {
	controllers.BaseController
}

type crashTemplateParam struct {
	Version  string
	Language string
	Hostname string
	AppName  string
	Time     string
	Ip       string
}

// @router /report [post]
func (o *CrashController) Post() {
	appId := o.Ctx.Input.Header("X-OpenRASP-AppID")
	app, err := models.GetAppById(appId)
	//if !app.EmailAlarmConf.Enable {
	//	o.ServeWithEmptyData()
	//	return
	//}
	isCrashReportEnable := false
	if app.AttackTypeAlarmConf != nil {
		if _, ok := (*app.AttackTypeAlarmConf)["crash"]; !ok {
			isCrashReportEnable = true
		} else {
			for _, t := range (*app.AttackTypeAlarmConf)["crash"] {
				if t == "email" {
					isCrashReportEnable = true
				}
			}
		}
	} else {
		isCrashReportEnable = true
	}
	if !isCrashReportEnable {
		o.ServeWithEmptyData()
		return
	}
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app")
	}
	raspId := o.GetString("rasp_id")
	if raspId == "" {
		o.ServeError(http.StatusBadRequest, "rasp_id can not be empty")
	}
	rasp, err := models.GetRaspById(raspId)
	// send to es
	alarm := make(map[string]interface{})
	alarm["rasp_id"] = raspId
	alarm["app_id"] = appId
	alarm["plugin_name"] = ""
	alarm["plugin_version"] = ""
	alarm["rasp_home"] = ""
	alarm["host_type"] = ""
	alarm["register_ip"] = ""
	alarm["hostname"] = ""
	alarm["language_version"] = ""
	alarm["register_time"] = ""
	alarm["environ"] = make(map[string]string)
	if err != nil {
		hostname := o.GetString("hostname")
		language := o.GetString("language")
		rasp = &models.Rasp{
			HostName: hostname,
			Language: language,
		}
		alarm["hostname"] = hostname
		alarm["language"] = language
		alarm["version"] = ""
	} else {
		alarm["hostname"] = rasp.HostName
		alarm["language"] = rasp.Language
		alarm["version"] = rasp.Version
		alarm["plugin_name"] = rasp.PluginName
		alarm["plugin_version"] = rasp.PluginVersion
		alarm["rasp_home"] = rasp.RaspHome
		alarm["host_type"] = rasp.HostType
		alarm["register_ip"] = rasp.RegisterIp
		alarm["hostname"] = rasp.HostName
		alarm["language_version"] = rasp.LanguageVersion
		alarm["register_time"] = rasp.RegisterTime
		alarm["environ"] = rasp.Environ
	}
	crashLog, info, err := o.GetFile("crash_log")
	if err != nil {
		o.ServeError(http.StatusBadRequest, "parse uploadFile error", err)
	}
	if crashLog == nil {
		o.ServeError(http.StatusBadRequest, "must have the crash log parameter")
	}
	defer crashLog.Close()
	crashLogContent, err := ioutil.ReadAll(crashLog)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to read crash log", err)
	}

	// send to es
	//crashLogContent, err := ioutil.ReadFile("hs_err_pid20067-java.log")
	//if err != nil {
	//	o.ServeError(http.StatusBadRequest, "hehe", err)
	//}
	alarm["crash_log"] = string(crashLogContent)
	alarm["@timestamp"] = time.Now().UnixNano() / 1000000
	alarm["event_time"] = alarm["@timestamp"]
	sendEmail, err := logs.AddCrashAlarm(alarm)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "send crash alarm to es failed", err)
	}
	if app.EmailAlarmConf.Enable && sendEmail {
		err = sendCrashEmailAlarm(crashLogContent, info.Filename, app, rasp)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "send email failed", err)
		}
	}
	o.ServeWithEmptyData()
}

func sendCrashEmailAlarm(crashLogContent []byte, fileName string, app *models.App, rasp *models.Rasp) error {
	// 获取app的email配置用户名和密码
	emailConf, err := models.GetEmailConfByAppId(app.Id)
	if err != nil {
		return err
	}
	if len(emailConf.RecvAddr) > 0 && emailConf.ServerAddr != "" {
		var (
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
		subject := "OpenRASP agent crashed on " + rasp.HostName
		boundary := "OpenRASPCrashLogData"
		head := map[string]string{
			"From":              emailAddr.String(),
			"To":                strings.Join(emailConf.RecvAddr, ","),
			"Subject":           subject,
			"Content-Type":      "multipart/mixed; boundary=" + boundary,
			"X-Priority":        "3",
			"X-MSMail-Priority": "Normal",
			"X-Mailer":          "Microsoft Outlook Express 6.00.2900.2869",
			"X-MimeOLE":         "Produced By Microsoft MimeOLE V6.00.2900.2869",
			"ReturnReceipt":     "1",
		}
		t, err := template.ParseFiles("views/crash.tpl")
		if err != nil {
			beego.Error("failed to render email template: " + err.Error())
			return err
		}
		crashData := new(bytes.Buffer)
		templateData := &crashTemplateParam{
			Hostname: rasp.HostName,
			Language: rasp.Language,
			Time:     time.Now().Format("2006-01-02 15:04:05"),
			Version:  rasp.Version,
			AppName:  app.Name,
			Ip:       rasp.RegisterIp,
		}
		err = t.Execute(crashData, templateData)
		if err != nil {
			beego.Error("failed to execute email template: " + err.Error())
			return err
		}
		for k, v := range head {
			msg += fmt.Sprintf("%s: %s\r\n", k, v)
		}
		msg += "\r\n" + "--" + boundary + "\r\n"
		msg += "Content-Type: text/html; charset=utf-8\r\n\r\n"
		msg += crashData.String()
		msg += "\r\n\r\n\r\n"
		msg += "--" + boundary + "\r\n"
		msg += "Content-Type: text/plain\r\n"
		msg += "Content-Transfer-Encoding: base64\r\n"
		msg += "Content-Disposition: attachment; filename=\"" + fileName + "\""
		msg += "\r\n\r\n"

		b := make([]byte, base64.StdEncoding.EncodedLen(len(crashLogContent)))
		base64.StdEncoding.Encode(b, crashLogContent)
		data := bytes.NewBuffer(nil)
		data.WriteString(msg)
		data.Write(b)
		data.WriteString("\r\n")
		data.WriteString("--" + boundary + "--")
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
			return models.SendEmailWithTls(emailConf, auth, data.Bytes())
		}
		return models.SendNormalEmail(emailConf, auth, data.Bytes())
	} else {
		beego.Error(
			"failed to send email alarm: the email receiving address and email server address can not be empty", emailConf)
		return errors.New("the email receiving address and email server address can not be empty")
	}
}
