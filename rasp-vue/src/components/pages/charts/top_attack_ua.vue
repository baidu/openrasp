<template>
  <div style="height: 12rem;">
    <VueC3 v-show="data.length" :handler="source_handler" />
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
  name: 'TopAttackUa',
  components: {
    VueC3
  },
  data: function() {
    return {
      source_handler: new Vue(),
      data: []
    }
  },
  methods: {
    setData: function(data) {
      this.data = data

      const source_chart = {
        size: {
          height: 192
        },
        data: {
          columns: this.data,
          type: 'donut',
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
            name: function(name, ratio, id, index) {
              if (name.length > 70) {
                return name.substr(0, 70) + ' ...'
              } else {
                return name
              }
            }
          }
        }
      }

      this.source_handler.$emit('init', source_chart)
    }
  }
}
</script>
