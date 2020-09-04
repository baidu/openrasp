<template>
  <footer class="footer">
    <div class="container">
      <div class="row align-items-center flex-row-reverse">
        <div class="col-auto ml-lg-auto">
          <div class="row align-items-center">
            <div class="col-auto">
              <ul class="list-inline list-inline-dots mb-0">
                <li class="list-inline-item"><a href="https://rasp.baidu.com/" target="_blank">官方网站</a></li>
                <li class="list-inline-item"><a href="https://rasp.baidu.com/#section-books" target="_blank">最佳实践</a></li>
              </ul>
            </div>
          </div>
        </div>
        <div class="col-12 col-lg-auto mt-3 mt-lg-0 text-center">
          Copyright © 2017-2020 Baidu, Inc. {{ cloud.version }} ({{ cloud.commit_id.substr(0, 8) }})，编译时间 {{ cloud.build_time}}
        </div>
      </div>
    </div>
  </footer>
</template>

<script>
import { rasp_version, rasp_commit_id, rasp_build_time } from '@/util/version'

export default {
  name: 'Footer',
  data: function() {
    return {
      cloud: {
        version: '获取中',
        commit_id: '',
        build_time: ''
      },
      fe: {
        version: rasp_version,
        commit_id: rasp_commit_id,
        build_time: rasp_build_time
      }
    }
  },
  methods: {
    getVersion() {
      this.request.post('v1/version', {}).then(res => {
        this.cloud = res
      })
    }
  },
  mounted() {
    this.getVersion()
  }
}
</script>
