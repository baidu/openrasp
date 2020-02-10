package controllers

import "rasp-cloud/environment"

type GeneralController struct {
	BaseController
}

// @router /version [post]
func (o *GeneralController) Version() {
	result := make(map[string]interface{})
	result["version"] = environment.Version
	o.Serve(result)
}