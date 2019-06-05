<template>
    <div>
        <div v-if="data.policy_id == '3001'">
            <div class="h6">
                配置文件路径
            </div>
            <p>
                {{ data.policy_params.config_file }}
            </p>
            <div class="h6">
                问题描述
            </div>
            <p>
                JSESSIONID 未开启 httpOnly，若开启可提升服务器对XSS的防范效果。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3001" target="_blank">点击这里</a>了解更多。
            </p>
        </div>

        <div v-if="data.policy_id == '3005'">
            <div class="h6">
                配置文件路径
            </div>
            <p>
                {{ data.policy_params.config_file }}
            </p>
            <div class="h6">
                问题描述
            </div>
            <p>
                开启 Directory Listing 功能，可能会泄露项目代码、配置等敏感信息。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3005" target="_blank">点击这里</a>了解更多。
            </p>
        </div>

        <div v-if="data.policy_id == '3007'">
            <div class="h6">
                配置文件路径
            </div>
            <p>
                {{ data.policy_params.config_file }}
            </p>
            <div class="h6">
                问题描述
            </div>
            <p>
                以 root/Administrator/system 权限启动应用服务器，会产生巨大的风险。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3007" target="_blank">点击这里</a>了解更多。
            </p>
        </div>                

        <div v-if="data.policy_id == '3002'">
            <div class="h6">
                进程 PID
            </div>
            <p>
                {{ data.policy_params.pid }}
            </p>
            <div class="h6">
                问题描述
            </div>
            <p>
                以root权限启动应用服务器，会产生巨大的风险。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3002" target="_blank">点击这里</a>了解更多。
            </p>
        </div>

        <div v-if="data.policy_id == '3003'">
            <div v-if="data.policy_params.config_file">
                <div class="h6">
                    配置文件路径
                </div>
                <p>
                    {{ data.policy_params.config_file }}
                </p>
            </div>

            <div v-if="data.policy_params.hostname && data.policy_params.port">
                <div class="h6">
                    服务器信息
                </div>
                <p>
                    {{ data.policy_params.hostname }}:{{ data.policy_params.port }}
                </p>
            </div>

            <div v-if="data.policy_params.socket">
                <div class="h6">
                    服务器信息
                </div>
                <p>
                    {{ data.policy_params.socket }}
                </p>
            </div>           

            <div class="h6">
                弱口令
            </div>
            <p>
                {{ data.policy_params.username }}:{{ data.policy_params.password }}
            </p>

            <div class="h6">
                问题描述
            </div>
            <p v-if="data.policy_params.server == 'mysql'">
                MySQL 存在弱口令，若账号被爆破会有数据泄露风险。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3003" target="_blank">点击这里</a>了解更多。
            </p>
            <p v-else-if="data.policy_params.type">
                {{ data.policy_params.type }} 存在弱口令。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3003" target="_blank">点击这里</a>了解更多。
            </p>
            <p v-else>
                Tomcat 管理后台存在弱口令，若管理后台对外暴露，会有被入侵的风险。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3003" target="_blank">点击这里</a>了解更多。
            </p>            
        </div>

        <div v-if="data.policy_id == '3004'">
            <div class="h6">
                webapps 路径
            </div>
            <p>
                {{ data.policy_params.path }}
            </p>

            <div class="h6">
                未删除的默认应用列表
            </div>
            <p>
                {{ data.policy_params.apps.join (", ") }}
            </p>
            <div class="h6">
                问题描述
            </div>
            <p>
                没有删除 tomcat 默认的 app，或多或少会泄露敏感信息，或者造成管理后台对外暴露的风险。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3004" target="_blank">点击这里</a>了解更多。
            </p>             
        </div>

        <div v-if="data.policy_id == '3006'">
            <div class="h6">
                数据库类型
            </div>
            <p>
                {{ data.policy_params.server }}
            </p>

            <div class="h6">
                用户名
            </div>
            <p>
                {{ data.policy_params.username }}
            </p>

            <div class="h6" v-if="data.policy_params.connectionString">
                connectionString
            </div>
            <p v-if="data.policy_params.connectionString">
                {{ data.policy_params.connectionString }}
            </p>

            <div class="h6" v-if="data.policy_params.socket">
                UNIX socket 路径
            </div>
            <p v-if="data.policy_params.socket">
                {{ data.policy_params.socket }}
            </p>

            <div class="h6">
                应用堆栈
            </div>
            <pre>{{ data.stack_trace }}</pre>
            <div class="h6">
                问题描述
            </div>
            <p>
                当存在SQL注入漏洞，使用高权限账号连接数据库会带来更大风险，泄露更多的数据。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#3004" target="_blank">点击这里</a>了解更多。
            </p>
        </div>

        <!-- 4000 - 4999 PHP 相关 -->
        <div v-if="data.policy_id == '4001'">
            <div class="h6">
                问题描述
            </div>
            <p>
                allow_url_include 没有关闭，当应用存在文件包含、任意文件读取等漏洞，开启这个配置会让应用更加容易被入侵。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#4001" target="_blank">点击这里</a>了解更多。
            </p>            
        </div>    

        <div v-if="data.policy_id == '4002'">
            <div class="h6">
                问题描述
            </div>
            <p>
                expose_php 没有关闭，会泄露PHP版本号。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#4002" target="_blank">点击这里</a>了解更多。
            </p>
        </div>     

        <div v-if="data.policy_id == '4003'">
            <div class="h6">
                问题描述
            </div>
            <p>
                display_errors 没有关闭，用户可以在前台看到PHP程序的错误信息。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#4002" target="_blank">点击这里</a>了解更多。
            </p>            
        </div>

        <div v-if="data.policy_id == '4004'">
            <div class="h6">
                问题描述
            </div>
            <p>
                yaml.decode_php 没有关闭，将允许YAML反序列化PHP对象。若应用存在漏洞，可导致服务器被入侵。
                <a href="https://rasp.baidu.com/doc/usage/security_policy.html#4002" target="_blank">点击这里</a>了解更多。
            </p>
        </div>

    </div>

</template>

<script>

export default {
    name: 'baseline_params',
    data: function () {
        return {
            data: {
                policy_params: {}
            }
        }
    },
    methods: {
        setData: function (data) {
            this.data = data

            // v1.0 版本，weblogic 忘记增加 policy_params 字段，简单修复
            if (! data['policy_params']) {
                data['policy_params'] = {}
            }
        }
    }
}
</script>

