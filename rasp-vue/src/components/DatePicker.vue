<template>
  <input ref="daterangepicker" type="text" name="daterange" class="form-control" autocomplete="off" style="width: 210px; ">
</template>

<script>
import 'daterangepicker'
import moment from 'moment'

export default {
  name: 'DatePicker',
  data() {
    return {
      start: moment().subtract(1, 'months').startOf('day'),
      end: moment().endOf('day')
    }
  },
  mounted() {
    window.$(this.$refs.daterangepicker).daterangepicker({
      startDate: this.start,
      endDate: this.end
    }, (start, end, label) => {
      this.start = start
      this.end = end

      this.$emit('selected')
      this.$emit('change', { start, end })
    })
  }
}
</script>
