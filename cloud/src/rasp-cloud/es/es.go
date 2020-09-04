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

package es

import (
	"github.com/olivere/elastic"
	"time"
	"context"
	"github.com/astaxie/beego/logs"
	"strconv"
	"github.com/astaxie/beego"
	"rasp-cloud/tools"
	"fmt"
	"strings"
	"rasp-cloud/conf"
	"errors"
	"encoding/json"
	"rasp-cloud/environment"
)

var (
	ElasticClient *elastic.Client
	Version       string
	ttlIndexes    = make(chan map[string]time.Duration, 1)
	minEsVersion  = "5.6.0"
	maxEsVersion  = "7.0.0"
)

func init() {
	ttlIndexes <- make(map[string]time.Duration)
	if *conf.AppConfig.Flag.StartType != conf.StartTypeReset {
		esAddr := conf.AppConfig.EsAddr
		client, err := elastic.NewSimpleClient(elastic.SetURL(esAddr...),
			elastic.SetBasicAuth(conf.AppConfig.EsUser, conf.AppConfig.EsPwd),
			elastic.SetSnifferTimeoutStartup(5*time.Second),
			elastic.SetSnifferTimeout(5*time.Second),
			elastic.SetSnifferInterval(30*time.Minute))
		if err != nil {
			tools.Panic(tools.ErrCodeESInitFailed, "init ES failed", err)
		}
		go startTTL(24 * time.Hour)

		Version, err = client.ElasticsearchVersion(esAddr[0])
		if err != nil {
			tools.Panic(tools.ErrCodeESInitFailed, "failed to get es version", err)
		}
		beego.Info("ES version: " + Version)
		if strings.Compare(Version, minEsVersion) < 0 {
			tools.Panic(tools.ErrCodeESInitFailed, "unable to support the ElasticSearch with a version lower than "+
				minEsVersion+ ","+ " the current version is "+ Version, nil)
		}
		if strings.Compare(Version, maxEsVersion) >= 0 {
			tools.Panic(tools.ErrCodeESInitFailed,
				"unable to support the ElasticSearch with a version greater than or equal to "+
					maxEsVersion+ ","+ " the current version is "+ Version, nil)
		}
		ElasticClient = client
	}
}

func startTTL(duration time.Duration) {
	ticker := time.NewTicker(duration)
	for {
		select {
		case <-ticker.C:
			DeleteExpiredData()
		}
	}
}

func DeleteExpiredData() {
	defer func() {
		if r := recover(); r != nil {
			beego.Error(r)
		}
	}()
	ttls := <-ttlIndexes
	defer func() {
		ttlIndexes <- ttls
	}()
	for index, duration := range ttls {
		expiredTime := strconv.FormatInt((time.Now().UnixNano()-int64(duration))/1000000, 10)
		ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
		r, err := ElasticClient.DeleteByQuery(index).QueryString("@timestamp:<" + expiredTime).Do(ctx)
		if err != nil {
			if r != nil && r.Failures != nil {
				beego.Error(r.Failures)
			}
			beego.Error("failed to delete expired data for index " + index + ": " + err.Error())
		} else {
			var deleteNum int64
			if r != nil {
				deleteNum = r.Deleted
			}
			beego.Info("delete expired data successfully for index " + index + ", total: " +
				strconv.FormatInt(deleteNum, 10))
		}
		cancel()
	}
}

func DeleteLogs(index string) (err error) {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(30*time.Second))
	defer cancel()
	expiredTime := strconv.FormatInt((time.Now().UnixNano())/1000000, 10)
	//r, err := ElasticClient.Delete().Index(index).Type(docType).Id("*").Do(ctx)
	r, err := ElasticClient.DeleteByQuery(index).QueryString("@timestamp:<" + expiredTime).Do(ctx)
	if err != nil {
		if r != nil && r.Failures != nil {
			beego.Error(r.Failures)
		}
		beego.Error("failed to delete expired data for index " + index + ": " + err.Error())
	} else {
		var deleteNum int64
		if r != nil {
			deleteNum = r.Deleted
		}
		beego.Info("delete expired data successfully for index " + index + ", total: " +
			strconv.FormatInt(deleteNum, 10))
	}
	return err
}

func RegisterTTL(duration time.Duration, index string) {
	ttls := <-ttlIndexes
	defer func() {
		ttlIndexes <- ttls
	}()
	ttls[index] = duration
}

func CreateTemplate(name string, body string) error {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	_, err := elastic.NewIndicesPutTemplateService(ElasticClient).Name(name).BodyString(body).Do(ctx)
	if err != nil {
		tools.Panic(tools.ErrCodeESInitFailed, "failed to create es template: "+name, err)
	}
	beego.Info("put es template: " + name)
	return nil
}

