package controllers

import (
	"rasp-cloud/environment"
	"rasp-cloud/tools"
)

type GeneralController struct {
	BaseController
}

// @router /version [post]
func (o *GeneralController) Version() {
	result := make(map[string]interface{})
	result["version"] = environment.Version
	result["build_time"] = tools.BuildTime
	result["commit_id"] = tools.CommitID
	o.Serve(result)
}
