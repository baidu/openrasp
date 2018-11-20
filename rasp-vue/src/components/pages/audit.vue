<template>
	<div class="my-3 my-md-5">
		<div class="container">
			<div class="page-header">
				<h1 class="page-title">
					操作审计
				</h1>
				<div class="page-options d-flex">

				</div>
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
							<tr>
								<td>2018-10-26 10:11:05</td>
								<td>修改报警设置</td>
								<td>admin</td>
								<td>192.168.1.1</td>
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
import { mapGetters } from 'vuex'

export default {
	name: 'audit',
	data: function () {
		return {
      data: [],
      loading: false,
      currentPage: 1,
      srcip: "",
      total: 0
		}
	},
  watch: {
    currentPage: function (newVal, oldVal) {
      this.loadAudit(newVal)
    },
    current_app() {
      this.loadAudit(1);
    }
  },	
	mounted: function () {

	},
	methods: {
		loadAudit: function (page) {

		}
	},
	components: {

	}
}
</script>

