package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	_ "rasp-cloud/tests/start"
	"rasp-cloud/models"
)

var cookie = "******************"

func init() {
	if ok, _ := models.HasCookie(cookie); !ok {
		models.NewCookie(cookie, "1111")
	}
}

func TestHasCookie(t *testing.T) {
	Convey("Subject: Test Has Cookie\n", t, func() {
		Convey("when cookie exist", func() {
			has, _ := models.HasCookie(cookie)
			So(has, ShouldEqual, true)
		})

		Convey("when cookie doesn't exist", func() {
			has, _ := models.HasCookie("++++++++++++++++++")
			So(has, ShouldEqual, false)
		})
	})
}
