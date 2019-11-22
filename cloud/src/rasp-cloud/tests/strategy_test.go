package test

import (
	"testing"
	"errors"
	"github.com/bouk/monkey"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	"rasp-cloud/models"
)

func getValidStrategy() map[string]interface{}{
	return map[string]interface{}{
		"app_id":            start.TestApp.Id,
		"name":              "unittest策略",
		"description":       "单测",
	}
}

func getDeleteStrategy() (*models.Strategy){
	return &models.Strategy{
		Id:         start.TestRasp.StrategyId,
		AppId:      start.TestApp.Id,
	}
}

func TestSearchStrategy(t *testing.T) {
	Convey("Subject: Test Search Strategy Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/search", inits.GetJson(map[string]interface{}{
				"data": map[string]interface{}{
					"app_id": start.TestApp.Id,
				},
				"page":1,
				"perpage":10,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when search data can not be empty", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/search", inits.GetJson(map[string]interface{}{
				"page":1,
				"perpage":10,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when failed to get strategy", func() {
			monkey.Patch(models.FindStrategy,
				func(selector *models.Strategy, page int, perpage int) (
					count int, result []*models.Strategy, err error) {
						return 0, nil, errors.New("")
				},
			)
			defer monkey.Unpatch(models.FindStrategy)
			r := inits.GetResponse("POST", "/v1/api/strategy/search", inits.GetJson(map[string]interface{}{
				"data": map[string]interface{}{
					"app_id": start.TestApp.Id,
				},
				"page":1,
				"perpage":10,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestNewStrategy(t *testing.T) {
	Convey("Subject: Test New Strategy Api\n", t, func() {
		Convey("when the param is valid", func() {
			strategy := getValidStrategy()
			r := inits.GetResponse("POST", "/v1/api/strategy", inits.GetJson(strategy))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id cannot be empty", func() {
			strategy := getValidStrategy()
			strategy["app_id"] = ""
			r := inits.GetResponse("POST", "/v1/api/strategy", inits.GetJson(strategy))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when strategy name cannot be empty", func() {
			strategy := getValidStrategy()
			strategy["name"] = ""
			r := inits.GetResponse("POST", "/v1/api/strategy", inits.GetJson(strategy))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of strategy name cannot be greater than 64", func() {
			strategy := getValidStrategy()
			strategy["name"] = inits.GetLongString(65)
			r := inits.GetResponse("POST", "/v1/api/strategy", inits.GetJson(strategy))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of the description can not be greater than 1024", func() {
			strategy := getValidStrategy()
			strategy["description"] = inits.GetLongString(1025)
			r := inits.GetResponse("POST", "/v1/api/strategy", inits.GetJson(strategy))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when create strategy failed", func() {
			monkey.Patch(models.AddStratety,
				func(selector *models.Strategy) (result *models.Strategy, err error) {
					return nil, errors.New("")
				},
			)
			defer monkey.Unpatch(models.AddStratety)
			strategy := getValidStrategy()
			r := inits.GetResponse("POST", "/v1/api/strategy", inits.GetJson(strategy))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestConfigStrategy(t *testing.T) {
	Convey("Subject: Test Config Strategy Api\n", t, func() {
		Convey("when the param is valid", func() {
			monkey.Patch(models.GetStrategyById,
				func(id string, appId string) (result *models.Strategy, err error) {
					return nil, nil
				},
			)
			defer monkey.Unpatch(models.GetStrategyById)
			monkey.Patch(models.UpdateStrategyById,
				func(strategyId string, appId string, doc interface{}) (strategy *models.Strategy, err error) {
					return getDeleteStrategy(), nil
				},
			)
			defer monkey.Unpatch(models.UpdateStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"name":        "rename",
				"description": "haha",
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when strategy_id can not be empty", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": "",
				"app_id":      start.TestApp.Id,
				"name":        "rename",
				"description": "haha",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id cannot be empty", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      "",
				"name":        "rename",
				"description": "haha",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when strategy name cannot be empty", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"name":        "",
				"description": "haha",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of strategy name cannot be greater than 64", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"name":        inits.GetLongString(65),
				"description": "haha",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of the description can not be greater than 1024", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"name":        "rename",
				"description": inits.GetLongString(1025),
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when failed to get strategy", func() {
			monkey.Patch(models.GetStrategyById,
				func(id string, appId string) (result *models.Strategy, err error) {
					return nil, errors.New("")
				},
			)
			defer monkey.Unpatch(models.GetStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"name":        "rename",
				"description": "haha",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when failed to update strategy config", func() {
			monkey.Patch(models.UpdateStrategyById,
				func(strategyId string, appId string, doc interface{}) (result *models.Strategy, err error) {
					return nil, errors.New("")
				},
			)
			defer monkey.Unpatch(models.UpdateStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/config", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"name":        "rename",
				"description": "haha",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestSelectStrategy(t *testing.T) {
	Convey("Subject: Test Select Strategy Api\n", t, func() {
		Convey("when the param is valid", func() {
			monkey.Patch(models.SelectStratety,
				func(strategyId string, appId string, raspId []string) (exist bool, err error) {
					return true, nil
				},
			)
			defer monkey.Unpatch(models.SelectStratety)
			r := inits.GetResponse("POST", "/v1/api/strategy/select", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"rasp_id":     []string{start.TestRasp.Id},
				"app_id":      start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when strategy_id can not be empty", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/select", inits.GetJson(map[string]interface{}{
				"strategy_id": "",
				"app_id":      start.TestApp.Id,
				"rasp_id":     []string{start.TestRasp.Id},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id cannot be empty", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/select", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      "",
				"rasp_id":     []string{start.TestRasp.Id},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of rasp_id should be greater than 0", func() {
			r := inits.GetResponse("POST", "/v1/api/strategy/select", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
				"rasp_id":     inits.GetLongStringArray(0),
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when post strategy failed", func() {
			monkey.Patch(models.SelectStratety,
				func(strategyId string, appId string, raspId []string) (exist bool, err error) {
					return false, errors.New("")
				},
			)
			defer monkey.Unpatch(models.SelectStratety)
			r := inits.GetResponse("POST", "/v1/api/strategy/select", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"rasp_id":     []string{start.TestRasp.Id},
				"app_id":      start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the strategy does not exist in this app", func() {
			monkey.Patch(models.SelectStratety,
				func(strategyId string, appId string, raspId []string) (exist bool, err error) {
					return false, nil
				},
			)
			defer monkey.Unpatch(models.SelectStratety)
			r := inits.GetResponse("POST", "/v1/api/strategy/select", inits.GetJson(map[string]interface{}{
				"strategy_id": start.TestRasp.StrategyId,
				"rasp_id":     []string{start.TestRasp.Id},
				"app_id":      start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestDeleteStrategy(t *testing.T) {
	Convey("Subject: Test general csv Api\n", t, func() {
		Convey("when the param is valid", func() {
			monkey.Patch(models.RemoveStrategyById,
				func(StrategyId string) (strategy *models.Strategy, err error) {
					return getDeleteStrategy(), nil
				},
			)
			defer monkey.Unpatch(models.RemoveStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/delete", inits.GetJson(map[string]interface{}{
				"id":          start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})
		Convey("when the id cannot be empty", func() {
			monkey.Patch(models.RemoveStrategyById,
				func(StrategyId string) (strategy *models.Strategy, err error) {
					return getDeleteStrategy(), nil
				},
			)
			defer monkey.Unpatch(models.RemoveStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/delete", inits.GetJson(map[string]interface{}{
				"id":        "",
				"app_id":    start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
		Convey("when the app_id cannot be empty", func() {
			monkey.Patch(models.RemoveStrategyById,
				func(StrategyId string) (strategy *models.Strategy, err error) {
					return getDeleteStrategy(), nil
				},
			)
			defer monkey.Unpatch(models.RemoveStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/delete", inits.GetJson(map[string]interface{}{
				"id":          start.TestRasp.StrategyId,
				"app_id":      "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
		Convey("when failed to remove this strategy", func() {
			monkey.Patch(models.RemoveStrategyById,
				func(StrategyId string) (strategy *models.Strategy, err error) {
					return nil, errors.New("")
				},
			)
			defer monkey.Unpatch(models.RemoveStrategyById)
			r := inits.GetResponse("POST", "/v1/api/strategy/delete", inits.GetJson(map[string]interface{}{
				"id":          start.TestRasp.StrategyId,
				"app_id":      start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}
