<template>
  <div class="custom-file">
    <input type="file" class="custom-file-input" ref="fileInput" @change="handleFileChange($event.target.name, $event.target.files)">
    <label class="custom-file-label" v-if="file">{{ file.name }}</label>
    <label class="custom-file-label" v-if="! file">选择插件</label>
  </div>
</template>

<script>

export default {
  name: 'FileUpload',
  data: function () {
    return {
      file: undefined
    }
  },
  mounted: function () {

  },
  methods: {
    clear: function() {
      this.$refs.fileInput.value = ''
      this.file = undefined
    },
    handleFileChange(name, files) {
      // console.log ('got', files)
      
      this.file = files[0]
      this.$emit('input', files[0])
    }
  },
  components: {

  }
}
</script>
