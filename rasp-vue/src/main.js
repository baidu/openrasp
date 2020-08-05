// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import App from './App'
import router from './router'
import axios from 'axios'
import VueClipboard from 'vue-clipboard2'
import BootstrapVue from 'bootstrap-vue'
Vue.use(BootstrapVue)
import 'bootstrap'
import 'bootstrap/dist/css/bootstrap.css'
import 'bootstrap-vue/dist/bootstrap-vue.css'
import '@/assets/css/dashboard.css'
import '@/assets/css/daterangepicker.css'
import '@/assets/css/custom.css'

import 'expose-loader?bootbox!bootbox'

window.$ = window.jQuery = require('jquery')

Vue.config.productionTip = false

Vue.use(VueClipboard);

import './mixins'
import store from './store'

Vue.prototype.$http = axios

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  store,
  render: h => h(App)
})
