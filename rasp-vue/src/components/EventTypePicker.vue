<template>
  <div class="dropdown ml-2">
    <button type="button" class="btn btn-secondary dropdown-toggle" data-toggle="dropdown">
      攻击类型
    </button>
    <ul class="dropdown-menu dropdown-menu-right dropdown-menu-arrow keep-open-on-click" style="width: 440px; padding: 10px; ">
      <div class="row">
        <div class="col-6">
          <label class="custom-switch">
            <input type="checkbox" v-model="all_checked" checked="all_checked" @change="toggle_all()" class="custom-switch-input" />
            <span class="custom-switch-indicator">
            </span>
            <span class="custom-switch-description">
              全选
            </span>
          </label>
        </div>
      </div>
      <div class="row">
        <div class="col-6" v-for="c in categories" :key="c.id">
          <label class="custom-switch">
            <input type="checkbox" v-model="c.checked" checked="c.checked" @change="$emit('selected')" class="custom-switch-input" />
            <span class="custom-switch-indicator">
            </span>
            <span class="custom-switch-description">
              {{ c.name }}
            </span>
          </label>
        </div>
      </div>

      <!-- <li v-for="c in categories" :key="c.id">
        <div class="checkbox" style="padding: 10px 10px 0 10px; width: 100%; ">
          <label>
            <input type="checkbox" style="margin-right: 5px; " v-model="c.checked" @change="$emit('selected')" />
            {{ c.name }}

          </label>
        </div>
      </li> -->
    </ul>
  </div>
</template>

<script>
import { attack_types } from "../util"

export default {
  name: "EventTypePicker",
  data: function() {
    return {
      categories: [],
      all_checked: true
    }
  },
  methods: {
    toggle_all: function () {
      this.categories.forEach((row) => {
        row.checked = this.all_checked
      })
      this.$emit('selected')
    },
    selected: function() {
      var types = []
      this.categories.forEach((row) => {
        if (row.checked) {
          types.push(row.id)
        }
      })

      return types
    }
  },
  mounted: function() {
    var data = []
    var self = this

    Object.keys(attack_types).forEach(function(key) {
      
      // FIXME: 目前PHP几个webshell_XXX没有统一成 webshell 类型，
      // 所以 webshell 这个类型没必要显示出来
      if (key == 'webshell') {
        return
      }

      data.push({
        name: attack_types[key],
        id: key,
        checked: true
      })
    })

    this.categories = data
  }
}
</script>
