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
	"rasp-cloud/mongo"
)

type Token struct {
	Token       string `json:"token" bson:"_id"`
	Description string `json:"description" bson:"description"`
}

const (
	tokenCollectionName = "token"
	AuthTokenName       = "X-OpenRASP-Token"
)

func GetAllToken(page int, perpage int) (count int, result []*Token, err error) {
	count, err = mongo.FindAll(tokenCollectionName, nil, &result, perpage*(page-1), perpage)
	return
}

func HasToken(token string) (bool, error) {
	var result *Token
	err := mongo.FindId(tokenCollectionName, token, &result)
	if err != nil || result == nil {
		return false, err
	}
	return true, err
}

func AddToken(token *Token) (result *Token, err error) {
	token.Token = generateOperationId()
	err = mongo.Insert(tokenCollectionName, token)
	if err != nil {
		return
	}
	return token, err
}

func UpdateToken(token *Token) (result *Token, err error) {
	err = mongo.UpdateId(tokenCollectionName, token.Token, token)
	if err != nil {
		return
	}
	return token, err
}

func RemoveToken(tokenId string) (token *Token, err error) {
	err = mongo.FindId(tokenCollectionName, tokenId, &token)
	if err != nil {
		return
	}
	return token, mongo.RemoveId(tokenCollectionName, tokenId)
}
