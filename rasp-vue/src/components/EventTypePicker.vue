<template>
  <div class="dropdown ml-2">
    <button type="button" class="btn btn-secondary dropdown-toggle" data-toggle="dropdown">
      攻击类型
    </button>
    <ul class="dropdown-menu dropdown-menu-arrow keep-open-on-click">
      <li v-for="c in categories" :key="c.id">
        <div class="checkbox" style="padding: 10px 10px 0 10px; width: 100%; ">
          <label>
            <input type="checkbox" style="margin-right: 5px; " v-model="c.checked" @change="$emit('selected')" />
            {{ c.name }}
          </label>
        </div>
      </li>
    </ul>
  </div>
</template>

<script>
import { attack_types } from '../util'

export default {
  name: "EventTypePicker",
  data: function () {
    return {
      categories: []
    }
  },
  methods: {
    selected: function () {
      var types = [];
      this.categories.forEach(function (row) {
        if (row.checked) {
          types.push(row.id);
        }
      });

      return types;
    }
  },
  mounted: function () {
    var data = []
    var self = this

    Object.keys(attack_types).forEach(function (key) {
      data.push({
        name: attack_types[key],
        id: key,
        checked: true
      })
    })

    this.categories = data
  }
};
</script>
