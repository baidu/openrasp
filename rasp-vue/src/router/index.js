import Vue from 'vue'
import Router from 'vue-router'
import Cookie from 'js-cookie'
import store from '@/store'

import Login from '@/components/Login'
import dashboard from '@/components/pages/dashboard'
import baseline from '@/components/pages/baseline'
import hosts from '@/components/pages/hosts'
import settings from '@/components/pages/settings'
import plugins from '@/components/pages/plugins'
import audit from '@/components/pages/audit'
import events from '@/components/pages/events'
import vulns from '@/components/pages/vulns'
import exceptions from '@/components/pages/exceptions'
import Layout from '@/views/layout'

Vue.use(Router)

const router = new Router({
  routes: [
    {
      path: '/login',
      name: 'login',
      component: Login
    },
    {
      path: '/',
      beforeEnter(to, from, next) {
        if (!Cookie.get('RASP_AUTH_ID') && process.env.NODE_ENV === 'production') {
          next({ name: 'login' })
        } else {
          store.dispatch('loadAppList', to.params.app_id)
            .then(() => next())
            .catch(err => {
              console.error(err)
              next({ name: '500' })
            })
        }
      },
      component: Layout,
      children: [{
        path: 'dashboard/:app_id',
        name: 'dashboard',
        component: dashboard
      }, {
        path: 'hosts/:app_id/',
        name: 'hosts',
        component: hosts
      }, {
        path: 'audit/:app_id/',
        name: 'audit',
        component: audit
      }, {
        path: 'settings/:setting_tab/:app_id/',
        name: 'settings',
        component: settings
      }, {
        path: 'plugins/:app_id/',
        name: 'plugins',
        component: plugins
      }, {
        path: 'baseline/:app_id/',
        name: 'baseline',
        component: baseline
      }, {
        path: 'events/:app_id/',
        name: 'events',
        component: events
      }, {
        path: 'vulns/:app_id/',
        name: 'vulns',
        component: vulns
      }, {
        path: 'exceptions/:app_id/',
        name: 'exceptions',
        component: exceptions
      }, {
        path: '*',
        redirect: {
          name: 'dashboard'
        }
      }]
    },
    {
      path: '*',
      redirect: {
        name: 'dashboard'
      }
    }
  ],
  linkExactActiveClass: 'active'
})

// router.replace({ path: '*', redirect: '/' })
export default router
