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
            <div class="col-auto">
              <a href="https://github.com/baidu/openrasp" class="btn btn-outline-primary btn-sm">源代码</a>
            </div>
          </div>
        </div>
        <div class="col-12 col-lg-auto mt-3 mt-lg-0 text-center">
          Copyright © 2017-2019 Baidu, Inc. 当前版本 {{rasp_version}}
        </div>
      </div>
    </div>
  </footer>
</template>

<script>
import { rasp_version } from '@/util'

export default {
  name: 'Footer',
  data: function() {
    return {
      rasp_version: rasp_version
    }
  }
}
</script>
