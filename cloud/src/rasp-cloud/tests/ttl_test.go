package test

import (
	_ "unsafe"
	_ "rasp-cloud/es"
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/es"
)

func TestTTL(t *testing.T) {
	Convey("Subject: Test ES TTL\n", t, func() {
		defer func() {
			r := recover()
			So(r, ShouldEqual, nil)
		}()
		es.DeleteExpiredData()
	})
}
