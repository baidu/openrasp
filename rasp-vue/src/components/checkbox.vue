<template>
    <label class="custom-switch m-0">
        <input type="checkbox" v-model="checked" class="custom-switch-input">
        <span class="custom-switch-indicator" @click="toggle()"></span>
    </label>
</template>

<script>
export default {
    name: 'checkbox',
    data: function () {
        return {
            checked: false
        }
    },
    methods: {
        toggle() {
            this.checked = ! this.checked
        },
        is_checked() {
            return this.checked
        }
    }
}
</script>
