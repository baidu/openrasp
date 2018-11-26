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

package main

import (
	_ "rasp-cloud/models"
	_ "rasp-cloud/filter"
	_ "rasp-cloud/controllers"
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/logs"
	"os"
	"rasp-cloud/tools"
	"flag"
	"rasp-cloud/routers"
	"rasp-cloud/controllers"
)

func main() {
	beego.BConfig.Listen.Graceful = true
	handleCmdArgs()
	initLogger()
	beego.SetStaticPath("//", "dist")
	beego.ErrorController(&controllers.ErrorController{})
	beego.Run()
}

func handleCmdArgs() {
	startType := flag.String("type", "", "use to provide different routers")
	flag.Parse()
	routers.InitRouter(*startType)
}

func initLogger() {
	currentPath, err := tools.GetCurrentPath()
	if err != nil {
		tools.Panic("failed to get current path: " + err.Error())
	}
	if isExists, _ := tools.PathExists(currentPath + "/logs/api"); !isExists {
		err := os.MkdirAll(currentPath+"/logs/api", os.ModePerm)
		if err != nil {
			tools.Panic("failed to create logs/api dir")
		}
	}
	logs.SetLogFuncCall(true)
	logs.SetLogger(logs.AdapterFile,
		`{"filename":"logs/api/agent-cloud.log","daily":true,"maxdays":10,"perm":"0777"}`)
	if beego.BConfig.RunMode == "dev" {
		logs.SetLevel(beego.LevelDebug)
	} else {
		logs.SetLevel(beego.LevelInformational)
		beego.BConfig.EnableErrorsShow = false
		beego.BConfig.EnableErrorsRender = false

	}
}
