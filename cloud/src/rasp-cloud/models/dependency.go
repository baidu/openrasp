package models

import (
	"crypto/md5"
	"fmt"
	"rasp-cloud/es"
	"rasp-cloud/models/logs"
	"sort"
	"time"
	"github.com/olivere/elastic"
	"encoding/json"
	"context"
	"github.com/astaxie/beego"
	"strings"
)

type Dependency struct {
	Path         []string `json:"path"`
	CreateTime   int64    `json:"@timestamp"`
	RaspId       string   `json:"rasp_id"`
	HostName     string   `json:"hostname"`
	RegisterIp   string   `json:"register_ip"`
	AppId        string   `json:"app_id"`
	Vendor       string   `json:"vendor"`
	Product      string   `json:"product"`
	Version      string   `json:"version"`
	Tag          string   `json:"tag"`
	SearchString string   `json:"search_string"`
	Source       string   `json:"source"`
}

type SearchDependencyParam struct {
	Page    int `json:"page"`
	Perpage int `json:"perpage"`
	Data *struct {
		Id           string `json:"_id,omitempty" valid:"MaxSize(512)"`
		AppId        string `json:"app_id,omitempty" valid:"Required;MaxSize(512)"`
		RaspId       string `json:"rasp_id,omitempty" valid:"MaxSize(512)"`
		HostnameOrIp string `json:"hostname,omitempty" valid:"MaxSize(1024)"`
		Tag          string `json:"tag,omitempty" valid:"MaxSize(1024)"`
		KeyWord      string `json:"key_word,omitempty" valid:"MaxSize(1024)"`
		Source       string `json:"source,omitempty" valid:"MaxSize(1024)"`
	} `json:"data" valid:"Required"`
}

var (
	DependencyIndexName      = "openrasp-dependency-data"
	AliasDependencyIndexName = "real-openrasp-dependency-data"
	dependencyType           = "dependency"
)

func AddDependency(rasp *Rasp, dependencies []*Dependency) error {
	docs := make([]interface{}, 0, len(dependencies))
	docsMap := make([]map[string]interface{}, 0, len(dependencies))
	idContent := ""
	for _, dependency := range dependencies {
		doc := make(map[string]interface{})
		sort.Strings(dependency.Path)
		idContent += fmt.Sprint(dependency.Path)
		idContent += fmt.Sprint(dependency.RaspId)
		idContent += fmt.Sprint(dependency.Product)
		idContent += fmt.Sprint(dependency.Version)
		idContent += fmt.Sprint(dependency.Vendor)
		dependency.CreateTime = time.Now().UnixNano() / 1000000
		dependency.AppId = rasp.AppId
		dependency.RaspId = rasp.Id
		dependency.HostName = rasp.HostName
		dependency.RegisterIp = rasp.RegisterIp
		dependency.Tag = dependency.Vendor + ":" + dependency.Product + ":" + dependency.Version
		dependency.SearchString = dependency.Product + dependency.Version
		doc["upsert_id"] = fmt.Sprintf("%x", md5.Sum([]byte(idContent)))
		doc["content"] = dependency
		docs = append(docs, dependency)
		docsMap = append(docsMap, doc)
	}
	err := logs.AddLogsWithKafka("dependency-data", rasp.AppId, docs)
	if err != nil {
		return err
	}
	return es.BulkInsert(es.GetIndex(AliasDependencyIndexName, rasp.AppId), dependencyType, docsMap)
}

func SearchDependency(appId string, param *SearchDependencyParam) (int64, []map[string]interface{}, error) {
	query, err := getDependencyQuery(param)
	if err != nil {
		return 0, nil, err
	}
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	index := es.GetIndex(AliasDependencyIndexName, appId)
	queryResult, err := es.ElasticClient.Search(index).
		Query(query).
		From((param.Page - 1) * param.Perpage).
		Sort("tag", true).
		Size(param.Perpage).Do(ctx)
	if err != nil {
		if queryResult != nil && queryResult.Error != nil {
			beego.Error(queryResult.Error, index)
		}
		return 0, nil, err
	}
	var total int64
	result := make([]map[string]interface{}, 0, param.Perpage)
	if queryResult != nil && queryResult.Hits != nil && queryResult.Hits.Hits != nil {
		hits := queryResult.Hits.Hits
		total = queryResult.Hits.TotalHits
		result = make([]map[string]interface{}, len(hits))
		for index, item := range hits {
			result[index] = make(map[string]interface{})
			err := json.Unmarshal(*item.Source, &result[index])
			if err != nil {
				return 0, nil, err
			}
			es.HandleSearchResult(result[index], item.Id)
		}
	}
	return total, result, nil
}

