package test

import (
	_ "rasp-cloud/tests/inits"
	_ "rasp-cloud/environment"
	_ "rasp-cloud/models"
	//_ "rasp-cloud/filter"
	_ "rasp-cloud/controllers"
	"testing"
	"github.com/astaxie/beego"
	"rasp-cloud/routers"
	"rasp-cloud/controllers"
	"rasp-cloud/conf"
	. "github.com/smartystreets/goconvey/convey"
)

func init() {
	routers.InitRouter()
	beego.ErrorController(&controllers.ErrorController{})
}

func TestAppConfig(t *testing.T) {
	Convey("Subject: Test Config Init\n", t, func() {
		So(*conf.AppConfig.Flag.StartType, ShouldEqual, conf.StartTypeDefault)
	})
}
