package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	. "github.com/smartystreets/goconvey/convey"
	"time"
	"github.com/bouk/monkey"
	"reflect"
	"github.com/astaxie/beego/context"
	"rasp-cloud/models"
	"errors"
)

func TestHeartBeat(t *testing.T) {
	Convey("Subject: Test Heartbeat Api\n", t, func() {
		start.TestApp.ConfigTime = int64(time.Now().Nanosecond() / 1000000)
		start.TestApp.WhitelistConfig = []models.WhitelistConfigItem{{
			Url: "http://rasp.baidu.com/a/b",
			Hook: map[string]bool{
				"sql": true,
			},
		}}
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
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the mongodb has errors", func() {
			data := map[string]interface{}{
				"rasp_id":        start.TestRasp.Id,
				"plugin_version": start.TestRasp.PluginVersion + time.Now().String(),
				"plugin_md5":     "5165165b1ccc",
				"config_time":    0,
			}
			monkey.Patch(models.GetRaspById,
				func(id string) (rasp *models.Rasp, err error) {
					return nil, errors.New("")
				},
			)
			r := inits.GetResponse("POST", "/v1/agent/heartbeat", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetRaspById)

			monkey.Patch(models.UpsertRaspById,
				func(id string, rasp *models.Rasp) (error) {
					return errors.New("")
				},
			)
			r = inits.GetResponse("POST", "/v1/agent/heartbeat", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.UpsertRaspById)

			monkey.Patch(models.GetSelectedPlugin,
				func(appId string, hasContent bool) (plugin *models.Plugin, err error) {
					return nil, errors.New("")
				},
			)
			r = inits.GetResponse("POST", "/v1/agent/heartbeat", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetSelectedPlugin)
		})

	})
}
