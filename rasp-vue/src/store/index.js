import Vue from 'vue'
import Vuex from 'vuex'
import { request } from '@/util'

Vue.use(Vuex)

const store = new Vuex.Store({
  state: {
    app_list: [],
    current_app: {},
    sticky: true
  },
  mutations: {
    setCurrentApp(state, data) {
      this.state.current_app = data
    },
    setAppList(state, data) {
      this.state.app_list = data
    },
    setSticky(state, data) {
      this.state.sticky = data
    }
  },
  actions: {
    loadAppList({ commit }, appId) {
      request.post('v1/api/app/get', {
        page: 1,
        perpage: 100
      }).then(res => {
        const { data } = res
        commit('setAppList', data)
        if (appId) {
          const app = data.find(row => row.id === appId)
          if (app) {
            commit('setCurrentApp', app)
          } else {
            alert('没有这个应用: ' + appId)
            commit('setCurrentApp', data[0])
          }
        } else {
          commit('setCurrentApp', data[0])
        }
      })
    }
  },
  getters: {
    app_list: state => state.app_list,
    current_app: state => state.current_app,
    sticky: state => state.sticky
  }
})

export default store
