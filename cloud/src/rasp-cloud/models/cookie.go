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

package models

import (
	"time"
	"rasp-cloud/mongo"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2"
	"rasp-cloud/conf"
	"gopkg.in/mgo.v2/bson"
)

type Cookie struct {
	Id   string    `json:"id" bson:"_id"`
	Time time.Time `json:"time" bson:"time"`
}

const (
	cookieCollectionName = "cookie"
	AuthCookieName       = "RASP_AUTH_ID"
)

func init() {
	index := &mgo.Index{
		Key:         []string{"time"},
		Background:  true,
		Name:        "time",
		ExpireAfter: time.Duration(conf.AppConfig.CookieLifeTime) * time.Hour,
	}
	err := mongo.CreateIndex(cookieCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to create index for app collection", err)
	}
}

func NewCookie(id string) error {
	return mongo.Insert(cookieCollectionName, &Cookie{Id: id, Time: time.Now()})
}

func HasCookie(id string) (bool, error) {
	var result *Cookie
	err := mongo.FindId(cookieCollectionName, id, &result)
	if err != nil || result == nil {
		return false, err
	}
	return true, err
}

func RemoveCookie(id string) error {
	return mongo.RemoveId(cookieCollectionName, id)
}

func RemoveAllCookie() error {
	_, err := mongo.RemoveAll(cookieCollectionName, bson.M{})
	return err
}
