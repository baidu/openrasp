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
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/logs"
	"golang.org/x/crypto/ssh/terminal"
	"io/ioutil"
	"log"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"rasp-cloud/conf"
	"rasp-cloud/tools"
	"strconv"
	"strings"
	"syscall"
	"time"
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
	OldPid = readPIDFILE(PidFileName)
	StartFlag := &conf.Flag{}
	StartFlag.StartType = flag.String("type", "", "Specify startup type: panel/agent, or both if none provided")
	StartFlag.Daemon = flag.Bool("d", false, "Fork to background")
	StartFlag.Version = flag.Bool("version", false, "Show version string")
	StartFlag.Operation = flag.String("s", "", "Send signal to master process: stop/restart")
	StartFlag.Upgrade = flag.String("upgrade", "", "Execute upgrade job, e.g update ElasticSearch mapping")
	flag.Parse()
	if *StartFlag.Version {
		handleVersionFlag()
	}
	beego.Info("Version: " + Version)
	if *StartFlag.Operation != "" {
		HandleOperation(*StartFlag.Operation)
	}
	if tools.BuildTime != "" {
		beego.Info("Build Time: " + tools.BuildTime)
	}
	if tools.CommitID != "" {
		beego.Info("Git Commit ID: " + tools.CommitID)
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
	beego.Info("===== Startup Type: " + *StartFlag.StartType + " =====")
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
	case conf.StatusOperation:
		status()
	default:
		log.Println("unknown operation!")
	}
}

func restart() {
	pid, err := strconv.Atoi(OldPid)
	if CheckPIDAlreadyRunning(PidFileName) {
		log.Println("Restarting........")
		if err != nil {
			tools.Panic(tools.ErrCodeGetPidFailed, "Failed to get pid", err)
		}
		err = syscall.Kill(pid, syscall.SIGHUP)
		if err != nil {
			log.Fatalln(err)
		}
		restartCnt := 0
		for {
			exist, err := processExists(OldPid)
			if err != nil {
				log.Fatalln(err)
			}
			if !exist {
				break;
			}
			restartCnt += 1
			time.Sleep(1 * time.Second)
			if restartCnt == 10 {
				log.Println("this operation may spend about a few minutes")
			}
			if restartCnt >= 120 {
				log.Fatalln("Restart timeout! Probably the process has been restarted immediately")
			}
		}
		log.Println("Restart success!")
	} else {
		log.Printf("The process id:%s is not exists or not a rasp process!", OldPid)
		os.Exit(-1)
	}
	os.Exit(0)
}

func stop()  {
	pid, err := strconv.Atoi(OldPid)
	if CheckPIDAlreadyRunning(PidFileName) {
		log.Println("Stopping........")
		if err != nil {
			tools.Panic(tools.ErrCodeGetPidFailed, "Failed to get pid", err)
		} else {
			err = syscall.Kill(pid, syscall.SIGQUIT)
			if err != nil {
				log.Fatalln(err)
			}
			log.Println("Stop ok!")
		}
	} else {
		log.Printf("The process id:%s is not exists!", OldPid)
	}
	os.Exit(0)
}

func status() {
	_, err := strconv.Atoi(OldPid)
	if CheckPIDAlreadyRunning(PidFileName) {
		if err != nil {
			tools.Panic(tools.ErrCodeGetPidFailed, "failed to get pid", err)
		}
		log.Printf("The rasp-cloud is running!")
		os.Exit(0)
	} else {
		log.Printf("The rasp-cloud is dead!")
		os.Exit(-1)
	}
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
	} else {
		var cnt int
		port := beego.AppConfig.DefaultInt("httpport", 8080)
		for cnt = 0; cnt < 30; cnt++ {
			res := CheckPort(port)
			if res {
				break
			}
			time.Sleep(1 * time.Second)
		}
		if cnt == 29 {
			log.Fatal("start timeout! for details please check the log in 'logs/api/agent-cloud.log'")
		} else {
			log.Println("start successfully, for details please check the log in 'logs/api/agent-cloud.log'")
		}
	}
	os.Exit(0)
}

func CheckForkStatus(remove bool) {
	f, ret := newPIDFile(PidFileName, remove)
	if ret == false && f == nil {
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
	maxSize := strconv.FormatInt(conf.AppConfig.LogMaxSize, 10)
	maxDays := strconv.Itoa(conf.AppConfig.LogMaxDays)
	if isExists, _ := tools.PathExists(logPath); !isExists {
		err := os.MkdirAll(logPath, os.ModePerm)
		if err != nil {
			tools.Panic(tools.ErrCodeLogInitFailed, "failed to create logs/api dir", err)
		}
	}
	logs.SetLogFuncCall(true)
	logs.SetLogger(logs.AdapterFile,
		`{"filename":"`+logPath+`/agent-cloud.log","daily":true,"maxdays":`+maxDays+`,"perm":"0777","maxsize": `+maxSize+`}`)

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

func processExists(pid string) (bool, error) {
	if _, err := os.Stat(filepath.Join("/proc", pid)); err == nil {
		port := beego.AppConfig.DefaultInt("httpport", 8080)
		lsof := exec.Command("/bin/bash", "-c", "lsof -i tcp:" + strconv.Itoa(port))
		out, _ := lsof.Output()
		if strings.Index(string(out), "rasp-") != -1 {
			return true, nil
		} else {
			return false, nil
		}
	}
	return false, nil
}

func checkPIDAlreadyExists(path string, remove bool) bool {
	//pid := readPIDFILE(path)
	if res, err := processExists(OldPid); res && err == nil && OldPid != " "{
		log.Printf("the main process %s has already exist!", OldPid)
		return true
	}
	if remove {
		removePIDFile()
	}
	return false
}

func CheckPIDAlreadyRunning(path string) bool {
	//pid := readPIDFILE(path)
	cwd, err := os.Getwd()
	if err != nil {
		log.Println("getwd error:", err)
	}
	ret := pidFileExists(filepath.Join(cwd, path))
	if ret == false {
		return false
	}

	if res, err := processExists(OldPid); res && err == nil && OldPid != " "{
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
	tcpAddress, err := net.ResolveTCPAddr("tcp", ":"+strconv.Itoa(port))
	if err != nil {
		log.Fatal(err)
		return false
	}

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