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

package conf

import (
	"rasp-cloud/tools"
	"github.com/astaxie/beego"
)

const (
	StartTypeForeground = "panel"
	StartTypeAgent      = "agent"
	StartTypeReset      = "reset"
	StartTypeDefault    = "default"
)

type RaspAppConfig struct {
	EsAddr             string
	EsUser             string
	EsPwd              string
	MongoDBAddr        string
	MongoDBUser        string
	MongoDBPwd         string
	MongoDBName        string
	MongoDBPoolLimit   int
	MaxPlugins         int
	AlarmLogMode       string
	AlarmBufferSize    int
	AlarmCheckInterval int64
	CookieLifeTime     int
	Flag               *Flag
}

type Flag struct {
	StartType *string
	Password  *string
	Daemon    *bool
	Version   *bool
}

var (
	AppConfig = &RaspAppConfig{}
)

func InitConfig(startFlag *Flag) {
	AppConfig.Flag = startFlag
	AppConfig.EsAddr = beego.AppConfig.String("EsAddr")
	if AppConfig.EsAddr == "" {
		failLoadConfig("the 'EsAddr' config item in app.conf can not be empty")
	}
	AppConfig.EsUser = beego.AppConfig.DefaultString("EsUser", "")
	AppConfig.EsPwd = beego.AppConfig.DefaultString("EsPwd", "")
	AppConfig.MongoDBAddr = beego.AppConfig.DefaultString("MongoDBAddr", "")
	if AppConfig.MongoDBAddr == "" {
		failLoadConfig("the 'MongoDBAddr' config item in app.conf can not be empty")
	}
	AppConfig.MongoDBPoolLimit = beego.AppConfig.DefaultInt("MongoDBPoolLimit", 1024)
	if AppConfig.MongoDBPoolLimit <= 0 {
		failLoadConfig("the 'poolLimit' config must be greater than 0")
	} else if AppConfig.MongoDBPoolLimit < 10 {
		beego.Warning("the value of 'poolLimit' config is less than 10, it will be set to 10")
		AppConfig.MongoDBPoolLimit = 10
	}
	AppConfig.MongoDBName = beego.AppConfig.DefaultString("MongoDBName", "openrasp")
	AppConfig.MongoDBUser = beego.AppConfig.DefaultString("MongoDBUser", "")
	AppConfig.MongoDBPwd = beego.AppConfig.DefaultString("MongoDBPwd", "")
	if value, err := beego.AppConfig.Int("MaxPlugins"); err != nil || value <= 0 {
		failLoadConfig("the 'AlarmBufferSize' config must be greater than 0")
	} else if value < 10 {
		beego.Warning("the value of 'MaxPlugins' config is less than 10, it will be set to 10")
		AppConfig.MaxPlugins = 10
	} else {
		AppConfig.MaxPlugins = value
	}
	AppConfig.AlarmLogMode = beego.AppConfig.DefaultString("AlarmLogMode", "file")
	AppConfig.AlarmBufferSize = beego.AppConfig.DefaultInt("AlarmBufferSize", 300)
	if AppConfig.AlarmBufferSize <= 0 {
		failLoadConfig("the 'AlarmBufferSize' config must be greater than 0")
	} else if AppConfig.AlarmBufferSize < 100 {
		beego.Warning("the value of 'AlarmBufferSize' config is less than 100, it will be set to 100")
		AppConfig.AlarmBufferSize = 100
	}
	AppConfig.AlarmCheckInterval = beego.AppConfig.DefaultInt64("AlarmCheckInterval", 120)
	if AppConfig.AlarmCheckInterval <= 0 {
		failLoadConfig("the 'AlarmCheckInterval' config must be greater than 0")
	} else if AppConfig.AlarmCheckInterval < 10 {
		beego.Warning("the value of 'AlarmCheckInterval' config is less than 10, it will be set to 10")
		AppConfig.AlarmCheckInterval = 10
	}
	AppConfig.CookieLifeTime = beego.AppConfig.DefaultInt("CookieLifeTime", 7*24)
	if AppConfig.CookieLifeTime <= 0 {
		failLoadConfig("the 'CookieLifeTime' config must be greater than 0")
	}
}

func failLoadConfig(msg string) {
	tools.Panic(tools.ErrCodeConfigInitFailed, msg, nil)
}
