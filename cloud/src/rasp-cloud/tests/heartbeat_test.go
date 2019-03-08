package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	. "github.com/smartystreets/goconvey/convey"
	"time"
	"fmt"
)

func TestHeartBeat(t *testing.T) {
	Convey("Subject: Test Heartbeat Api\n", t, func() {
		start.TestApp.ConfigTime = int64(time.Now().Nanosecond() / 1000000)
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

		//Convey("when the plugin is selected", func() {
		//	r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
		//		"id": start.TestApp.SelectedPluginId,
		//	}))
		//	So(r.Status, ShouldBeGreaterThan, 0)
		//})
		//
		//Convey("when the plugin id is empty", func() {
		//	r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
		//		"id": "",
		//	}))
		//	So(r.Status, ShouldBeGreaterThan, 0)
		//})
		//
		//Convey("when the plugin id does not exist", func() {
		//	r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
		//		"id": "0000000000000000000000000000000000",
		//	}))
		//	So(r.Status, ShouldBeGreaterThan, 0)
		//})
	})
}
