package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	"rasp-cloud/models"
	. "github.com/smartystreets/goconvey/convey"
)

func getValidDependency() map[string]interface{} {
	return
}

func TestPostDependency(t *testing.T) {
	Convey("Subject: Test Delete App Api\n", t, func() {
		Convey("when app data valid", func() {
			app := getValidApp()
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			defer models.RemoveAppById(r.Data.(map[string]interface{})["id"].(string))
			So(r.Status, ShouldEqual, 0)
		})
	})
}
