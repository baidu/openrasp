<template>
  <div class="my-3 my-md-5">
    <div class="container">
      <div class="page-header" style="flex-direction: column; display: flex;">
        <div
          style="
            display: flex;
            flex-direction: row;
            justify-content: flex-start;
            width: 100%;
          "
        >
          <h1 class="page-title" style="padding-top: 3px;">
            崩溃信息
          </h1>
          <div class="page-options d-flex" style="margin-top: 5px;">
            <!-- <b-dropdown text="应用语言" class="ml-4" right>
              <b-container style="width: 250px;">
                <b-form-row>
                  <template v-for="(row, index) in language_list">
                    <b-col :key="index">
                      <label class="custom-switch">
                        <input
                          v-model="selected_language"
                          type="checkbox"
                          class="custom-switch-input"
                          :value="row.value"
                        />
                        <span class="custom-switch-indicator" />
                        <span class="custom-switch-description">
                          {{ row.name }}
                        </span>
                      </label>
                    </b-col>
                  </template>
                </b-form-row>
              </b-container>
            </b-dropdown> -->
            <div class="input-icon ml-2">
              <span class="input-icon-addon">
                <i class="fe fe-calendar" />
              </span>
              <DatePicker ref="datePicker" @selected="loadEvents(1)" />
            </div>
            <div class="input-icon ml-2">
              <span class="input-icon-addon">
                <i class="fe fe-search" />
              </span>
              <input
                v-model.trim="hostname"
                type="text"
                class="form-control w-10"
                placeholder="主机名称"
                @keyup.enter="loadEvents(1)"
              />
            </div>

            <div class="input-icon ml-2">
              <span class="input-icon-addon">
                <i class="fe fe-search" />
              </span>
              <input
                v-model.trim="crash_message"
                type="text"
                class="form-control w-10"
                placeholder="报警信息"
                style="width: 210px;"
                @keyup.enter="loadEvents(1)"
              />
            </div>

            <button
              class="btn btn-primary ml-2"
              @click="loadEvents(1)"
              style="height: 38px;"
            >
              搜索
            </button>
          </div>
        </div>
      </div>
      <div class="card">
        <div class="card-body">
          <VueLoading
            v-if="loading"
            type="spiningDubbles"
            color="rgb(90, 193, 221)"
            :size="{ width: '50px', height: '50px' }"
          />

          <nav v-if="!loading && total > 0">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block;">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} /
                  {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination
              v-model="currentPage"
              align="right"
              :total-rows="total"
              :per-page="10"
              @change="loadEvents($event)"
            />
          </nav>

          <table v-if="!loading" class="table table-striped table-bordered">
            <thead>
              <tr>
                <th>
                  最后发现时间
                </th>
                <th nowrap>
                  主机名
                </th>
                <th>
                  RASP 版本
                </th>
                <th>
                  RASP 目录
                </th>
                <th nowrap>
                  报警信息
                </th>
                <th>
                  操作
                </th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="row in data" :key="row.id">
                <td nowrap>
                  {{ moment(row.event_time).format("YYYY-MM-DD") }}
                  <br />
                  {{ moment(row.event_time).format("HH:mm:ss") }}
                </td>                
                <td>
                  {{ row.hostname }}
                </td>
                <td nowrap>
                  {{ row.language }}/{{ row.version }} <br>
                  {{ row.plugin_name ? row.plugin_name : 'official' }}/{{ row.plugin_version }}
                </td>
                <td>
                  {{ row.rasp_home }}
                </td>
                <td style="word-break: break-word;">
                  {{ row.crash_message }}
                </td>
                <td nowrap>
                  <a href="javascript:" @click="showEventDetail(row)">
                    查看详情
                  </a>
                </td>
              </tr>
            </tbody>
          </table>
          <p v-if="!loading && total == 0" class="text-center">暂无数据</p>

          <nav v-if="!loading && total > 10">
            <ul class="pagination pull-left">
              <li class="active">
                <span style="margin-top: 0.5em; display: block;">
                  <strong>{{ total }}</strong> 结果，显示 {{ currentPage }} /
                  {{ ceil(total / 10) }} 页
                </span>
              </li>
            </ul>
            <b-pagination
              v-model="currentPage"
              align="right"
              :total-rows="total"
              :per-page="10"
              @change="loadEvents($event)"
            />
          </nav>
        </div>
      </div>
    </div>

    <CrashDetailModal ref="crashDetailModal" />
  </div>
</template>

<script>
import CrashDetailModal from "@/components/modals/crashDetailModal";
import DatePicker from "@/components/DatePicker";
import { mapGetters } from "vuex";

export default {
  name: "Crash",
  components: {
    CrashDetailModal,
    DatePicker,
  },
  data() {
    return {
      data: [],
      language_list: [
        {
          name: "Java",
          value: "java",
        },
        {
          name: "PHP",
          value: "php",
        },
      ],
      loading: false,
      currentPage: 1,
      hostname: "",
      rasp_id: "",
      selected_language: ["java", "php"],
      crash_message: "",
      total: 0,
    };
  },
  computed: {
    ...mapGetters(["current_app"]),
  },
  watch: {
    current_app() {
      this.loadEvents(1);
    },
    selected() {
      this.loadEvents(1);
    },
    selected_language() {
      this.loadEvents(1);
    },
  },
  mounted() {
    if (!this.current_app.id) {
      return;
    }
    this.loadEvents(1);
  },
  methods: {
    ceil: Math.ceil,
    displayURL(row) {
      if (!row.url) {
        return "-";
      }

      if (row.url.length > 100) {
        return row.url.substring(0, 100) + " ...";
      }

      return row.url;
    },
    selectAll({ target }) {
      this.selected = target.checked ? Object.keys(this.attack_types) : [];
    },
    showEventDetail(data) {
      this.$refs.crashDetailModal.showModal(data);
    },
    loadEvents(page) {
      this.loading = true;
      return this.request
        .post("v1/api/log/crash/search", {
          data: {
            start_time: this.$refs.datePicker.start.valueOf(),
            end_time: this.$refs.datePicker.end.valueOf(),
            app_id: this.current_app.id,
            hostname: this.hostname,
            language: this.selected_language,
            rasp_id: this.rasp_id,
            crash_message: this.crash_message,
          },
          page: page,
          perpage: 10,
        })
        .then((res) => {
          this.currentPage = page;
          this.data = res.data;
          this.total = res.total;
          this.loading = false;
        });
    },
  },
};
</script>

