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

package api

import (
	"rasp-cloud/controllers"
	"net/http"
	"rasp-cloud/models"
	"encoding/json"
	"math"
)

type TokenController struct {
	controllers.BaseController
}

// @router /get [post]
func (o *TokenController) Get() {
	var param map[string]int
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &param)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error", err)
	}
	page := param["page"]
	if page <= 0 {
		o.ServeError(http.StatusBadRequest, "page must be greater than 0")
	}
	perpage := param["perpage"]
	if perpage <= 0 {
		o.ServeError(http.StatusBadRequest, "perpage must be greater than 0")
	}
	total, tokens, err := models.GetAllToken(page, perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get tokens", err)
	}
	if tokens == nil {
		tokens = make([]*models.Token, 0)
	}
	var result = make(map[string]interface{})
	result["total"] = total
	result["total_page"] = math.Ceil(float64(total) / float64(perpage))
	result["page"] = page
	result["perpage"] = perpage
	result["data"] = tokens
	o.Serve(result)
}

// @router / [post]
func (o *TokenController) Post() {
	var token *models.Token
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &token)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error", err)
	}
	if len(token.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of the token description must be less than 1024")
	}
	token, err = models.AddToken(token)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to create new token", err)
	}
	o.Serve(token)
}

// @router /delete [post]
func (o *TokenController) Delete() {
	var token *models.Token
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &token)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json format error", err)
	}
	if len(token.Token) == 0 {
		o.ServeError(http.StatusBadRequest, "the token param cannot be empty")
	}
	currentToken := o.Ctx.Input.Header(models.AuthTokenName)
	if currentToken == token.Token {
		o.ServeError(http.StatusBadRequest, "can not delete the token currently in use")
	}
	token, err = models.RemoveToken(token.Token)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove token", err)
	}
	o.Serve(token)
}
