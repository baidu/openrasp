// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import App from './App'
import router from './router'
import axios from 'axios'
import BootstrapVue from 'bootstrap-vue'

import bootstrap       from 'bootstrap'
import daterangepicker from 'daterangepicker'

import 'expose-loader?bootbox!bootbox'

window.$ = window.jQuery = require('jquery')

Vue.use(BootstrapVue)
Vue.config.productionTip = false

import './mixins'
import store from './store'

Vue.mixin({
  store: store
})

Vue.prototype.$http = axios

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  components: { App },
  template: '<App/>'
})
