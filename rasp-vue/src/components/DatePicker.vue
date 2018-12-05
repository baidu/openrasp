<template>
  <input type="text" name="dates" class="form-control pull-right">
</template>

<script>
import 'daterangepicker'
import moment from 'moment'

export default {
  name: 'DatePicker',
  data: function() {
    return {
      start: moment().subtract(1, 'months').startOf('day'),
      end: moment().endOf('day')
    }
  },
  mounted: function() {
    var self = this

    this.$nextTick(function() {
      window.$(this.$el).daterangepicker({
        startDate: self.start,
        endDate: self.end
      }, function(start, end, label) {
        self.start = start
        self.end = end

        self.$emit('selected')
      })
    })
  }
}
</script>
