//Copyright 2017-2020 Baidu Inc.
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

type ErrorController struct {
	BaseController
}

func (o *ErrorController) Error404() {
	o.errorStatus(404)
}

func (o *ErrorController) Error500() {
	o.errorStatus(500)
}

func (o *ErrorController) Error503() {
	o.errorStatus(503)
}

func (o *ErrorController) Error502() {
	o.errorStatus(502)
}

func (o *ErrorController) errorStatus(code int) {
	o.ServeStatusCode(code, code)
}
