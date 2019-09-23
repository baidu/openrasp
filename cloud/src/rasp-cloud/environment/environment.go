//Copyright 2017-2019 Baidu Inc.
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

package environment

import (
	"bytes"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"rasp-cloud/conf"
	"rasp-cloud/tools"
	"syscall"
	"sync"
	"strconv"
	"strings"
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/logs"
	"golang.org/x/crypto/ssh/terminal"
	"time"
	"io/ioutil"
)

var (
	Version = "1.3"
	LogPath = "logs/"
	EsFileName = LogPath + "es.pid"
	MongoFileName = LogPath + "mongo.pid"
	MinMongoVersion = "3.6.0"
	MinEsVersion  = "5.6.0"
	MaxEsVersion  = "7.0.0"
	lock sync.Mutex
)

func init() {
	chdir()
	StartFlag := &conf.Flag{}
	StartFlag.StartType = flag.String("type", "", "use to provide different routers")
	StartFlag.Daemon = flag.Bool("d", false, "use to run as daemon process")
	StartFlag.Version = flag.Bool("version", false, "use to get version")
	StartFlag.Operation = flag.String("s", "", "send signal to a master process: stop, restart")
	flag.Parse()
	if *StartFlag.Version {
		handleVersionFlag()
	}
	beego.Info("Version: " + Version)
	if tools.BuildTime != "" {
		beego.Info("Build Time: " + tools.BuildTime)
	}
	if tools.CommitID != "" {
		beego.Info("Git Commit ID: " + tools.CommitID)
	}
	if *StartFlag.Operation != "" {
		parseOperation(*StartFlag.Operation)
	}
	if *StartFlag.StartType == conf.StartTypeReset {
		HandleReset(StartFlag)
	}
	if *StartFlag.Daemon {
		HandleDaemon()
	}
	initLogger()
	initEnvConf()
	if *StartFlag.StartType == "" {
		allType := conf.StartTypeDefault
		StartFlag.StartType = &allType
	}
	conf.InitConfig(StartFlag)
	beego.Info("===== start type: " + *StartFlag.StartType + " =====")
}

func handleVersionFlag() {
	fmt.Println("Version:       " + Version)
	if tools.BuildTime != "" {
		fmt.Println("Build Time:    " + tools.BuildTime)
	}
	if tools.CommitID != "" {
		fmt.Println("Git Commit ID: " + tools.CommitID)
	}
	os.Exit(0)
}

func parseOperation(operation string)  {
	switch operation {
	case "restart":
		restart()
	case "stop":
		stop()
	default:
		log.Println("unknown operation!")
	}
}

func restart() {
	lock.Lock()
	defer lock.Unlock()
	port := beego.AppConfig.DefaultInt("httpport", 8080)
	pid := readNetstat(port,  "restart")
	log.Println("restarting........")
	c := "kill -HUP " + pid
	cmd := exec.Command("/bin/sh", "-c", c)
	err := cmd.Start()
	if err == nil {
		log.Println("restart ok!")
	}
	os.Exit(0)
}

func stop()  {
	port := beego.AppConfig.DefaultInt("httpport", 8080)
	pid := readNetstat(port,  "stop")
	log.Println("stopping........")
	c := "kill -QUIT " + pid
	cmd := exec.Command("/bin/sh", "-c", c)
	err := cmd.Start()
	if err == nil {
		log.Println("stop success!")
	}
	os.Exit(0)
}

func chdir() {
	path, err := tools.GetCurrentPath()
	if err != nil {
		tools.Panic(tools.ErrCodeChDirFailed, "failed to get current dir", err)
	}
	fmt.Println(path)
	err = os.Chdir(path)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to change dir to "+path, err)
	}
}

func HandleReset(startFlag *conf.Flag) {
	fmt.Print("Enter new admin password: ")
	pwd1, err := terminal.ReadPassword(int(syscall.Stdin))
	fmt.Println()
	if err != nil {
		fmt.Println("failed to read password from terminal: " + err.Error())
		os.Exit(tools.ErrCodeResetUserFailed)
	}
	fmt.Print("Retype new admin password: ")
	pwd2, err := terminal.ReadPassword(int(syscall.Stdin))
	fmt.Println()
	if err != nil {
		fmt.Println("failed to read password from terminal: " + err.Error())
		os.Exit(tools.ErrCodeResetUserFailed)
	}
	if bytes.Compare(pwd1, pwd2) != 0 {
		fmt.Println("Sorry, passwords do not match")
		os.Exit(tools.ErrCodeResetUserFailed)
	} else {
		pwd := string(pwd1)
		startFlag.Password = &pwd
	}
}

