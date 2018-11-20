<template>
  <div style="height: 12rem;">
    <vue-c3 :handler="source_handler" v-show="data.length"></vue-c3>
    <p class="text-center" style="display: flex; justify-content: center; height: 100%; align-items: center; " v-show="! data.length">暂无数据</p>
  </div>
</template>

<script>
import Vue from "vue"
import VueC3 from "vue-c3"
import { attack_types } from "../../../util"

export default {
  name: "top_attack_ua",
  data: function() {
    return {
      source_handler: new Vue(),
      data: []
    }
  },
  methods: {
    setData: function(data) {
      this.data = data
      
      let source_chart = {
        size: {
          height: 192
        },
        data: {
          columns: this.data,
          type: "donut",
          names: attack_types
        },
        axis: {
          x: {
            tick: {
              width: 200
            }
          }
        },
        legend: {
          show: false
        },
        padding: {
          bottom: 0,
          top: 0
        },
        tooltip: {
          format: {
            name: function (name, ratio, id, index) { 
              if (name.length > 70) {
                return name.substr(0, 70) + ' ...'
              } else {
                return name
              }
            }
          }
        }
      }

      this.source_handler.$emit("init", source_chart)
    }
  },
  components: {
    VueC3
  }
}
</script>
