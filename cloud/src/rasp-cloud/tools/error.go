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

package tools

import (
	"github.com/astaxie/beego/logs"
	"strconv"
	"os"
)

const (
	ErrCodeLogInitFailed          = 30001 + iota
	ErrCodeMongoInitFailed
	ErrCodeESInitFailed
	ErrCodeConfigInitFailed
	ErrCodeStartTypeNotSupport
	ErrCodeGeneratePasswdFailed
	ErrCodeGeoipInit
	ErrCodeResetUserFailed
	ErrCodeInitDefaultAppFailed
	ErrCodeInitChildProcessFailed
	ErrCodeChDirFailed
)

func Panic(errCode int, message string, err error) {
	message = "[" + strconv.Itoa(errCode) + "] " + message
	if err != nil {
		message = message + ": " + err.Error()
	}
	logs.Error(message)
	os.Exit(errCode)
}
