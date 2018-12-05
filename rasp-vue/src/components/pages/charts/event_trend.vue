<template>
  <VueC3 :handler="trend_handler" style="height: 15rem;" />
</template>

<script>
import Vue from 'vue'
import VueC3 from 'vue-c3'
import { attack_types } from '../../../util'

export default {
  name: 'EventTrend',
  components: {
    VueC3
  },
  data: function() {
    return {
      trend_handler: new Vue()
    }
  },
  methods: {
    setData: function(data) {
      var self = this

      data.data[0].unshift('拦截请求')
      data.data[1].unshift('记录日志')

      const trend_chart = {
        size: {
          height: 250
        },
        data: {
          columns: data.data,
          type: 'area',
          groups: [['data1', 'data2']],
          colors: {
            data1: '#467fcf',
            data2: '#5eba00'
          },
          names: {
            data1: '拦截数量',
            data2: '日志数量'
          }
        },
        axis: {
          y: {
            padding: {
              bottom: 0
            },
            show: false,
            tick: {
              outer: false
            }
          },
          x: {
            padding: {
              left: 0,
              right: 0
            },
            show: false
          }
        },
        legend: {
          position: 'inset',
          padding: 0,
          inset: {
            anchor: 'top-left',
            x: 20,
            y: 8,
            step: 10
          }
        },
        tooltip: {
          format: {
            title: function(x) {
              return self.moment(data.labels[x]).format('YYYY-MM-DD')
            }
          }
        },
        padding: {
          bottom: 0,
          left: -1,
          right: -1
        },
        point: {
          show: false
        }
      }
      this.trend_handler.$emit('init', trend_chart)
    }
  }
}
</script>
