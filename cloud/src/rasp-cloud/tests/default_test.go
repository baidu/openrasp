package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	_ "rasp-cloud/tests/start"
	"rasp-cloud/tools"
	"golang.org/x/crypto/ssh/terminal"
	"github.com/bouk/monkey"
	"os/exec"
	"reflect"
	"rasp-cloud/environment"
	"rasp-cloud/conf"
	"os"
	"errors"
	"rasp-cloud/mongo"
	"rasp-cloud/models/logs"
	"time"
	"rasp-cloud/controllers"
	"bytes"
	"io/ioutil"
	"strconv"
	"syscall"
)

func Test404(t *testing.T) {
	Convey("Subject: Test 404\n", t, func() {
		r := inits.GetResponseRecorder("POST", "/v1/api/notexist", "")
		So(r.Code, ShouldEqual, 404)
	})
}

func TestError(t *testing.T) {
	Convey("Subject: Test 500\n", t, func() {
		c := &controllers.ErrorController{}
		monkey.PatchInstanceMethod(reflect.TypeOf(&controllers.BaseController{}), "ServeStatusCode",
			func(*controllers.BaseController, int, int, ...string) {
				return
			},
		)
		defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&controllers.BaseController{}), "ServeStatusCode")
		c.Error500()
		c.Error502()
		c.Error503()
	})

	Convey("Subject: Test 503\n", t, func() {
		r := inits.GetResponseRecorder("POST", "/v1/api/notexist", "")
		So(r.Code, ShouldEqual, 404)
	})

	Convey("Subject: Test 502\n", t, func() {
		r := inits.GetResponseRecorder("POST", "/v1/api/notexist", "")
		So(r.Code, ShouldEqual, 404)
	})
}

func TestPing(t *testing.T) {
	Convey("Subject: Test Ping\n", t, func() {
		r := inits.GetResponse("GET", "/v1/ping", "")
		So(r.Status, ShouldEqual, 0)
	})
}

func TestLogger(t *testing.T) {
	Convey("Subject: Test Logger\n", t, func() {
		monkey.PatchInstanceMethod(reflect.TypeOf(&tools.RaspFileLogWriter{}), "NeedRotate",
			func(*tools.RaspFileLogWriter, int, int) bool {
				return true
			})
		defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&tools.RaspFileLogWriter{}), "NeedRotate")
		logger := tools.NewFileWriter().(*tools.RaspFileLogWriter)
		logger.Filename = "logs/api/agent-cloud"
		logger.DoRotate(time.Now())
		logger.MaxLines = 0
		logger.MaxSize = 0
		logger.MaxFiles = 1
		logger.MaxDays = 1
		logger.Filename = "logs/api/agent-cloud"
		logger.DoRotate(time.Now())
		logger.Rotate = true
		err := logs.AddLogWithFile("attack-alarm", map[string]interface{}{})
		So(err, ShouldEqual, nil)
	})
}

