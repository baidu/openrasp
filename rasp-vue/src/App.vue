<template>
  <div class="page">
    <div class="page-main" v-if="is_authed">
      <Navigation></Navigation>
      <keep-alive>
        <router-view></router-view>
      </keep-alive>
    </div>
    <Login v-if="! is_authed"></Login>
    <Footer v-if="is_authed"></Footer>
  </div>
</template>

<style lang="scss">
  @import './assets/css/dashboard.css';
  @import './assets/css/daterangepicker.css';
  @import './assets/css/custom.css';
</style>

<script>
import Navigation from "@/components/Navigation"
import Footer from "@/components/Footer"
import Login from "@/components/Login"
import router from "./router"
import { mapGetters } from "vuex"

export default {
  name: "App",
  mounted: function() {},
  methods: {},
  computed: {
    ...mapGetters(["is_authed"])
  },
  components: {
    Navigation,
    Footer,
    Login
  }
}
</script>

