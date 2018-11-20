import Vue from 'vue'
import moment from 'moment'
import { VueLoading } from 'vue-loading-template'
import { api_request } from '../util'

Vue.mixin({
    data: function () {
      return {}
    },
    filters: {
        capitalize: function (value) {
            if (!value) return ''
            value = value.toString()
            return value.charAt(0).toUpperCase() + value.slice(1)
        }
    },
    mounted: function() {  
      $(document).on('click.bs.dropdown.data-api', '.dropdown .keep-open-on-click', (event) => {
        event.stopPropagation();
      });
    },
    methods: {
      moment: function(...a) {
        return moment(...a)
      },
      api_request: api_request
    },
    components: {
      VueLoading
    }
})