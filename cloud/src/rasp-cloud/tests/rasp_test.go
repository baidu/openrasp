package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	"github.com/bouk/monkey"
	"reflect"
	"github.com/astaxie/beego/context"
	"rasp-cloud/tests/start"
)

func TestRaspRegister(t *testing.T) {
	Convey("Subject: Test Rasp Register Api\n", t, func() {
		monkey.PatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header",
			func(input *context.BeegoInput, key string) string {
				return start.TestApp.Id
			},
		)
		defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header")

		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(start.TestRasp))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the rasp_id is empty", func() {
			rasp := *start.TestRasp
			rasp.Id = ""
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of rasp_id is less than 16", func() {
			rasp := *start.TestRasp
			rasp.Id = "123456789"
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of version is greater than 50", func() {
			rasp := *start.TestRasp
			rasp.Version = inits.GetLongString(51)
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of hostname is greater than 1024", func() {
			rasp := *start.TestRasp
			rasp.HostName = inits.GetLongString(1025)
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the hostname is empty", func() {
			rasp := *start.TestRasp
			rasp.HostName = ""
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the language is empty", func() {
			rasp := *start.TestRasp
			rasp.Language = ""
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of language is greater than 50", func() {
			rasp := *start.TestRasp
			rasp.Language = inits.GetLongString(51)
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the language version is empty", func() {
			rasp := *start.TestRasp
			rasp.LanguageVersion = ""
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of language version is greater than 50", func() {
			rasp := *start.TestRasp
			rasp.LanguageVersion = inits.GetLongString(51)
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of server type is greater than 256", func() {
			rasp := *start.TestRasp
			rasp.ServerType = inits.GetLongString(257)
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of server version is greater than 50", func() {
			rasp := *start.TestRasp
			rasp.ServerVersion = inits.GetLongString(51)
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the register ip is invalid ip address", func() {
			rasp := *start.TestRasp
			rasp.RegisterIp = "123456.1223"
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the heartbeat interval is less than 0", func() {
			rasp := *start.TestRasp
			rasp.HeartbeatInterval = -10
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the environ is nil", func() {
			rasp := *start.TestRasp
			rasp.Environ = nil
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the length of environ key is greater than 4096", func() {
			rasp := *start.TestRasp
			rasp.Environ = map[string]string{inits.GetLongString(4097): "123"}
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of environ value is greater than 4096", func() {
			rasp := *start.TestRasp
			rasp.Environ = map[string]string{"123": inits.GetLongString(4097)}
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestSearchRasp(t *testing.T) {
	Convey("Subject: Test Rasp Search Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/rasp/search", inits.GetJson(
				map[string]interface{}{
					"data":    start.TestRasp,
					"page":    1,
					"perpage": 1,
				},
			))
			So(r.Status, ShouldEqual, 0)
		})
	})
}

func TestDeleteRasp(t *testing.T) {
	Convey("Subject: Test Rasp Delete Api\n", t, func() {
		Convey("when the length of environ value is greater than 4096", func() {
			rasp := *start.TestRasp
			rasp.Environ = map[string]string{"123": inits.GetLongString(4097)}
			r := inits.GetResponse("POST", "/v1/agent/rasp", inits.GetJson(rasp))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}
