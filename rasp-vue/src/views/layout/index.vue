<template>
  <div id="main" class="page">
    <div style="min-height: calc(100vh - 67px)">
      <Navigation />
      <RouterView style="min-height: 100%; margin-bottom: -50px;" />
    </div>
    <Footer style="height: 67px" />
  </div>
</template>

<script>
import Navigation from '@/components/Navigation'
import Footer from '@/components/Footer'
export default {
  name: 'Main',
  components: {
    Navigation,
    Footer
  }
}
</script>

