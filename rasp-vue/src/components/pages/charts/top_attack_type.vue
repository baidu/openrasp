<template>
  <div style="height: 12rem;">
    <vue-c3 :handler="category_handler" v-show="data.length"></vue-c3>
    <p class="text-center" style="display: flex; justify-content: center; height: 100%; align-items: center; " v-show="! data.length">暂无数据</p>
  </div>
</template>

<script>
import Vue from "vue"
import VueC3 from "vue-c3"
import { attack_types } from "../../../util"

export default {
  name: "top_attack_type",
  data: function () {
    return {
      data: [],
      category_handler: new Vue()
    }
  },
  methods: {
    setData: function (data) {
      this.data = data
      
      const category_chart = {
        size: {
          height: 192
        },
        data: {
          columns: data,
          type: "pie",
          names: attack_types
        },
        axis: {},
        legend: {
          show: false
        },
        padding: {
          bottom: 0,
          top: 0
        }
      }

      this.category_handler.$emit("init", category_chart)
    }
  },
  components: {
    VueC3
  }
}
</script>
