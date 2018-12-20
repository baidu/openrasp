<template>
  <div style="height: 12rem;">
    <VueC3 v-show="data.length" :handler="category_handler" />
    <p v-show="! data.length" class="text-center" style="display: flex; justify-content: center; height: 100%; align-items: center; ">
      暂无数据
    </p>
  </div>
</template>

<script>
import Vue from 'vue'
import VueC3 from 'vue-c3'
import { attack_types } from '../../../util'

export default {
  name: 'TopAttackType',
  components: {
    VueC3
  },
  data: function() {
    return {
      data: [],
      category_handler: new Vue()
    }
  },
  methods: {
    setData: function(data) {
      this.data = data

      const category_chart = {
        size: {
          height: 192
        },
        data: {
          columns: data,
          type: 'pie',
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

      this.category_handler.$emit('init', category_chart)
    }
  }
}
</script>
