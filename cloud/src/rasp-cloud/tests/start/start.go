package start

import (
	_ "rasp-cloud/tests/inits"
	_ "rasp-cloud/environment"
	_ "rasp-cloud/models"
	//_ "rasp-cloud/filter"
	_ "rasp-cloud/controllers"
	"rasp-cloud/routers"
	"github.com/astaxie/beego"
	"rasp-cloud/controllers"
	"github.com/astaxie/beego/context"
	"rasp-cloud/models"
)

var TestApp = &models.App{
	Name:        "test_app",
	Language:    "java",
	Description: "test app",
}
var online = false
var TestRasp = &models.Rasp{
	Id:                "1234567890abc121321354545135135",
	Language:          "java",
	Version:           "1.0",
	HostName:          "ubuntu",
	RegisterIp:        "10.23.25.36",
	LanguageVersion:   "1.8",
	ServerType:        "tomcat",
	RaspHome:          "/home/work/tomcat8",
	PluginVersion:     "2019-03-10-10000",
	HeartbeatInterval: 180,
	LastHeartbeatTime: 1551781949000,
	RegisterTime:      1551781949000,
	Environ:           map[string]string{},
	Online:            &online,
}

func init() {
	routers.InitRouter()
	beego.ErrorController(&controllers.ErrorController{})
	beego.BConfig.RecoverFunc = func(*context.Context) {
		if err := recover(); err != nil {
		}
	}
	count, apps, _ := models.GetAllApp(1, 1, false)
	if count > 0 {
		TestApp = apps[0]
	} else {
		TestApp, _ = models.AddApp(TestApp)
	}
	TestRasp.AppId = TestApp.Id
	models.UpsertRaspById(TestRasp.Id, TestRasp)
}
