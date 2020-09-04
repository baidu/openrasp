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

package api

import (
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"net/http"
	"math"
)

type DependencyController struct {
	controllers.BaseController
}

// @router /search [post]
func (o *DependencyController) Search() {
	param := o.handleSearchParam()
	total, result, err := models.SearchDependency(param.Data.AppId, param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "search dependency failed", err)
	}
	o.Serve(map[string]interface{}{
		"total":      total,
		"total_page": math.Ceil(float64(total) / float64(param.Perpage)),
		"page":       param.Page,
		"perpage":    param.Perpage,
		"data":       result,
	})
}

// @router /aggr [post]
func (o *DependencyController) AggrWithSearch() {
	param := o.handleSearchParam()
	total, result, err := models.AggrDependencyByQuery(param.Data.AppId, param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "aggregation dependency failed", err)
	}
	o.Serve(map[string]interface{}{
		"total":      total,
		"total_page": math.Ceil(float64(total) / float64(param.Perpage)),
		"page":       param.Page,
		"perpage":    param.Perpage,
		"data":       result,
	})
}

func (o *DependencyController) handleSearchParam() *models.SearchDependencyParam {
	var param models.SearchDependencyParam
	o.UnmarshalJson(&param)
	o.ValidPage(param.Page, param.Perpage)
	o.ValidParam(&param)
	o.ValidParam(param.Data)
	return &param
}
