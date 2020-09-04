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

package conf

import (
	"rasp-cloud/tools"
	"github.com/astaxie/beego"
	"strings"
)

const (
	StartTypeForeground = "panel"
	StartTypeAgent      = "agent"
	StartTypeReset      = "reset"
	StartTypeDefault    = "default"
	RestartOperation 	= "restart"
	StatusOperation 	= "status"
	StopOperation 		= "stop"
)

type RaspAppConfig struct {
	EsAddr                []string
	EsUser                string
	EsPwd                 string
	EsTTL                 int64
	KafkaAddr             string
	KafkaUser             string
	KafkaPwd              string
	KafkaEnable           bool
	KafkaTopic            string
	MongoDBAddr           []string
	MongoDBUser           string
	MongoDBPwd            string
	MongoDBName           string
	MongoDBPoolLimit      int
	MaxPlugins            int
	AlarmLogMode          string
	AlarmBufferSize       int
	AlarmCheckInterval    int64
	CookieLifeTime        int
	RegisterCallbackUrl   string
	RegisterCallbackToken string
	RequestBodyEnable     bool
	ErrorLogEnable        bool
	DebugModeEnable       bool
	Flag                  *Flag
	LogMaxSize            int64
	LogMaxDays            int
	LogPath               string
	OffLineInterval       int64
}

type Flag struct {
	StartType *string
	Password  *string
	Daemon    *bool
	Version   *bool
	Operation *string
	Upgrade   *string
}

var (
	AppConfig = &RaspAppConfig{}
)

func InitConfig(startFlag *Flag) {
	initConstConfig()
	path, err := tools.GetCurrentPath()
	if err != nil {
		beego.Warn(err)
		path = "/home/openrasp/"
	}
	AppConfig.Flag = startFlag
	AppConfig.EsAddr = initArrayConfig(strings.Split(beego.AppConfig.String("EsAddr"), ","))
	AppConfig.EsUser = beego.AppConfig.DefaultString("EsUser", "")
	AppConfig.EsPwd = beego.AppConfig.DefaultString("EsPwd", "")
	AppConfig.EsTTL = beego.AppConfig.DefaultInt64("EsTTL", 365)
	AppConfig.KafkaAddr = beego.AppConfig.String("KafkaAddr")
	AppConfig.KafkaUser = beego.AppConfig.DefaultString("KafkaUser", "")
	AppConfig.KafkaPwd = beego.AppConfig.DefaultString("KafkaPwd", "")
	AppConfig.KafkaEnable = beego.AppConfig.DefaultBool("KafkaEnable", false)
	AppConfig.KafkaTopic = beego.AppConfig.DefaultString("KafkaTopic", "")
	AppConfig.MongoDBAddr = initArrayConfig(strings.Split(beego.AppConfig.String("MongoDBAddr"), ","))
	AppConfig.MongoDBPoolLimit = beego.AppConfig.DefaultInt("MongoDBPoolLimit", 1024)
	AppConfig.MongoDBName = beego.AppConfig.DefaultString("MongoDBName", "openrasp")
	AppConfig.MongoDBUser = beego.AppConfig.DefaultString("MongoDBUser", "")
	AppConfig.MongoDBPwd = beego.AppConfig.DefaultString("MongoDBPwd", "")
	AppConfig.MaxPlugins = beego.AppConfig.DefaultInt("MaxPlugins", 30)
	AppConfig.AlarmLogMode = beego.AppConfig.DefaultString("AlarmLogMode", "file")
	AppConfig.AlarmBufferSize = beego.AppConfig.DefaultInt("AlarmBufferSize", 300)
	AppConfig.AlarmCheckInterval = beego.AppConfig.DefaultInt64("AlarmCheckInterval", 120)
	AppConfig.CookieLifeTime = beego.AppConfig.DefaultInt("CookieLifeTime", 7*24)
	AppConfig.RegisterCallbackUrl = beego.AppConfig.DefaultString("RegisterCallbackUrl", "")
	AppConfig.RegisterCallbackToken = beego.AppConfig.DefaultString("RegisterCallbackToken", "")
	AppConfig.RequestBodyEnable = beego.AppConfig.DefaultBool("RequestBodyEnable", false)
	AppConfig.ErrorLogEnable = beego.AppConfig.DefaultBool("ErrorLogEnable", false)
	AppConfig.LogMaxSize = beego.AppConfig.DefaultInt64("LogMaxSize", 104857600)
	AppConfig.LogMaxDays = beego.AppConfig.DefaultInt("LogMaxDays", 10)
	AppConfig.DebugModeEnable = beego.AppConfig.DefaultBool("DebugModeEnable", false)
	AppConfig.LogPath = beego.AppConfig.DefaultString("LogPath", path + "logs")
	AppConfig.OffLineInterval = beego.AppConfig.DefaultInt64("OffLineInterval", 180)
	ValidRaspConf(AppConfig)
}

func initArrayConfig(config []string) []string {
	for i, item := range config {
		config[i] = strings.TrimSpace(item)
	}
	return config
}

func initConstConfig() {
	beego.AppConfig.Set("gzipCompressLevel", "2")
	beego.AppConfig.Set("includedMethods", "GET;POST")
}

func ValidRaspConf(config *RaspAppConfig) {
	if len(config.EsAddr) == 0 {
		failLoadConfig("the 'EsAddr' config item in app.conf can not be empty")
	}
	if len(config.MongoDBAddr) == 0 {
		failLoadConfig("the 'MongoDBAddr' config item in app.conf can not be empty")
	}

	if config.MongoDBPoolLimit <= 0 {
		failLoadConfig("the 'poolLimit' config must be greater than 0")
	} else if config.MongoDBPoolLimit < 10 {
		beego.Warning("the value of 'poolLimit' config is less than 10, it will be set to 10")
		config.MongoDBPoolLimit = 10
	}

	if config.EsTTL <= 0 {
		failLoadConfig("the 'EsTTL' config must be greater than 0")
	}

	if config.MaxPlugins <= 0 {
		failLoadConfig("the 'MaxPlugins' config must be greater than 0")
	} else if config.MaxPlugins < 10 {
		beego.Warning("the value of 'MaxPlugins' config is less than 10, it will be set to 10")
		config.MaxPlugins = 10
	}

	if config.AlarmBufferSize <= 0 {
		failLoadConfig("the 'AlarmBufferSize' config must be greater than 0")
	} else if config.AlarmBufferSize < 100 {
		beego.Warning("the value of 'AlarmBufferSize' config is less than 100, it will be set to 100")
		config.AlarmBufferSize = 100
	}
	if config.AlarmCheckInterval <= 0 {
		failLoadConfig("the 'AlarmCheckInterval' config must be greater than 0")
	} else if config.AlarmCheckInterval < 10 {
		beego.Warning("the value of 'AlarmCheckInterval' config is less than 10, it will be set to 10")
		config.AlarmCheckInterval = 10
	}
	if config.CookieLifeTime <= 0 {
		failLoadConfig("the 'CookieLifeTime' config must be greater than 0")
	}
}

func failLoadConfig(msg string) {
	tools.Panic(tools.ErrCodeConfigInitFailed, msg, nil)
}
