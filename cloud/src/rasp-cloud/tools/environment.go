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

package tools

import (
	"os"
	"github.com/astaxie/beego/logs"
	"github.com/astaxie/beego"
	"flag"
	"log"
	"os/exec"
	"fmt"
	"golang.org/x/crypto/ssh/terminal"
	"syscall"
	"bytes"
)

const (
	Version             = "1.0RC1"
	StartTypeForeground = "panel"
	StartTypeAgent      = "agent"
	StartTypeReset      = "reset"
	StartTypeDefault    = "default"
)

type Flag struct {
	StartType *string
	Password  *string
	Daemon    *bool
	Version   *bool
}

var (
	StartFlag = &Flag{}
)

func init() {
	StartFlag.StartType = flag.String("type", "", "use to provide different routers")
	StartFlag.Daemon = flag.Bool("d", false, "use to run as daemon process")
	StartFlag.Version = flag.Bool("version", false, "use to get version")
	flag.Parse()

	if *StartFlag.Version {
		fmt.Println(Version)
		os.Exit(0)
	}
	if *StartFlag.StartType == "reset" {
		fmt.Print("Enter new admin password: ")
		pwd1, err := terminal.ReadPassword(int(syscall.Stdin))
		if err != nil {

		}
		fmt.Print("\nRetype new admin password: ")
		pwd2, err := terminal.ReadPassword(int(syscall.Stdin))
		if err != nil {

		}
		if bytes.Compare(pwd1, pwd2) != 0 {
			fmt.Println("Sorry, passwords do not match")
			os.Exit(ErrCodeResetUserFailed)
		} else {
			pwd := string(pwd1)
			StartFlag.Password = &pwd
		}
	}
	if *StartFlag.Daemon {
		err := fork()
		if err != nil {
			Panic(ErrCodeInitChildProcessFailed, "failed to launch child process, error", err)
		}
		log.Println("start successfully, for details please check the log in 'logs/api/agent-cloud.log'")
		os.Exit(0)
	}
	initLogger()
	initEnvConf()
	if *StartFlag.StartType == "" {
		allType := StartTypeDefault
		StartFlag.StartType = &allType
	}
	beego.Info("===== start type: " + *StartFlag.StartType + " =====")
}

func fork() (err error) {
	path := os.Args[0]
	var args []string
	if len(os.Args) > 1 {
		for _, arg := range os.Args[1:] {
			if arg == "-d" {
				break
			}
			args = append(args, arg)
		}
	}
	log.Println("args: ", args)
	cmd := exec.Command(path, args...)
	err = cmd.Start()
	return
}

func initLogger() {
	currentPath, err := GetCurrentPath()
	if err != nil {
		Panic(ErrCodeLogInitFailed, "failed to get current path", err)
	}
	if isExists, _ := PathExists(currentPath + "/logs/api"); !isExists {
		err := os.MkdirAll(currentPath+"/logs/api", os.ModePerm)
		if err != nil {
			Panic(ErrCodeLogInitFailed, "failed to create logs/api dir", err)
		}
	}
	logs.SetLogFuncCall(true)
	logs.SetLogger(logs.AdapterFile,
		`{"filename":"logs/api/agent-cloud.log","daily":true,"maxdays":10,"perm":"0777"}`)

}

func initEnvConf() {
	if beego.BConfig.RunMode == "dev" {
		logs.SetLevel(beego.LevelDebug)
	} else {
		logs.SetLevel(beego.LevelInformational)
		beego.BConfig.EnableErrorsShow = false
		beego.BConfig.EnableErrorsRender = false
	}
}
