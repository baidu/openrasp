<template>
  <div class="page">
    <div class="page-main">
      <Navigation></Navigation>
      <keep-alive>
        <router-view></router-view>
      </keep-alive>
    </div>
    <Footer></Footer>
  </div>
</template>

<script>
import Navigation from '@/components/Navigation.vue'
import Footer from '@/components/Footer.vue'
import router from './router'

export default {
  name: 'App',
  mounted: function () {
  
  },
  methods: {

  },
  components: {
    Navigation,
    Footer
  }
}
</script>

