package agent

import (
	"rasp-cloud/conf"
	"rasp-cloud/controllers"
	"net/http"
	"rasp-cloud/models"
	"crypto/md5"
	"fmt"
	"github.com/astaxie/beego"
	"strings"
)

// Operations about plugin
type DependencyController struct {
	controllers.BaseController
}

type dependencyParam struct {
	RaspId     string                 `json:"rasp_id"`
	Dependency []*dependencyParamItem `json:"dependency"`
}

type dependencyParamItem struct {
	Path    string `json:"path"`
	Vendor  string `json:"vendor"`
	Product string `json:"product"`
	Version string `json:"version"`
	Source  string `json:"source"`
}

// @router / [post]
func (o *DependencyController) Post() {
	var param dependencyParam
	o.UnmarshalJson(&param)
	if param.RaspId == "" {
		o.ServeError(http.StatusBadRequest, "rasp_id can not be empty")
	}
	rasp, err := models.GetRaspById(param.RaspId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get rasp", err)
	}
	if len(param.Dependency) > 0 {
		dependencyMap := make(map[string]*models.Dependency)
		dependencies := make([]*models.Dependency, 0, len(param.Dependency))
		for _, item := range param.Dependency {
			if isValid, errMsg := o.checkDependencyParamItem(item); !isValid {
				if !conf.AppConfig.DebugModeEnable {
						if len(strings.Split(strings.ToLower(errMsg), "empty")) > 1 {
							 continue
						}
				}
				beego.Error("failed to add dependency for rasp: "+rasp.Id, errMsg)
				continue
			}
			md5Value := fmt.Sprintf("%x",
				md5.Sum([]byte(item.Vendor+":"+item.Product+":"+item.Version+":"+item.Source)))
			if value, ok := dependencyMap[md5Value]; ok {
				value.Path = append(value.Path, item.Path)
			} else {
				model := &models.Dependency{
					Path:    []string{item.Path},
					Version: item.Version,
					Product: item.Product,
					Vendor:  item.Vendor,
					Source:  item.Source,
				}
				dependencyMap[md5Value] = model
				dependencies = append(dependencies, model)
			}
		}
		err := models.RemoveDependencyByRasp(rasp.AppId, rasp.Id)
		if err != nil {
			o.ServeError(http.StatusBadRequest, "failed to clear dependencies for this rasp", err)
		}
		if len(dependencies) > 0 {
			err = models.AddDependency(rasp, dependencies)
			if err != nil {
				o.ServeError(http.StatusBadRequest, "failed to add dependency data to ES", err)
			}
		}
	}
	o.ServeWithEmptyData()
}

func (o *DependencyController) checkDependencyParamItem(item *dependencyParamItem) (bool, string) {
	if item.Version == "" {
		return false, "the version of dependency can not be empty"
	}
	if item.Product == "" {
		return false, "the product of dependency can not be empty"
	}
	if item.Vendor == "" {
		return false, "the vendor of dependency can not be empty"
	}
	if item.Path == "" {
		return false, "the path of dependency can not be empty"
	}
	if len(item.Version) > 256 {
		return false, "the length of version can not be greater than 256"
	}
	if len(item.Product) > 256 {
		return false, "the length of product can not be greater than 256"
	}
	if len(item.Vendor) > 256 {
		return false, "the length of vendor can not be greater than 256"
	}
	if len(item.Path) > 1024 {
		return false, "the length of path can not be greater than 1024"
	}
	if len(item.Source) > 1024 {
		return false, "the length of source can not be greater than 1024"
	}
	return true, ""
}