func getDependencyAggr(dependencyTopHitName string) (*elastic.TermsAggregation) {
	dependencyMaxAggrName := "dependency_max_aggr"
	dependencyTopHitAggr := elastic.NewTopHitsAggregation().
		Size(1).
		Sort("@timestamp", false).
		DocvalueFields("@timestamp", "path", "hostname", "rasp_id", "product", "search_string",
			"vendor", "app_id", "source", "register_ip", "version")
	dependencyTimeMaxAggr := elastic.NewMaxAggregation().Field("@timestamp")
	return elastic.NewTermsAggregation().
		Field("tag").
		Size(10000).
		Order(dependencyMaxAggrName, false).
		SubAggregation(dependencyMaxAggrName, dependencyTimeMaxAggr).
		SubAggregation(dependencyTopHitName, dependencyTopHitAggr)
}

func AggrDependencyByQuery(appId string, param *SearchDependencyParam) (int64, []map[string]interface{}, error) {
	var dependencyTopHitName = "dependency_top_hit"
	var aggrName = "dependency_aggr"
	query, err := getDependencyQuery(param)
	if err != nil {
		return 0, nil, err
	}
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(15*time.Second))
	defer cancel()
	index := es.GetIndex(AliasDependencyIndexName, appId)
	aggr := getDependencyAggr(dependencyTopHitName)
	if strings.Compare(es.Version[0:1], "5") > 0 {
		aggr.OrderByKeyAsc()
	} else {
		aggr.OrderByTermAsc()
	}
	queryResult, err := es.ElasticClient.Search(index).
		Query(query).
		Size(0).
		Aggregation(aggrName, aggr).
		Do(ctx)
	if err != nil {
		if queryResult != nil && queryResult.Error != nil {
			beego.Error(queryResult.Error, index)
		}
		return 0, nil, err
	}
	var total int64
	result := make([]map[string]interface{}, 0, param.Perpage)
	if queryResult != nil && queryResult.Aggregations != nil {
		if terms, ok := queryResult.Aggregations.Terms(aggrName); ok && terms.Buckets != nil {
			total = int64(len(terms.Buckets))
			result = make([]map[string]interface{}, 0, param.Perpage)
			for i := 0; i < param.Perpage; i++ {
				index := i + (param.Page-1)*param.Perpage
				if index >= int(total) {
					break
				}
				value := make(map[string]interface{})
				item := terms.Buckets[index]
				if topHit, ok := item.TopHits(dependencyTopHitName); ok && topHit.Hits != nil && topHit.Hits.Hits != nil {
					hits := topHit.Hits.Hits
					if len(hits) > 0 {
						err := json.Unmarshal(*hits[0].Source, &value)
						if err != nil {
							return 0, nil, err
						}

						value["rasp_count"] = terms.Buckets[index].DocCount
						es.HandleSearchResult(value, hits[0].Id)
						result = append(result, value)
					}
				}
			}
			return total, result, nil
		}

	}
	return total, result, nil
}

func getDependencyQuery(param *SearchDependencyParam) (query *elastic.BoolQuery, err error) {
	var searchContent map[string]string
	content, err := json.Marshal(param.Data)
	if len(content) > 0 {
		err = json.Unmarshal(content, &searchContent)
		if err == nil {
			query = elastic.NewBoolQuery()
			queries := make([]elastic.Query, 0, len(searchContent)+1)
			for k, v := range searchContent {
				if k == "key_word" {
					shouldQueries := make([]elastic.Query, 2)
					shouldQueries[0] = elastic.NewWildcardQuery("tag", "*"+v+"*")
					shouldQueries[1] = elastic.NewWildcardQuery("search_string", "*"+v+"*")
					query.Must(elastic.NewBoolQuery().Should(shouldQueries...))
				} else if k == "hostname" {
					shouldQueries := make([]elastic.Query, 2)
					shouldQueries[0] = elastic.NewWildcardQuery("hostname", "*"+v+"*")
					shouldQueries[1] = elastic.NewWildcardQuery("register_ip", "*"+v+"*")
					query.Must(elastic.NewBoolQuery().Should(shouldQueries...))
				} else {
					queries = append(queries, elastic.NewTermQuery(k, v))
				}
			}
			query.Filter(queries ...)
		}
	}
	return
}

func RemoveDependencyByRasp(appId string, raspId string) error {
	query := elastic.NewBoolQuery().Filter(elastic.NewTermQuery("rasp_id", raspId))
	return es.DeleteByQuery(es.GetIndex(AliasDependencyIndexName, appId), dependencyType, query)
}

func RemoveDependencyByApp(appId string) error {
	return es.DeleteIndex(es.GetIndex(DependencyIndexName, appId))
}
