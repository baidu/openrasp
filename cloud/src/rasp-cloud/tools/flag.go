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

package tools

import (
	"flag"
	"fmt"
)

const (
	StartTypeForeground = "panel"
	StartTypeAgent      = "agent"
	StartTypeAll        = "all"
)

var StartType *string

func init() {
	StartType = flag.String("type", "", "use to provide different routers")
	flag.Parse()
	fmt.Println("===== start type: " + *StartType + "=====")
	if *StartType == "" {
		allType := StartTypeAll
		StartType = &allType
	}
}
