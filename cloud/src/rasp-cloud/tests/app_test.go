package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/models"
	"time"
)

var testApp = &models.App{
	Name:        "test_app",
	Language:    "java",
	Description: "test app",
}

func init() {
	models.AddApp(testApp)
}

func TestHandleApp(t *testing.T) {

	Convey("Subject: Test App Post Api\n", t, func() {
		r := inits.GetResponse(t, "POST", "/v1/api/app", `{}`)
		Convey("the app id must be exist", func() {
			So(r.Status, ShouldBeGreaterThan, 0)
		})
		r = inits.GetResponse(t, "POST", "/v1/api/app",
			`{"name":"test_app_"`+time.Now().String()+`,"language":"java","description":"test app"}`)
		Convey("get all app with paging", func() {
			So(r.Status, ShouldEqual, 0)
		})

	})

	Convey("Subject: Test App Get Api\n", t, func() {
		r := inits.GetResponse(t, "POST", "/v1/api/app/get", `{"id":"not exist app id"}`)
		Convey("the app id must be exist", func() {
			So(r.Status, ShouldBeGreaterThan, 0)
		})
		r = inits.GetResponse(t, "POST", "/v1/api/app/get", `{"page":1,"perpage":10}`)
		Convey("get all app with paging", func() {
			So(r.Status, ShouldEqual, 0)
		})
		r = inits.GetResponse(t, "POST", "/v1/api/app/get", `{"page":-1,"perpage":10}`)
		Convey("page param must be greater than 0", func() {
			So(r.Status, ShouldBeGreaterThan, 0)
		})
		r = inits.GetResponse(t, "POST", "/v1/api/app/get", `{"page":-1,"perpage":-10}`)
		Convey("perpage param must be greater than 0", func() {
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})

}
func TestConfigApp(t *testing.T) {

}

func TestConfigAlarm(t *testing.T) {

}
func TestTestEmail(t *testing.T) {

}

func TestTestDing(t *testing.T) {

}

func TestTestHttp(t *testing.T) {

}
func TestPushAlarm(t *testing.T) {
}

func TestDeleteApp(t *testing.T) {

}

func TestAlarm(t *testing.T) {

}

func TestConfigWhiteList(t *testing.T) {

}

func TestConfigGenerate(t *testing.T) {

}

func TestSelectPlugin(t *testing.T) {

}

func TestGetSelectedPlugin(t *testing.T) {

}
