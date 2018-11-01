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
	"rasp-cloud/mongo"
	"strconv"
	"time"
	"math/rand"
	"fmt"
	"crypto/sha1"
)

type Token struct {
	Token       string `json:"token" bson:"_id"`
	Description string `json:"description" bson:"description"`
}

const (
	tokenCollectionName = "token"
)

func GetAllTokent(page int, perpage int) (count int, result []*Token, err error) {
	count, err = mongo.FindAll(tokenCollectionName, nil, &result, perpage*(page-1), perpage)
	return
}

func HasTokent(token string) (bool, error) {
	var result *Token
	err := mongo.FindId(tokenCollectionName, token, &result)
	if err != nil || result == nil {
		return false, err
	}
	return true, err
}

func AddToken(token *Token) (result *Token, err error) {
	token.Token = generateToken()
	err = mongo.Insert(tokenCollectionName, token)
	result = token
	return
}

func RemoveToken(tokenId string) (token *Token, err error) {
	err = mongo.FindId(tokenCollectionName, tokenId, &token)
	if err != nil {
		return
	}
	return token, mongo.RemoveId(tokenCollectionName, tokenId)
}

func generateToken() string {
	random := "openrasp_token" + strconv.FormatInt(time.Now().UnixNano(), 10) + strconv.Itoa(rand.Intn(5000))
	return fmt.Sprintf("%x", sha1.Sum([]byte(random)))
}
