import Vue from "vue"
import Vuex from "vuex"
import { api_request } from "../util"

Vue.use(Vuex)

const store = new Vuex.Store({
  state: {
    app_list: [],
    current_app: {}
  },
  mutations: {
    setCurrentApp(state, data) {
      this.state.current_app = data;
    },
    setAppList(state, data) {
      this.state.app_list = data;
    }
  },
  actions: {
    loadAppList: function({ commit }, appId) {
      api_request("v1/api/app/get", { page: 1, perpage: 100 }, function(
        data
      ) {
        data = data.data
        
        commit("setAppList", data);
        if (appId) {
          let app;
          data.some (function (row) {
            if (row.id == appId) {
              app = row            
              return true
            }
          })
          if (app) {
            commit("setCurrentApp", app)
          } else {
            alert('没有这个应用: ' + appId);
            commit("setCurrentApp", data[0]);
          }
        } else {
          commit("setCurrentApp", data[0]);
        }
      })
    }
  },
  getters: {
    app_list: state => state.app_list,
    current_app: state => state.current_app,
  }
})

export default store;
