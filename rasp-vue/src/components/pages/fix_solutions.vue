<template>
  <div class="event-detail">
    <div v-if="data.attack_type">{{ data.attack_type | getSolutionText }}</div>
  </div>
</template>

<script>
import {Solutions} from "@/util/solutions"

export default {
  name: 'fix_solutions',
  filters: {
    getSolutionText(type) {
      return Solutions[type] || ''
    }
  },
  data: function () {
    return {
      data: {
        attack_params: {}
      }
    }
  },
  methods: {
    setData: function (data) {
      this.data = data
    }
  }
}
</script>

