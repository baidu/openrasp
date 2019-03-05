package inits

import (
	"fmt"
	"path/filepath"
	"rasp-cloud/tools"
	"github.com/bouk/monkey"
	"bytes"
	"net/http/httptest"
	"github.com/astaxie/beego"
	"net/http"
	"encoding/json"
	"testing"
	. "github.com/smartystreets/goconvey/convey"
)

type Response struct {
	Status int         `json:"status"`
	Desc   string      `json:"description"`
	Data   interface{} `json:"data"`
}

func init() {
	apppath, _ := filepath.Abs(filepath.Dir("../"))
	monkey.Patch(tools.GetCurrentPath, func() (string, error) {
		return apppath, nil
	})
	fmt.Println(tools.GetCurrentPath())
}

func GetResponse(t *testing.T, method string, path string, body string) (*Response) {
	r, _ := http.NewRequest(method, path, bytes.NewBuffer([]byte(body)))
	w := httptest.NewRecorder()
	beego.BeeApp.Handlers.ServeHTTP(w, r)
	beego.Trace("testing", "TestGet", "Code[%d]\n%s", w.Code, w.Body.String())
	response := &Response{}
	Convey("Request Status Code Should be 200", t, func() {
		So(w.Code, ShouldEqual, 200)
	})
	err := json.Unmarshal(w.Body.Bytes(), response)
	Convey("Response Format Should Be Valid Json ", t, func() {
		So(err, ShouldEqual, nil)
	})
	return response
}
