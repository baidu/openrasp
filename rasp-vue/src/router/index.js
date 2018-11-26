import Vue from 'vue'
import Router from 'vue-router'

import Login from '@/components/Login'
import dashboard from '@/components/pages/dashboard'
import baseline from '@/components/pages/baseline'
import hosts from '@/components/pages/hosts'
import settings from '@/components/pages/settings'
import plugins from '@/components/pages/plugins'
import audit from '@/components/pages/audit'
import events from '@/components/pages/events'

Vue.use(Router)

const router = new Router({
  routes: [
    // {
    //   path: '/login/',
    //   name: 'login',
    //   component: Login
    // },
    {
      path: '/dashboard/:app_id/',
      name: 'dashboard',
      component: dashboard
    },
    {
      path: '/hosts/:app_id/',
      name: 'hosts',
      component: hosts
    },
    {
      path: '/audit/:app_id/',
      name: 'audit',
      component: audit
    },
    {
      path: '/settings/:app_id/',
      name: 'settings',
      component: settings
    },
    {
      path: '/plugins/:app_id/',
      name: 'plugins',
      component: plugins
    },
    {
      path: '/baseline/:app_id/',
      name: 'baseline',
      component: baseline
    },         
    {
      path: '/events/:app_id/',
      name: 'events',
      component: events
    }        
  ],
  linkExactActiveClass: "active",
})

// router.replace({ path: '*', redirect: '/' })
export default router
