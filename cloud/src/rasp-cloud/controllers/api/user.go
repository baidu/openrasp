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

package api

import (
	"crypto/md5"
	"fmt"
	"math/rand"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"strconv"
	"time"
)

type UserController struct {
	controllers.BaseController
}

// @router /login [post]
func (o *UserController) Login() {
	var loginData map[string]string
	o.UnmarshalJson(&loginData)
	logUser := loginData["username"]
	logPasswd := loginData["password"]
	if logUser == "" || logPasswd == "" {
		o.ServeError(http.StatusBadRequest, "username or password cannot be empty")
	}
	if len(logUser) > 512 || len(logPasswd) > 512 {
		o.ServeError(http.StatusBadRequest, "the length of username or password cannot be greater than 512")
	}
	err := models.VerifyUser(logUser, logPasswd)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "username or password is incorrect", err)
	}
	cookie := fmt.Sprintf("%x", md5.Sum([]byte(strconv.Itoa(rand.Intn(10000)) + logUser + "openrasp"+
		strconv.FormatInt(time.Now().UnixNano(), 10))))
	err = models.NewCookie(cookie)
	if err != nil {
		o.ServeError(http.StatusUnauthorized, "failed to create cookie", err)
	}
	o.Ctx.SetCookie(models.AuthCookieName, cookie)
	o.ServeWithEmptyData()
}

// @router /islogin [get,post]
func (o *UserController) IsLogin() {
	o.ServeWithEmptyData()
}

// @router /update [post]
func (o *UserController) Update() {
	var param struct {
		OldPwd string `json:"old_password"`
		NewPwd string `json:"new_password"`
	}
	o.UnmarshalJson(&param)
	if param.OldPwd == "" {
		o.ServeError(http.StatusBadRequest, "old_password can not be empty")
	}
	if param.NewPwd == "" {
		o.ServeError(http.StatusBadRequest, "new_password can not be empty")
	}
	err := models.RemoveAllCookie()
	if err != nil {
		o.ServeError(http.StatusBadRequest, err.Error())
	}
	err = models.UpdatePassword(param.OldPwd, param.NewPwd)
	if err != nil {
		o.ServeError(http.StatusBadRequest, err.Error())
	}
	o.ServeWithEmptyData()
}

// @router /logout [get,post]
func (o *UserController) Logout() {
	o.Ctx.SetCookie(models.AuthCookieName, "")
	cookie := o.Ctx.GetCookie(models.AuthCookieName)
	models.RemoveCookie(cookie)
	o.ServeWithEmptyData()
}
