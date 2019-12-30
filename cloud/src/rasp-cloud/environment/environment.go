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
	"strconv"
	"strings"
	"time"
	"io/ioutil"
	"net"
	"path/filepath"
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/logs"
	"golang.org/x/crypto/ssh/terminal"
	"log"
	"os"
	"os/exec"
	"rasp-cloud/conf"
	"rasp-cloud/tools"
	"syscall"
)

type PIDFile struct {
	path string
}

var (
	UpdateMappingConfig map[string]interface{}
	StartBeego          = true
	Version             = "1.3"
	LogPath             = "logs/"
	PidFileName         = LogPath + "pid.file"
	OldPid              = ""
)

func init() {
	chdir()
	StartFlag := &conf.Flag{}
	StartFlag.StartType = flag.String("type", "", "use to provide different routers")
	StartFlag.Daemon = flag.Bool("d", false, "use to run as daemon process")
	StartFlag.Version = flag.Bool("version", false, "use to get version")
	StartFlag.Operation = flag.String("s", "", "send signal to a master process: stop, restart")
	StartFlag.Upgrade = flag.String("upgrade", "", "Execute upgrade job, e.g update ElasticSearch mapping")
	flag.Parse()
	if *StartFlag.Version {
		handleVersionFlag()
	}
	beego.Info("Version: " + Version)
	OldPid = readPIDFILE(PidFileName)
	if tools.BuildTime != "" {
		beego.Info("Build Time: " + tools.BuildTime)
	}
	if tools.CommitID != "" {
		beego.Info("Git Commit ID: " + tools.CommitID)
	}
	if *StartFlag.Operation != "" {
		HandleOperation(*StartFlag.Operation)
	}
	if *StartFlag.Upgrade != "" {
		StartBeego = false
		HandleUpgrade(*StartFlag.Upgrade)
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
	if *StartFlag.Operation != "" {
		CheckForkStatus(true)
	} else {
		CheckForkStatus(false)
	}
	beego.Info("===== startup type: " + *StartFlag.StartType + " =====")
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

func HandleOperation(operation string)  {
	switch operation {
	case conf.RestartOperation:
		restart()
	case conf.StopOperation:
		stop()
	default:
		log.Println("unknown operation!")
	}
}

func restart() {
	pid, err := strconv.Atoi(OldPid)
	if CheckPIDAlreadyRunning(PidFileName) {
		log.Println("restarting........")
		if err != nil {
			tools.Panic(tools.ErrCodeGetPidFailed, "failed to get pid", err)
		}
		err = syscall.Kill(pid, syscall.SIGHUP)
		if err != nil {
			log.Fatalln(err)
		}
		time.Sleep(5 * time.Second)
		log.Println("restart success!")
	} else {
		log.Printf("the process id:%s is not exists!", OldPid)
	}
	os.Exit(0)
}

func stop()  {
	pid, err := strconv.Atoi(OldPid)
	if CheckPIDAlreadyRunning(PidFileName) {
		log.Println("stopping........")
		if err != nil {
			tools.Panic(tools.ErrCodeGetPidFailed, "failed to get pid", err)
		}else {
			err = syscall.Kill(pid, syscall.SIGQUIT)
			if err != nil {
				log.Fatalln(err)
			}
			log.Println("stop ok!")
		}
	} else {
		log.Printf("the process id:%s is not exists!", OldPid)
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

func HandleUpgrade(flag string) {
	UpdateMappingConfig = make(map[string]interface{})
	switch flag {
	case "120to121":
		log.Println("Going to update ElasticSearch mapping")

		UpdateMappingConfig["attack-alarm-template"] = "120to121"
		UpdateMappingConfig["policy-alarm-template"] = "120to121"
		UpdateMappingConfig["error-alarm-template"] = "120to121"
	case "121to122":
		log.Println("Going to update 121to122")
	default:
		log.Println("Unknown upgrade job specified: " + flag)
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
	if CheckPIDAlreadyRunning(PidFileName) {
		RecoverPid(PidFileName, false)
		log.Fatal("fail to start! for details please check the log in 'logs/api/agent-cloud.log'")
	}else {
		port := beego.AppConfig.DefaultInt("httpport", 8080)
		res := CheckPort(port)
		if res == false {
			RecoverPid(PidFileName, false)
			log.Fatal("fail to start! for details please check the log in 'logs/api/agent-cloud.log'")
		}
		log.Println("start successfully, for details please check the log in 'logs/api/agent-cloud.log'")
	}
	os.Exit(0)
}

func CheckForkStatus(remove bool) {
	f, ret := newPIDFile(PidFileName, remove)
	if ret == false && f == nil{
		log.Fatalf("create %s error, for details please check the log in 'logs/api/agent-cloud.log'", PidFileName)
	}
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

func processExists(pid string) bool {
	if _, err := os.Stat(filepath.Join("/proc", pid)); err == nil {
		return true
	}
	return false
}

func checkPIDAlreadyExists(path string, remove bool) bool {
	pid := readPIDFILE(path)
	if processExists(pid) && pid != " "{
		log.Printf("the main process %s has already exist!", pid)
		return true
	}
	if remove {
		removePIDFile()
	}
	return false
}

func CheckPIDAlreadyRunning(path string) bool {
	pid := readPIDFILE(path)
	cwd, err := os.Getwd()
	if err != nil {
		log.Println("getwd error:", err)
	}
	ret := pidFileExists(filepath.Join(cwd, path))
	if ret == false {
		return false
	}

	if processExists(pid) && pid != ""{
		return true
	}
	return false
}

func pidFileExists(path string) bool {
	_, err := os.Stat(path)
	if err != nil {
		if os.IsExist(err) {
			return true
		}
		return false
	}
	return true
}

func readPIDFILE(path string) string {
	if pidByte, err := ioutil.ReadFile(path); err == nil {
		pid := strings.TrimSpace(string(pidByte))
		if pid != "" {
			return pid
		}
	}
	return " "
}

func newPIDFile(path string, remove bool) (*PIDFile, bool) {
	if ret := checkPIDAlreadyExists(path, remove); ret == false {
		beego.Info("start new pid file!")
	}

	if err := os.MkdirAll(filepath.Dir(path), os.FileMode(0755)); err != nil {
		log.Println("Mkdir error:", err)
		return nil, false
	}
	if err := ioutil.WriteFile(path, []byte(fmt.Sprintf("%d", os.Getpid())), 0644); err != nil {
		log.Println("WriteFile error:", err)
		return nil, false
	}
	return &PIDFile{path: path}, true
}

func removePIDFile() error {
	return os.Remove(PidFileName)
}

func CheckPort(port int) bool {
	tcpAddress, err := net.ResolveTCPAddr("tcp", ":" + strconv.Itoa(port))
	if err != nil {
		log.Fatal(err)
		return false
	}

	time.Sleep(5 * time.Second)
	listener, err := net.ListenTCP("tcp", tcpAddress)
	if err != nil {
		return true
	} else {
		listener.Close()
		return false
	}
	return true
}

func RecoverPid(path string, remove bool) (*PIDFile, bool) {
	time.Sleep(1 * time.Second)
	if ret := checkPIDAlreadyExists(path, remove); ret == false {
		beego.Info("start new pid file!")
	}

	if err := os.MkdirAll(filepath.Dir(path), os.FileMode(0755)); err != nil {
		log.Println("Mkdir error:", err)
		return nil, false
	}
	if err := ioutil.WriteFile(path, []byte(fmt.Sprintf("%s", OldPid)), 0644); err != nil {
		log.Println("WriteFile error:", err)
		return nil, false
	}
	return &PIDFile{path: path}, true
}