func HandleDaemon() {
	err := fork()
	if err != nil {
		tools.Panic(tools.ErrCodeInitChildProcessFailed, "failed to launch child process, error", err)
	}
	checkForkStatus()
	log.Println("start successfully, for details please check the log in 'logs/api/agent-cloud.log'")
	os.Exit(0)
}

func checkForkStatus() {
	port := beego.AppConfig.DefaultInt("httpport", 8080)
	pid := readNetstat(port, "")
	log.Println("current_pid: ", pid)
	readEsFile(EsFileName, "version")
	readMongoFile(MongoFileName, "version")
	//initPidLogger(pid)
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
	log.Println("args:", args)
	cmd := exec.Command(path, args...)
	err = cmd.Start()
	return
}

func initLogger() {
	logPath := "logs/api"
	if isExists, _ := tools.PathExists(logPath); !isExists {
		err := os.MkdirAll(logPath, os.ModePerm)
		if err != nil {
			tools.Panic(tools.ErrCodeLogInitFailed, "failed to create logs/api dir", err)
		}
	}
	logs.SetLogFuncCall(true)
	logs.SetLogger(logs.AdapterFile,
		`{"filename":"`+logPath+`/agent-cloud.log","daily":true,"maxdays":10,"perm":"0777"}`)

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

//func initPidLogger(pid string) {
//	logFile, err := os.Create(pidFileName)
//	defer logFile.Close()
//	if err != nil {
//		log.Fatalln("open rasp-cloud.pid error")
//	}
//	pidLog := log.New(logFile, "[Info]", log.Llongfile)
//	pidLog.Printf("pid:%s", pid)
//}

func readEsFile(fileName string, keyword string) {
	contentBytes, err := ioutil.ReadFile(fileName)
	if err != nil{
		tools.Panic(tools.ErrCodeReadPidFileFailed, "fail to read es file", err)
	}
	logContent := string(contentBytes[:])
	if strings.Contains(logContent, keyword + ":") {
		startIndex := strings.Index(logContent, keyword+":")
		length := len(logContent)
		version := logContent[startIndex + len(keyword) + 1 : length]
		if version != "" {
			if strings.Compare(version, MinEsVersion) < 0 {
				log.Println("unable to support the ElasticSearch with a version lower than "+
					MinEsVersion+ ","+ " the current version is "+ version)
				os.Exit(-1)
			}
			if strings.Compare(version, MaxEsVersion) >= 0 {
				log.Println("unable to support the ElasticSearch with a version greater than or equal to "+
					MaxEsVersion+ ","+ " the current version is "+ version)
				os.Exit(-1)
			}
		}else {
			log.Println("fail to start process, for details please check the log in 'logs/api/agent-cloud.log'")
			os.Exit(-1)
		}
	}
}

func readMongoFile(fileName string, keyword string) {
	contentBytes, err := ioutil.ReadFile(fileName)
	if err != nil{
		tools.Panic(tools.ErrCodeReadPidFileFailed, "fail to read mongo.pid", err)
	}
	logContent := string(contentBytes[:])
	if strings.Contains(logContent, keyword + ":") {
		startIndex := strings.Index(logContent, keyword+":")
		length := len(logContent)
		version := logContent[startIndex + len(keyword) + 1 : length]
		if version != "" {
			if strings.Compare(version, MinMongoVersion) < 0 {
				log.Println("unable to support the MongoDB with a version lower than "+
					MinMongoVersion+ ","+ " the current version is "+ version)
			}
		}else {
			log.Println("fail to start process, for details please check the log in 'logs/api/agent-cloud.log'")
			os.Exit(-1)
		}
	}
}

func readNetstat(port int, operation string) (pid string){
	portStr := strconv.Itoa(port)
	if portStr != "" {
		c := "netstat -tunpl|grep " + portStr
		cmd := exec.Command("/bin/sh", "-c", c)
		out, _ := cmd.Output()
		idx := strings.Index(string(out),"rasp-cloud")
		if idx == -1 || operation != "" {
			if operation != "stop" {
				time.Sleep(5 * time.Second)
			}
			cmd := exec.Command("/bin/sh", "-c", c)
			out, _ = cmd.Output()
			if len(string(out)) != 0 {
				idx = strings.Index(string(out),"rasp-cloud")
				firstSplit := strings.Split(string(out),"/./")[0]
				secondSplit := strings.Split(firstSplit, " ")
				pid := secondSplit[len(secondSplit) - 1]
				if idx != -1{
					log.Println("httpport status ok!")
					return pid
				}
			}else {
				log.Println("fail to start process, for details please check the log in 'logs/api/agent-cloud.log'")
				os.Exit(-1)
			}
		}else {
			log.Println("fail to start process, for details please check the log in 'logs/api/agent-cloud.log'")
			os.Exit(-1)
		}
	}
	return
}