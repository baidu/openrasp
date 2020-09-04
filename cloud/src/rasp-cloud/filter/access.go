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

package filter

import (
	"github.com/astaxie/beego/logs"
	"os"
	"rasp-cloud/conf"
	"rasp-cloud/tools"
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/context"
	"strconv"
	"time"
)

var (
	accessLogger *logs.BeeLogger
)

func init() {
	initAccessLogger()
	beego.InsertFilter("/*", beego.BeforeRouter, logAccess)
	beego.InsertFilter("/", beego.BeforeStatic, handleStatic)
}

func logAccess(ctx *context.Context) {
	var cont string
	cont += "[T]" + formatTime(time.Now().Unix(), "15:04:05") + " " + ctx.Input.Method() + " " +
		ctx.Input.Site() + ctx.Input.URI() + " - [I]" + ctx.Input.IP() + " | [U]" + ctx.Input.UserAgent()
	if ctx.Input.Referer() != "" {
		cont += "[F]" + ctx.Input.Referer()
	}
	if conf.AppConfig.RequestBodyEnable {
		body := ctx.Input.RequestBody
		cont += " - [B]" + string(body)
	}
	accessLogger.Info(cont)
}

func handleStatic(ctx *context.Context) {
	ctx.Output.Header("Cache-Control", "no-cache, no-store, max-age=0")
}

func formatTime(timestamp int64, format string) (times string) {
	tm := time.Unix(timestamp, 0)
	times = tm.Format(format)
	return
}

func initAccessLogger() {
	//var logPathSplit []string
	logFileName := "/access.log"
	maxSize := strconv.FormatInt(conf.AppConfig.LogMaxSize, 10)
	maxDays := strconv.Itoa(conf.AppConfig.LogMaxDays)
	logPath := conf.AppConfig.LogPath
	logAccessPath := conf.AppConfig.LogPath + "/access"
	// 判断后缀名称
	//if strings.HasSuffix(logPath, ".log") {
	//	logPathSplit = strings.Split(logPath, "/")
	//	logFileName = "/" + logPathSplit[len(logPathSplit) - 1]
	//	logPathSplitNoLogFileName := logPathSplit[:len(logPathSplit) - 1]
	//	logPath = strings.Join(logPathSplitNoLogFileName, "/")
	//}
	if isExists, _ := tools.PathExists(logPath); !isExists {
		err := os.MkdirAll(logPath, os.ModePerm)
		if err != nil {
			tools.Panic(tools.ErrCodeLogInitFailed, "failed to create " + logPath + " dir", err)
		}
	}
	accessLogger = logs.NewLogger()
	accessLogger.EnableFuncCallDepth(true)
	accessLogger.SetLogFuncCallDepth(4)
	logAccessPath += logFileName
	err := accessLogger.SetLogger(logs.AdapterFile,
		`{"filename":"`+logAccessPath+`","daily":true,"maxdays":`+maxDays+`,"perm":"0777","maxsize": `+maxSize+`}`)
	if err != nil {
		tools.Panic(tools.ErrCodeLogInitFailed, "failed to init access log", err)
	}
}
