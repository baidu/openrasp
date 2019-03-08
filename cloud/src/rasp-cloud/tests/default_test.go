package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
)

func Test404(t *testing.T) {
	Convey("Subject: Test 404\n", t, func() {
		r := inits.GetResponseRecorder("POST", "/v1/api/notexist", "")
		So(r.Code, ShouldEqual, 404)
	})

	Convey("Subject: Test Ping\n", t, func() {
		r := inits.GetResponse("GET", "/v1/ping", "")
		So(r.Status, ShouldEqual, 0)
	})
}
