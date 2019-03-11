package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	. "github.com/smartystreets/goconvey/convey"
	"time"
	"fmt"
	"github.com/bouk/monkey"
	"reflect"
	"github.com/astaxie/beego/context"
)

func TestHeartBeat(t *testing.T) {
	Convey("Subject: Test Heartbeat Api\n", t, func() {
		start.TestApp.ConfigTime = int64(time.Now().Nanosecond() / 1000000)
		monkey.PatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header",
			func(input *context.BeegoInput, key string) string {
				return start.TestApp.Id
			},
		)
		defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header")
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/agent/heartbeat", inits.GetJson(map[string]interface{}{
				"rasp_id":        start.TestRasp.Id,
				"plugin_version": start.TestRasp.PluginVersion + time.Now().String(),
				"plugin_md5":     "5165165b1ccc",
				"config_time":    0,
			}))
			fmt.Println(r.Desc)
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the app id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/agent/heartbeat", inits.GetJson(map[string]interface{}{
				"rasp_id":        start.TestRasp.Id,
				"plugin_version": start.TestRasp.PluginVersion + time.Now().String(),
				"plugin_md5":     "5165165b1ccc",
				"config_time":    0,
			}))
			fmt.Println(r.Desc)
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the app id doesn't exist", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header",
				func(input *context.BeegoInput, key string) string {
					return "00000000000000"
				},
			)
			r := inits.GetResponse("POST", "/v1/agent/heartbeat", inits.GetJson(map[string]interface{}{
				"rasp_id":        start.TestRasp.Id,
				"plugin_version": start.TestRasp.PluginVersion + time.Now().String(),
				"plugin_md5":     "5165165b1ccc",
				"config_time":    0,
			}))
			fmt.Println(r.Desc)
			So(r.Status, ShouldEqual, 0)
		})

	})
}
