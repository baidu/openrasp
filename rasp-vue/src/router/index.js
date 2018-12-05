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
      path: '/dashboard/:app_id',
      component: Layout,
      children: [{
        path: '',
        name: 'dashboard',
        component: dashboard
      }]
    },
    {
      path: '/hosts/:app_id/',
      component: Layout,
      children: [{
        path: '',
        name: 'hosts',
        component: hosts
      }]
    },
    {
      path: '/audit/:app_id/',
      name: 'audit',
      component: Layout,
      children: [{
        path: '',
        name: 'audit',
        component: audit
      }]
    },
    {
      path: '/settings/:app_id/',
      component: Layout,
      children: [{
        path: '',
        name: 'settings',
        component: settings
      }]
    },
    {
      path: '/plugins/:app_id/',
      component: Layout,
      children: [{
        path: '',
        name: 'plugins',
        component: plugins
      }]
    },
    {
      path: '/baseline/:app_id/',
      component: Layout,
      children: [{
        path: '',
        name: 'baseline',
        component: baseline
      }]
    },
    {
      path: '/events/:app_id/',
      component: Layout,
      children: [{
        path: '',
        name: 'events',
        component: events
      }]
    }
  ],
  linkExactActiveClass: 'active'
})

// router.replace({ path: '*', redirect: '/' })
export default router