//Copyright 2017-2018 Baidu Inc.
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

package models

import (
	"github.com/astaxie/beego"
	"rasp-cloud/tools"
	"regexp"
)

var (
	user     string
	password string
)

func init() {
	user = beego.AppConfig.String("user")
	password = beego.AppConfig.String("passwd")
	hasNum := regexp.MustCompile(".*[0-9].*").Match([]byte(password))
	hasLetter := regexp.MustCompile(".*([a-z]|[A-Z]).*").Match([]byte(password))
	if len(user) == 0 || len(password) < 8 || len(password) > 50 || !hasNum || !hasLetter {
		tools.Panic("the login user and password can not be empty," +
			"the length of password can not be less than 8," +
			"the length of password can not be greater than 50," +
			"password must contain numbers and letters")
	}
}

func GetLoginUser() string {
	return user
}

func GetLoginPasswd() string {
	return password
}
