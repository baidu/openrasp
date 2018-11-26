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

package mongo

import (
	"gopkg.in/mgo.v2"
	"time"
	"github.com/astaxie/beego"
	"rasp-cloud/tools"
	"gopkg.in/mgo.v2/bson"
	"strconv"
	"math/rand"
	"fmt"
	"crypto/sha1"
)

var (
	session *mgo.Session
	DbName  = beego.AppConfig.String("MongoDBName")
)

func init() {
	poolLimit, err := beego.AppConfig.Int("MongoPoolLimit")
	if err != nil {
		poolLimit = 1024
	}
	dialInfo := &mgo.DialInfo{
		Addrs:     []string{beego.AppConfig.String("MongoDBAddr")},
		Direct:    false,
		Timeout:   time.Second * 20,
		PoolLimit: poolLimit,
	}
	session, err = mgo.DialWithInfo(dialInfo)

	if err != nil {
		tools.Panic("init mongodb failed: " + err.Error())
	}

	session.SetMode(mgo.Monotonic, true)
}

func NewSession() *mgo.Session {
	return session.Copy()
}

func Count(collection string) (int, error) {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).Count()
}

func CreateIndex(collection string, index *mgo.Index) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).EnsureIndex(*index)
}

func Insert(collection string, doc interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).Insert(doc)
}

func UpsertId(collection string, id interface{}, doc interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	_, err := newSession.DB(DbName).C(collection).UpsertId(id, doc)
	return err
}

func FindAll(collection string, query interface{}, result interface{}, skip int, limit int,
	sortFields ...string) (count int, err error) {
	newSession := NewSession()
	defer newSession.Close()
	count, err = newSession.DB(DbName).C(collection).Find(query).Count()
	if err != nil {
		return
	}
	err = newSession.DB(DbName).C(collection).Find(query).Skip(skip).Limit(limit).Sort(sortFields...).All(result)
	return
}

func FindAllWithSelect(collection string, query interface{}, result interface{}, selector interface{},
	skip int, limit int) (count int, err error) {
	newSession := NewSession()
	defer newSession.Close()
	count, err = newSession.DB(DbName).C(collection).Find(query).Count()
	if err != nil {
		return
	}
	err = newSession.DB(DbName).C(collection).Find(query).Select(selector).Skip(skip).Limit(limit).All(result)
	return
}

func FindId(collection string, id string, result interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).FindId(id).One(result)
}

func FindOne(collection string, query interface{}, result interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).Find(query).One(result)
}

func FindOneBySort(collection string, query interface{}, result interface{}, sortFields ...string) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).Find(query).Sort(sortFields...).One(result)
}

func FindAllBySort(collection string, query interface{}, skip int, limit int, result interface{}, sortFields ...string) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).Find(query).Sort(sortFields...).Skip(skip).Limit(limit).All(result)
}

func UpdateId(collection string, id interface{}, doc interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).UpdateId(id, bson.M{"$set": doc})
}

func RemoveId(collection string, id interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	return newSession.DB(DbName).C(collection).RemoveId(id)
}

func RemoveAll(collection string, selector interface{}) error {
	newSession := NewSession()
	defer newSession.Close()
	_, err := newSession.DB(DbName).C(collection).RemoveAll(selector)
	return err
}

func GenerateObjectId() string {
	random := string(bson.NewObjectId()) +
		strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(10000))
	return fmt.Sprintf("%x", sha1.Sum([]byte(random)))
}
