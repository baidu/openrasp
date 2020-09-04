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

package models

import (
	"crypto/sha1"
	"errors"
	"fmt"
	"github.com/astaxie/beego"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"math/rand"
	"rasp-cloud/mongo"
	"rasp-cloud/tools"
	"strconv"
	"time"
)

type Strategy struct {
	Id               	string                 `json:"id" bson:"_id"`
	AppId				string				   `json:"app_id" bson:"app_id"`
	Name             	string                 `json:"name"  bson:"name"`
	Description      	string                 `json:"description"  bson:"description"`
	CreateTime       	int64                  `json:"create_time"  bson:"create_time"`
	ConfigTime       	int64                  `json:"config_time"  bson:"config_time"`
	GeneralConfig   	map[string]interface{} `json:"general_config"  bson:"general_config"`
	WhitelistConfig  	[]WhitelistConfigItem  `json:"whitelist_config"  bson:"whitelist_config"`
	SelectedPluginId 	string                 `json:"selected_plugin_id" bson:"selected_plugin_id"`
	AlgorithmConfig     map[string]interface{} `json:"algorithm_config"`
}

const (
	strategyCollectionName = "strategy"
)

func init() {
	index := &mgo.Index{
		Key:        []string{"app_id"},
		Unique:     false,
		Background: true,
		Name:       "app_id",
	}
	err := mongo.CreateIndex(strategyCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed,
			"failed to create app_id index for strategy collection", err)
	}
}

func AddStratety(strategy *Strategy) (result *Strategy, err error) {
	strategy.Id = generateStrategyId(strategy.Name, strategy.AppId)
	strategy.CreateTime = time.Now().Unix()
	if err := mongo.FindOne(strategyCollectionName, bson.M{"name": strategy.Name}, &App{}); err != mgo.ErrNotFound {
		if err != nil {
			return nil, err
		}
		return nil, errors.New("duplicate strategy name")
	}

	err = mongo.Insert(strategyCollectionName, strategy)
	if err != nil {
		return nil, errors.New("failed to insert strategy to db: " + err.Error())
	}
	result = strategy
	beego.Info("Succeed to create strategy, name: " + strategy.Name)
	return
}

func generateStrategyId(Name string, AppId string) string {
	random := "openrasp_strategy" + Name + "_" + AppId + strconv.FormatInt(time.Now().UnixNano(), 10) +
		strconv.Itoa(rand.Intn(10000))
	return fmt.Sprintf("%x", sha1.Sum([]byte(random)))
}

func FindStrategy(selector *Strategy, page int, perpage int) (count int, result []*Strategy, err error) {
	var bsonContent []byte
	bsonContent, err = bson.Marshal(selector)
	if err != nil {
		return
	}
	bsonModel := bson.M{}
	err = bson.Unmarshal(bsonContent, &bsonModel)
	if err != nil {
		return
	}
	query := bson.M{"app_id": bsonModel["app_id"]}
	count, err = mongo.FindAll(strategyCollectionName, query, &result, perpage*(page-1), perpage, "name")
	if err != nil {
		return 0, nil, err
	}
	return count, result, err
}

func GetStrategyById(id string, appId string) (strategy *Strategy, err error) {
	err = mongo.FindOne(strategyCollectionName, bson.M{
		"_id":     id,
		"app_id":  appId}, &strategy)
	if err != nil {
		return nil, err
	}
	return strategy, nil
}

func UpdateStrategyById(strategyId string, appId string, doc interface{}) (strategy *Strategy, err error) {
	strategy, err = GetStrategyById(strategyId, appId)
	if err == nil && strategy != nil {
		err = mongo.UpdateId(strategyCollectionName, strategy.Id, doc)
		if err != nil {
			return
		}
	}
	return strategy, err
}

func UpdateGeneralConfigForStrategy(StrategyId string, appId string, config map[string]interface{}) (strategy *Strategy,
	err error) {
	return UpdateStrategyById(StrategyId, appId, bson.M{"general_config": config, "config_time": time.Now().UnixNano()})
}

func UpdateWhiteListConfigForStrategy(StrategyId string, appId string, config []WhitelistConfigItem) (strategy *Strategy,
	err error) {
	return UpdateStrategyById(StrategyId, appId, bson.M{"whitelist_config": config, "config_time": time.Now().UnixNano()})
}

func RemoveStrategyById(StrategyId string) (strategy *Strategy, err error) {
	err = mongo.FindId(strategyCollectionName, StrategyId, &strategy)
	if err != nil {
		return
	}
	return strategy, mongo.RemoveId(strategyCollectionName, StrategyId)
}

func GetStrategyCount() (count int, err error) {
	return mongo.Count(strategyCollectionName)
}

func SelectStratety(strategyId string, appId string, raspId []string) (exist bool, err error){
	for _, id := range raspId {
		rasp, err := GetRaspById(id)
		if err != nil {
			return false, err
		}

		strategy, err := GetStrategyById(strategyId, appId)
		if err == nil && strategy != nil {
			rasp.StrategyId = strategyId
			err = UpsertRaspById(id, rasp)
			if err != nil {
				return false, err
			}
			return true, nil
		}
	}
	return false, nil
}