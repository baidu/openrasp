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

