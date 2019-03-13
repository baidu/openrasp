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
	. "github.com/smartystreets/goconvey/convey"
)

type Response struct {
	Status int         `json:"status"`
	Desc   string      `json:"description"`
	Data   interface{} `json:"data"`
}

func init() {
	tools.GetCurrentPath()
	tools.PathExists("/xxx/xxx/xxxxxxx")
	apppath, _ := filepath.Abs(filepath.Dir("./"))
	monkey.Patch(tools.GetCurrentPath, func() (string, error) {
		return apppath, nil
	})
	fmt.Println(tools.GetCurrentPath())
}

func GetResponse(method string, path string, body string) (*Response) {
	r, _ := http.NewRequest(method, path, bytes.NewBuffer([]byte(body)))
	w := httptest.NewRecorder()
	beego.BeeApp.Handlers.ServeHTTP(w, r)
	response := &Response{}
	So(w.Code, ShouldEqual, 200)
	err := json.Unmarshal(w.Body.Bytes(), response)
	So(err, ShouldEqual, nil)
	return response
}

func GetResponseRecorder(method string, path string, body string) (*httptest.ResponseRecorder) {
	r, _ := http.NewRequest(method, path, bytes.NewBuffer([]byte(body)))
	w := httptest.NewRecorder()
	beego.BeeApp.Handlers.ServeHTTP(w, r)
	return w
}

func GetJson(data interface{}) string {
	jsonBytes, _ := json.Marshal(data)
	return string(jsonBytes)
}

func GetLongString(length int) string {
	result := ""
	for i := 0; i < length; i++ {
		result += "a"
	}
	return result
}

func GetLongStringArray(length int) []string {
	result := make([]string, length)
	for i := 0; i < length; i++ {
		result[i] = "a"
	}
	return result
}