func TestEnvironment(t *testing.T) {
	Convey("Subject: Test Environment Init\n", t, func() {
		Convey("test reset user password", func() {
			monkey.Patch(terminal.ReadPassword, func(fd int) ([]byte, error) {
				return []byte{}, nil
			})
			defer monkey.Unpatch(terminal.ReadPassword)
			environment.HandleReset(&conf.Flag{})
		})
		Convey("failed to read password from terminal", func() {
			monkey.Patch(terminal.ReadPassword, func(fd int) ([]byte, error) {
				return []byte{}, errors.New("")
			})
			defer monkey.Unpatch(terminal.ReadPassword)
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			environment.HandleReset(&conf.Flag{})
		})
		Convey("test reset user password when passwords do not match", func() {
			monkey.Patch(terminal.ReadPassword, func(fd int) ([]byte, error) {
				return []byte{}, nil
			})
			defer monkey.Unpatch(terminal.ReadPassword)
			monkey.Patch(bytes.Compare, func(a, b []byte) (int){
				return 1
			})
			defer monkey.Unpatch(bytes.Compare)
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			environment.HandleReset(&conf.Flag{})
		})
		Convey("test fork process", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			environment.HandleDaemon()
		})
		Convey("test fork process when start successfully", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(environment.CheckPIDAlreadyRunning, func(path string) (bool) {
				return false
			})
			defer monkey.Unpatch(environment.CheckPIDAlreadyRunning)
			environment.HandleDaemon()
		})
		Convey("test stop process", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(syscall.Kill, func(pid int, signum syscall.Signal) (error) {
				return nil
			})
			defer monkey.Unpatch(syscall.Kill)
			environment.HandleOperation("stop")
		})
		Convey("test stop process when atoi has errors", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(syscall.Kill, func(pid int, signum syscall.Signal) (error) {
				return nil
			})
			defer monkey.Unpatch(syscall.Kill)
			monkey.Patch(strconv.Atoi, func(s string) (int, error) {
				return 0, errors.New("")
			})
			defer monkey.Unpatch(strconv.Atoi)
			environment.HandleOperation("stop")
		})
		Convey("test stop process when process id is not exists", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(syscall.Kill, func(pid int, signum syscall.Signal) (error) {
				return nil
			})
			defer monkey.Unpatch(syscall.Kill)
			monkey.Patch(environment.CheckPIDAlreadyRunning, func(path string) (bool) {
				return false
			})
			defer monkey.Unpatch(environment.CheckPIDAlreadyRunning)
			environment.HandleOperation("stop")
		})
		Convey("test unknown process", func() {
			environment.HandleOperation("u")
		})
		Convey("test restart process", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(syscall.Kill, func(pid int, signum syscall.Signal) (error) {
				return nil
			})
			defer monkey.Unpatch(syscall.Kill)
			environment.HandleOperation("restart")
		})
		Convey("test restart process when atoi has errors", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(syscall.Kill, func(pid int, signum syscall.Signal) (error) {
				return nil
			})
			defer monkey.Unpatch(syscall.Kill)
			monkey.Patch(strconv.Atoi, func(s string) (int, error) {
				return 0, errors.New("")
			})
			defer monkey.Unpatch(strconv.Atoi)
			environment.HandleOperation("restart")
		})
		Convey("test restart process when process id is not exists", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			monkey.Patch(syscall.Kill, func(pid int, signum syscall.Signal) (error) {
				return nil
			})
			defer monkey.Unpatch(syscall.Kill)
			monkey.Patch(environment.CheckPIDAlreadyRunning, func(path string) (bool) {
				return false
			})
			defer monkey.Unpatch(environment.CheckPIDAlreadyRunning)
			environment.HandleOperation("restart")
		})
		Convey("test check pid already running when err is not nil", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start", func(cmd2 *exec.Cmd) (error) {
				return nil
			})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&exec.Cmd{}), "Start")
			monkey.Patch(os.Stat, func(name string) (os.FileInfo, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(os.Stat)
			monkey.Patch(os.Getwd, func() (dir string, err error) {
				return "/home/jdk", errors.New("")
			})
			defer monkey.Unpatch(os.Getwd)
			environment.CheckPIDAlreadyRunning("/home/jdk")
		})
		Convey("test MkdirAll occurs errors", func() {
			monkey.Patch(os.MkdirAll, func(path string, perm os.FileMode) (error) {
				return errors.New("")
			})
			defer monkey.Unpatch(os.MkdirAll)

			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			environment.CheckForkStatus(true)
		})
		Convey("test WriteFile occurs errors 2", func() {
			monkey.Patch(ioutil.WriteFile, func(filename string, data []byte, perm os.FileMode) (error){
				return errors.New("")
			})
			defer monkey.Unpatch(ioutil.WriteFile)
			monkey.Patch(os.Exit, func(fd int) {
				return
			})
			defer monkey.Unpatch(os.Exit)
			environment.CheckForkStatus(true)
		})
	})
}

func TestPanic(t *testing.T) {
	monkey.Patch(os.Exit, func(fd int) {
		return
	})
	defer monkey.Unpatch(os.Exit)
	Convey("Subject: Test App Config Init\n", t, func() {
		Convey("when ", func() {
			tools.Panic(10001, "", errors.New(""))
		})
	})
}

func TestAppConfigValidation(t *testing.T) {
	monkey.Patch(os.Exit, func(fd int) {
		return
	})
	defer monkey.Unpatch(os.Exit)
	Convey("Subject: Test App Config Init\n", t, func() {
		Convey("when the es addr is empty", func() {
			config := *conf.AppConfig
			config.EsAddr = []string{}
			conf.ValidRaspConf(&config)
		})

		Convey("when the mongodb addr is empty", func() {
			config := *conf.AppConfig
			config.MongoDBAddr = []string{}
			conf.ValidRaspConf(&config)
		})

		Convey("when the size of mongodb pool is less than 10", func() {
			config := *conf.AppConfig
			config.MongoDBPoolLimit = 9
			conf.ValidRaspConf(&config)
		})

		Convey("when the size of mongodb pool is 0", func() {
			config := *conf.AppConfig
			config.MongoDBPoolLimit = 0
			conf.ValidRaspConf(&config)
		})

		Convey("when max_plugins is 0", func() {
			config := *conf.AppConfig
			config.MaxPlugins = 0
			conf.ValidRaspConf(&config)
		})

		Convey("when max_plugins is less than 10", func() {
			config := *conf.AppConfig
			config.MaxPlugins = 9
			conf.ValidRaspConf(&config)
		})

		Convey("when the alarm buffer size is less than 100", func() {
			config := *conf.AppConfig
			config.AlarmBufferSize = 99
			conf.ValidRaspConf(&config)
		})

		Convey("when the alarm buffer size is 0", func() {
			config := *conf.AppConfig
			config.AlarmBufferSize = 0
			conf.ValidRaspConf(&config)
		})

		Convey("when the alarm check interval is less than 10", func() {
			config := *conf.AppConfig
			config.AlarmCheckInterval = 9
			conf.ValidRaspConf(&config)
		})

		Convey("when the alarm check interval is 0", func() {
			config := *conf.AppConfig
			config.AlarmCheckInterval = 0
			conf.ValidRaspConf(&config)
		})

		Convey("when the cookie life time is 0", func() {
			config := *conf.AppConfig
			config.CookieLifeTime = 0
			conf.ValidRaspConf(&config)
		})
	})
}

func TestMongoGenerateObjectId(t *testing.T) {
	Convey("Subject: Test App Config Init\n", t, func() {
		id := mongo.GenerateObjectId()
		So(id, ShouldNotEqual, "")
	})
}
