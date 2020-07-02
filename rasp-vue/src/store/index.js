import Vue from 'vue'
import Vuex from 'vuex'
import { request } from '@/util'

Vue.use(Vuex)

const store = new Vuex.Store({
  state: {
    app_list: [],
    current_app: {},
    app_count: 0,
    sticky: true
  },
  mutations: {
    setCurrentApp(state, data) {
      this.state.current_app = data
    },
    setAppList(state, data) {
      this.state.app_list = data
    },
    setAppListCount(state, data) {
      this.state.app_count = data
    },
    setSticky(state, data) {
      this.state.sticky = data
    }
  },
  actions: {
    loadAppList({ commit }, appId) {
      request.post('v1/api/app/get', {
        page: 1,
        perpage: 50
      }).then(res => {
        const { data, total } = res
        commit('setAppList', data)
        commit('setAppListCount', total)
        if (appId) {
          const app = data.find(row => row.id === appId)
          if (!app) {
            request.post('v1/api/app/get', {
              app_id: appId
            }).then(res => {
              if (res) {
                commit('setCurrentApp', res)
              } else {
                alert('没有这个应用: ' + appId)
                commit('setCurrentApp', data[0])
              }
            })
          } else {
            commit('setCurrentApp', app)
          }
        } else {
          commit('setCurrentApp', data[0])
        }
      })
    }
  },
  getters: {
    app_list: state => state.app_list,
    app_count: state => state.app_count,
    current_app: state => state.current_app,
    sticky: state => state.sticky
  }
})

export default store
