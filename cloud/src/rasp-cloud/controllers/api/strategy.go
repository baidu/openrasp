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
	"encoding/json"
	"gopkg.in/mgo.v2/bson"
	"math"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
)

// Operations about strategy
type StrategyController struct {
	controllers.BaseController
}

// @router /search [post]
func (o *StrategyController) Search() {
	var param struct {
		Data    *models.Strategy `json:"data" `
		Page    int          	 `json:"page"`
		Perpage int          	 `json:"perpage"`
	}
	o.UnmarshalJson(&param)
	if param.Data == nil {
		o.ServeError(http.StatusBadRequest, "search data can not be empty")
	}
	o.ValidPage(param.Page, param.Perpage)
	total, strategies, err := models.FindStrategy(param.Data, param.Page, param.Perpage)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get strategy", err)
	}
	if strategies == nil {
		strategies = make([]*models.Strategy, 0)
	}
	var result = make(map[string]interface{})
	result["total"] = total
	result["total_page"] = math.Ceil(float64(total) / float64(param.Perpage))
	result["page"] = param.Page
	result["perpage"] = param.Perpage
	result["data"] = strategies
	o.Serve(result)
}

func (o *StrategyController) validateStrategyConfig(config map[string]interface{}) {
	for key, value := range config {
		if value == nil {
			o.ServeError(http.StatusBadRequest, "the value of "+key+" config cannot be nil")
		}
		if key == "" {
			o.ServeError(http.StatusBadRequest,
				"the config key can not be empty")
		}
		if len(key) > 512 {
			o.ServeError(http.StatusBadRequest,
				"the length of config key '"+key+"' must be less than 512")
		}
		if v, ok := value.(string); ok {
			if len(v) >= 2048 {
				o.ServeError(http.StatusBadRequest,
					"the value's length of config item '"+key+"' must be less than 2048")
			}
		}
		if v, ok := value.(float64); ok {
			if v < 0 {
				o.ServeError(http.StatusBadRequest,
					"the value of config item '"+key+"' can not be less than 0")
			} else if key == "plugin.timeout.millis" || key == "body.maxbytes" || key == "syslog.reconnect_interval" ||
				key == "ognl.expression.minlength"{
				if v == 0 {
					o.ServeError(http.StatusBadRequest,
						"the value of config item '"+key+"' must be greater than 0")
				}
			}
		}
	}
}

func (o *StrategyController) validateWhiteListConfig(config []models.WhitelistConfigItem) {
	if len(config) > 200 {
		o.ServeError(http.StatusBadRequest,
			"the count of whitelist config items must be between (0,200]")
	}
	for _, value := range config {
		if len(value.Url) > 200 || len(value.Url) == 0 {
			o.ServeError(http.StatusBadRequest,
				"the length of whitelist config url must be between [1,200]")
		}
		for key := range value.Hook {
			if len(key) > 128 {
				o.ServeError(http.StatusBadRequest,
					"the length of hook's type can not be greater 128")
			}
		}
	}
}

// @router / [post]
func (o *StrategyController) Post() {
	var strategy = &models.Strategy{}
	o.UnmarshalJson(strategy)

	if strategy.AppId == ""{
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	if strategy.Name == "" {
		o.ServeError(http.StatusBadRequest, "strategy name cannot be empty")
	}
	if len(strategy.Name) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of strategy name cannot be greater than 64")
	}
	if len(strategy.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of the description can not be greater than 1024")
	}

	strategy, err := models.AddStratety(strategy)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "create strategy failed", err)
	}
	models.AddOperation(strategy.Id, models.OperationTypeAddStrategy, o.Ctx.Input.IP(),
		"New strategy created with name " + strategy.Name)
	o.Serve(strategy)
}

// @router /config [post]
func (o *StrategyController) ConfigStrategy() {
	var param struct {
		StrategyId       string `json:"strategy_id"           bson:"_id"`
		AppId 			 string `json:"app_id"                bson:"app_id"`
		Name        	 string `json:"name,omitempty"        bson:"name"`
		Description 	 string `json:"description,omitempty" bson:"description"`
	}

	o.UnmarshalJson(&param)
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id can not be empty")
	}
	_, err := models.GetStrategyById(param.StrategyId, param.AppId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get strategy", err)
	}
	if param.Name == "" {
		o.ServeError(http.StatusBadRequest, "app name cannot be empty")
	}
	if len(param.Name) > 64 {
		o.ServeError(http.StatusBadRequest, "the length of app name cannot be greater than 64")
	}

	if len(param.Description) > 1024 {
		o.ServeError(http.StatusBadRequest, "the length of app description can not be greater than 1024")
	}
	updateData := bson.M{"name": param.Name, "description": param.Description}
	strategy, err := models.UpdateStrategyById(param.StrategyId, param.AppId, updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to update strategy config", err)
	}
	operationData, err := json.Marshal(updateData)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "json Marshal error", err)
	}
	models.AddOperation(strategy.Id, models.OperationTypeEditStrategy, o.Ctx.Input.IP(), "Updated strategy info for "+
		param.StrategyId+": "+string(operationData))
	o.Serve(strategy)
}

// @router /select [post]
func (o *StrategyController) Select() {
	var param struct {
		StrategyId  string                  `json:"strategy_id"  bson:"_id"`
		RaspId 		[]string	            `json:"rasp_id"`
		AppId       string					`json:"app_id"`
	}

	o.UnmarshalJson(&param)
	if param.StrategyId == "" {
		o.ServeError(http.StatusBadRequest, "strategy_id cannot be empty")
	}
	if param.AppId == "" {
		o.ServeError(http.StatusBadRequest, "app_id cannot be empty")
	}
	if len(param.RaspId) <= 0 {
		o.ServeError(http.StatusBadRequest, "the length of rasp_id should be greater than 0")
	}
	exist, err := models.SelectStratety(param.StrategyId, param.AppId, param.RaspId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "post strategy failed", err)
	}
	if exist {
		for _, raspId := range param.RaspId {
			models.AddOperation(param.StrategyId, models.OperationTypeSelectStrategy, o.Ctx.Input.IP(),
				"Select strategy for rasp, id:" + raspId)
		}
		o.ServeWithEmptyData()
	} else {
		o.ServeError(http.StatusBadRequest, "the strategy does not exist in this app", err)
	}
}

// @router /delete [post]
func (o *StrategyController) Delete() {
	var strategy = &models.Strategy{}
	o.UnmarshalJson(strategy)

	if strategy.Id == "" {
		o.ServeError(http.StatusBadRequest, "the id cannot be empty")
	}
	if strategy.AppId == ""{
		o.ServeError(http.StatusBadRequest, "the app_id cannot be empty")
	}
	mutex.Lock()
	defer mutex.Unlock()
	strategy, err := models.RemoveStrategyById(strategy.Id)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to remove this strategy", err)
	}

	models.AddOperation(strategy.Id, models.OperationTypeDeleteStrategy, o.Ctx.Input.IP(),
		"Deleted strategy with name "+ strategy.Name)
	o.ServeWithEmptyData()
}