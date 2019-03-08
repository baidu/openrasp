package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
)

func TestRaspRegister(t *testing.T) {
	Convey("Subject: Test Rasp Register Api\n", t, func() {
		//Convey("when the param is valid", func() {
		//	r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
		//		"id": plugin.Id,
		//	}))
		//	So(r.Status, ShouldEqual, 0)
		//})
		//
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

func TestSearchRasp(t *testing.T) {

}

func TestDeleteRasp(t *testing.T) {

}
