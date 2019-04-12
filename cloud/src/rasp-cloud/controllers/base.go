//Copyright 2017-2019 Baidu Inc.
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http: //www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

package controllers

import (
	"github.com/astaxie/beego"
	"net/http"
	"encoding/json"
)

// base controller
type BaseController struct {
	beego.Controller
}

func (o *BaseController) Serve(data interface{}) {
	o.Data["json"] = map[string]interface{}{"status": 0, "description": "ok", "data": data}
	o.ServeJSON()
}

func (o *BaseController) ServeWithEmptyData() {
	o.Data["json"] = map[string]interface{}{"status": 0, "description": "ok", "data": make(map[string]interface{})}
	o.ServeJSON()
}

func (o *BaseController) ServeError(code int, description string, err ...error) {
	if len(err) > 0 && err[0] != nil {
		description = description + ": " + err[0].Error()
	}
	o.ServeStatusCode(code, description)
	panic(description)
}

func (o *BaseController) ServeStatusCode(code int, description ...string) {
	var des string
	if len(description) == 0 {
		des = http.StatusText(code)
	} else {
		des = description[0]
	}
	o.Data["json"] = map[string]interface{}{"status": code, "description": des}
	o.ServeJSON()
}

func (o *BaseController) UnmarshalJson(v interface{}) {
	err := json.Unmarshal(o.Ctx.Input.RequestBody, v)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
}

func (o *BaseController) ValidPage(page int, perpage int) {
	if page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	if perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}
	if perpage > 100 {
		o.ServeError(http.StatusBadRequest, "perpage must be less than 100")
	}
}
