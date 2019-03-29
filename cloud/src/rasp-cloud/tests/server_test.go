package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/models"
	"github.com/bouk/monkey"
	"errors"
	"gopkg.in/mgo.v2"
)

func getValidServerUrl() *models.ServerUrl {
	return &models.ServerUrl{
		PanelUrl:  "http://23.23.233.23:8086",
		AgentUrls: []string{"http://23.23.233.23:8086", "http://23.23.233.23:8087"},
	}
}

func TestPutServerUrl(t *testing.T) {
	Convey("Subject: Test Server Url Put Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(getValidServerUrl()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.PutServerUrl, func(*models.ServerUrl) (error) {
				return errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(getValidServerUrl()))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.PutServerUrl)
		})

		Convey("when the length of panel url is greater than 512", func() {
			data := getValidServerUrl()
			data.PanelUrl = "http://" + inits.GetLongString(513)
			r := inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the count of agent url is greater than 1024", func() {
			data := getValidServerUrl()
			data.AgentUrls = inits.GetLongStringArray(1025)
			r := inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of agent url is greater than 512", func() {
			data := getValidServerUrl()
			data.AgentUrls[0] = inits.GetLongString(513)
			r := inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when urls is invalid", func() {
			data := getValidServerUrl()
			data.PanelUrl = "a.b.c"
			r := inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)

			data = getValidServerUrl()
			data.AgentUrls[0] = "a.b.c"
			r = inits.GetResponse("POST", "/v1/api/server/url", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestGetServerUrl(t *testing.T) {
	Convey("Subject: Test Server Url Get Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/server/url/get", inits.GetJson(getValidServerUrl()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.GetServerUrl, func() (*models.ServerUrl, error) {
				return nil, mgo.ErrNotFound
			})
			r := inits.GetResponse("POST", "/v1/api/server/url/get", inits.GetJson(getValidServerUrl()))
			So(r.Status, ShouldEqual, 0)
			monkey.Unpatch(models.GetServerUrl)
		})

		Convey("when the mongo has errors", func() {
			monkey.Patch(models.GetServerUrl, func() (serverUrl *models.ServerUrl, err error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(models.GetServerUrl)
			r := inits.GetResponse("POST", "/v1/api/server/url/get", inits.GetJson(getValidServerUrl()))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}
