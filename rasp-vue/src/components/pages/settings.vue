<template>
	<div class="my-3 my-md-5">
	  <div class="container">
	    <div class="row">
	      <div class="col-lg-3 mb-4">
	        <div class="list-group list-group-transparent mb-0" role="tablist">
	          <a href="#settings-general" data-toggle="tab" class="list-group-item list-group-item-action active">
	            <span class="icon mr-3">
	              <i class="fe fe-settings">
	              </i>
	            </span>
	            通用设置
	          </a>
	          <a href="#settings-alarm" data-toggle="tab" class="list-group-item list-group-item-action">
	            <span class="icon mr-3">
	              <i class="fe fe-alert-triangle">
	              </i>
	            </span>
	            报警设置
	          </a>
	          <a href="#settings-whitelist" data-toggle="tab" class="list-group-item list-group-item-action">
	            <span class="icon mr-3">
	              <i class="fe fe-list">
	              </i>
	            </span>
	            黑白名单
	          </a>
	          <a href="#settings-algorithm" data-toggle="tab" class="list-group-item list-group-item-action">
	            <span class="icon mr-3">
	              <i class="fe fe-filter">
	              </i>
	            </span>
	            防护设置
	          </a>
	          <a href="#settings-auth" data-toggle="tab" class="list-group-item list-group-item-action">
	            <span class="icon mr-3">
	              <i class="fe fe-user">
	              </i>
	            </span>
	            登录认证
	          </a>      
	          <a href="#settings-app" data-toggle="tab" class="list-group-item list-group-item-action">
	            <span class="icon mr-3">
	              <i class="fe fe-server">
	              </i>
	            </span>
	            应用管理
	          </a>                
	        </div>
	      </div>
	      <div class="col-lg-9">
	        <div class="tab-content">
	          <GeneralSettings ref="generalSettings"></GeneralSettings>
	          <AlarmSettings ref="alarmSettings"></AlarmSettings>
	          <WhitelistSettings ref="whitelistSettings"></WhitelistSettings>	          
						<AlgorithmSettings ref="algorithmSettings"></AlgorithmSettings>
	        	<AuthSettings ref="authSettings"></AuthSettings>
	          <AppSettings ref="appSettings"></AppSettings>       
	        </div>
	      </div>
	    </div>
	  </div>
	</div>
	
</template>

<script>
import AlarmSettings from "@/components/pages/settings/alarm"
import AppSettings from "@/components/pages/settings/apps"
import AuthSettings from "@/components/pages/settings/auth"
import GeneralSettings from "@/components/pages/settings/general"
import WhitelistSettings from "@/components/pages/settings/whitelist"
import AlgorithmSettings from "@/components/pages/settings/algorithm"

import { mapGetters } from 'vuex'

export default {
  name: 'settings',
  data: function() {
    return {
      data: {},
      loading: false,
    }
	},
	watch: {
    current_app () {
      this.loadSettings()
    }
  },
  computed: {
    ...mapGetters(['current_app'])
  },
  mounted: function() {
    
	},
  activated: function() {
	  if (this.current_app.id && !this.loading && Object.keys(this.data).length === 0) {      
      this.loadSettings()
    }
  },  	
	methods: {
		loadSettings: function() {
			var self = this
			var body = {
				app_id: this.current_app.id
			}

			self.loading = true

			this.api_request('v1/api/app/get', body, function (data) {
				self.loading = false
				
				self.$refs.generalSettings.setData(data.general_config)
      	self.$refs.whitelistSettings.setData(data.whitelist_config)
				self.$refs.alarmSettings.setData({
					ding_alarm_conf: data.ding_alarm_conf,
					http_alarm_conf: data.http_alarm_conf,
					email_alarm_conf: data.email_alarm_conf,
				})
			})
		}
	},
  components: {
		AlarmSettings,
		WhitelistSettings,
		AppSettings,
		AuthSettings,
		AlgorithmSettings,
		GeneralSettings
  }
}
</script>