func CreateEsIndex(index string, alias string, template string) error {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(180*time.Second))
	defer cancel()
	exists, err := ElasticClient.IndexExists(index).Do(ctx)
	if err != nil {
		return err
	}
	if !exists {
		createResult, err := ElasticClient.CreateIndex(index).Do(ctx)
		if err != nil {
			return err
		}
		logs.Info("create es index: " + createResult.Index)
	} else {
		if environment.UpdateMappingConfig[template] != nil {
			beego.Info("updating template name:", template, "alias:", alias)
			destIndex := index + "-" + environment.UpdateMappingConfig[template].(string)
			err := UpdateMapping(destIndex, alias, template, ctx)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func Insert(index string, docType string, doc interface{}) (err error) {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(10*time.Second))
	defer cancel()
	_, err = ElasticClient.Index().Index(index).Type(docType).BodyJson(doc).Do(ctx)
	return
}

func BulkInsertAlarm(docType string, docs []map[string]interface{}) (err error) {
	bulkService := ElasticClient.Bulk()
	for _, doc := range docs {
		if doc["app_id"] == nil {
			beego.Error("failed to get app_id param from alarm: " + fmt.Sprintf("%+v", doc))
		}
		if appId, ok := doc["app_id"].(string); ok {
			if docType == "policy-alarm" || docType == "error-alarm" || docType == "attack-alarm" ||
				docType == "crash-alarm" {
				bulkService.Add(elastic.NewBulkUpdateRequest().
					Index("real-openrasp-" + docType + "-" + appId).
					Type(docType).
					Id(fmt.Sprint(doc["upsert_id"])).
					DocAsUpsert(true).
					Doc(doc))
			} else {
				if appId, ok := doc["app_id"].(string); ok {
					bulkService.Add(elastic.NewBulkIndexRequest().
						Index("real-openrasp-" + docType + "-" + appId).
						Type(docType).
						OpType("index").
						Doc(doc))
				}
			}
		} else {
			beego.Error("the type of alarm's app_id param is not string: " + fmt.Sprintf("%+v", doc))
		}
	}
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	response, err := bulkService.Do(ctx)
	if response.Errors {
		errContent, err := json.Marshal(response.Failed())
		if err == nil {
			return errors.New("ES bulk has errors: " + string(errContent))
		}
	}
	return err
}

func BulkInsert(index string, docType string, docs []map[string]interface{}) (err error) {
	bulkService := ElasticClient.Bulk()
	for _, doc := range docs {
		bulkService.Add(elastic.NewBulkUpdateRequest().
			Index(index).
			Type(docType).
			Id(fmt.Sprint(doc["upsert_id"])).
			DocAsUpsert(true).
			Doc(doc["content"]))
	}
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	_, err = bulkService.Do(ctx)
	return err
}

func DeleteIndex(indexName string) error {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	_, err := ElasticClient.DeleteIndex(indexName).Do(ctx)
	return err
}

func DeleteByQuery(index string, docType string, query elastic.Query) error {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	_, err := ElasticClient.DeleteByQuery(index).Type(docType).Query(query).ProceedOnVersionConflict().Do(ctx)
	if err != nil {
		beego.Error("failed to delete by query", err)
		return err
	}
	return nil
}

func GetIndex(base string, appId string) string {
	return base + "-" + appId
}

func HandleSearchResult(result map[string]interface{}, id string) {
	result["id"] = id
	delete(result, "_@timestamp")
	delete(result, "@version")
	delete(result, "tags")
	delete(result, "host")
}

func UpdateMapping(destIndex string, alias string, template string, ctx context.Context) error {
	// 获取alias对应的index
	res, err := ElasticClient.Aliases().Index(alias).Do(ctx)
	if err != nil {
		return err
	}

	if len(res.IndicesByAlias(alias)) == 0 {
		return errors.New("alias:" + fmt.Sprintf("%s", res) + "is not exist!")
	}
	if len(res.IndicesByAlias(alias)) > 2 {
		return errors.New("find duplicate alias:" + fmt.Sprintf("%s", res.IndicesByAlias(alias)))
	}

	oldIndexFromEs := res.IndicesByAlias(alias)[0]
	if len(res.IndicesByAlias(alias)) == 2 {
		if oldIndexFromEs == destIndex {
			oldIndexFromEs = res.IndicesByAlias(alias)[1]
		}
	}

	exists, err := ElasticClient.IndexExists(destIndex).Do(ctx)
	if err != nil {
		return err
	}
	if !exists {
		// 创建新索引
		err = CreateEsIndex(destIndex, alias, template)
		if err != nil {
			return err
		}
		//aliases := ElasticClient.Alias()
		// 去掉新建索引中的模版索引，换成原索引的模版索引
		//aliasName := res.Indices[destIndex].Aliases[0].AliasName
		//_, err = aliases.Add(destIndex, alias).Remove(destIndex, aliasName).Do(ctx)
		//if err != nil {
		//	return err
		//}
		_, err = ElasticClient.Reindex().SourceIndex(oldIndexFromEs).DestinationIndex(destIndex).
			ProceedOnVersionConflict().Do(ctx)
		if err != nil {
			return err
		}
		destIndexAlias, err := ElasticClient.Aliases().Index(destIndex).Do(ctx)
		if err != nil {
			return err
		}
		destIndexAliasName := destIndexAlias.Indices[destIndex].Aliases[0].AliasName
		_, err = ElasticClient.Alias().Remove(oldIndexFromEs, alias).Add(destIndex, alias).
			Remove(destIndex, destIndexAliasName).Do(ctx)
		if err != nil {
			return err
		}
		beego.Info("upgrade index success! newIndex:", destIndex, "oldIndex:", oldIndexFromEs)
		return nil
	}
	return nil
}
