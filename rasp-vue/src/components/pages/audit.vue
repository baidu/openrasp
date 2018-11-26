<template>
	<div class="my-3 my-md-5">
		<div class="container">
			<div class="page-header">
				<h1 class="page-title">
					操作日志
				</h1>
				<div class="page-options d-flex">
					<div class="input-icon ml-2">
						<span class="input-icon-addon">
							<i class="fe fe-calendar">
							</i>
						</span>
						<DatePicker ref="datePicker" v-on:selected="loadAudit(1)"></DatePicker>
					</div>					
				</div>
				<button class="btn btn-primary ml-2" @click="loadAudit(1)">搜索</button>
			</div>
			<div class="card">
				<div class="card-body">
					<vue-loading v-if="loading" type="spiningDubbles" color="rgb(90, 193, 221)" :size="{ width: '50px', height: '50px' }"></vue-loading>

					<table class="table table-bordered" v-if="! loading">
						<thead>
							<tr>
								<th>操作时间</th>
								<th>操作内容</th>
								<th>操作人</th>
								<th>IP 地址</th>
							</tr>
						</thead>
						<tbody>
							<tr v-for="row in data" :key="row.id">
								<td nowrap>
									{{ moment(row.event_time).format('YYYY-MM-DD HH:mm:ss') }}
								</td>
								<td>{{ row.content }}</td>
								<td nowrap>{{ row.user.length ? row.user : '-' }}</td>
								<td nowrap>{{ row.ip }}</td>
							</tr>
						</tbody>
					</table>
					<nav v-if="! loading">
						<b-pagination align="center" :total-rows="total" v-model="currentPage" :per-page="10">
						</b-pagination>
					</nav>
				</div>
			</div>
		</div>
	</div>
</template>

<script>
import DatePicker from "@/components/DatePicker"
import { mapGetters } from "vuex"

export default {
  name: "audit",
  data: function() {
    return {
      data: [],
      loading: false,
      currentPage: 1,
      srcip: "",
      total: 0
    }
  },
  watch: {
    currentPage: function(newVal, oldVal) {
      this.loadAudit(newVal)
    },
    current_app() {
      this.loadAudit(1)
    }
  },
  mounted: function() {},
  computed: {
    ...mapGetters(["current_app"])
  },
  activated: function() {
    if (!this.loading && !this.data.length) {
      this.loading = true
      this.loadAudit(1)
    }
  },
  methods: {
    loadAudit: function(page) {
      var self = this
      var body = {
        page: page,
		perpage: 10,
		start_time: this.$refs.datePicker.start.unix() * 1000,
        end_time: this.$refs.datePicker.end.unix() * 1000,
        data: {
          
        }
      }

      self.loading = true
      self.api_request("v1/api/operation/search", body, function(data) {
        self.data = data.data
        self.total = data.total
        self.loading = false
      })
    }
  },
  components: {
	  DatePicker
  }
}
</script>

