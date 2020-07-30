<template>
    <div class="event-detail">
        <div v-if="data.attack_type == 'sql'">
            <div class="h6">
                数据库类型
            </div>
            <p>
                {{ data.attack_params.server }}
            </p>
            <div class="h6">
                执行的SQL语句
                <!--
                <a href="javascript:" style="color: #467fcf"
                    v-clipboard:copy="data.attack_params.query"
                    v-clipboard:success="onCopySucc"
                    v-clipboard:error="onCopyError">[复制]</a> 
                -->
            </div>
            <p>
                {{ data.attack_params.query.length > 10000 ? data.attack_params.query + ' ...' : data.attack_params.query }}                
            </p>
        </div>

        <div v-if="data.attack_type == 'sql_exception'">
            <div class="h6">
                数据库类型
            </div>
            <p>
                {{ data.attack_params.server }}
            </p>            
            <div v-if="data.attack_params.query">
                <div class="h6">
                    执行的SQL语句 

                    <!--
                    <a href="javascript:" 
                        style="color: #467fcf" 
                        v-clipboard:copy="data.attack_params.query"
                        v-clipboard:success="onCopySucc"
                        v-clipboard:error="onCopyError">复制</a>
                    -->
                </div>
                <p>
                    {{ data.attack_params.query.length > 10000 ? data.attack_params.query + ' ...' : data.attack_params.query }}
                </p>
            </div>
            <!--
            <div class="h6">
                错误信息
            </div>
            <p>
                [{{ data.attack_params.error_code }}] {{ data.attack_params.error_msg }}
            </p>
            -->
        </div>        

        <div v-if="data.attack_type == 'directory'">
            <div class="h6">
                读取的目录
            </div>
            <p>
                {{ data.attack_params.path }}
            </p>
            <div class="h6">
                读取的目录 - 真实路径
            </div>
            <p>
                {{ data.attack_params.realpath }}
            </p>
        </div>        

        <div v-if="data.attack_type == 'readFile'">
            <div class="h6">
                读取的文件
            </div>
            <p>
                {{ data.attack_params.path }}
            </p>
            <div class="h6">
                读取的文件 - 真实路径
            </div>
            <p>
                {{ data.attack_params.realpath }}
            </p>
        </div>      

        <div v-if="data.attack_type == 'writeFile'">
            <div class="h6">
                写入的文件
            </div>
            <p>
                {{ data.attack_params.path }}
            </p>
            <div class="h6">
                写入的文件 - 真实路径
            </div>
            <p>
                {{ data.attack_params.realpath }}
            </p>
        </div>     

        <div v-if="data.attack_type == 'deleteFile'">
            <div class="h6">
                删除的文件
            </div>
            <p>
                {{ data.attack_params.path }}
            </p>
            <div class="h6">
                删除的文件 - 真实路径
            </div>
            <p>
                {{ data.attack_params.realpath }}
            </p>
        </div>      

        <div v-if="data.attack_type == 'include'">
            <div class="h6">
                要包含的文件
            </div>
            <p>
                {{ data.attack_params.url }}
            </p>
            <div class="h6" v-if="data.attack_params.realpath">
                要包含的文件 - 真实路径
            </div>
            <p>
                {{ data.attack_params.realpath }}
            </p>            
        </div>     

        <div v-if="data.attack_type == 'webdav'">
            <div class="h6">
                源文件
            </div>
            <p>
                {{ data.attack_params.source }}
            </p>
            <div class="h6">
                目标文件
            </div>
            <p>
                {{ data.attack_params.dest }}
            </p>            
        </div>                    

        <div v-if="data.attack_type == 'fileUpload'">
            <div class="h6" v-if="data.attack_params.name">
                Multipart 参数名称
            </div>
            <p v-if="data.attack_params.name">
                {{ data.attack_params.name }}
            </p>

            <div class="h6">
                上传的文件名
            </div>
            <p>
                {{ data.attack_params.filename }}
            </p>

            <template v-if="data.attack_params.dest_path">
                <div class="h6">
                    文件存储位置
                </div>
                <p>{{ data.attack_params.dest_path }}</p>

                <div class="h6">
                    文件存储位置（真实路径）
                </div>
                <p>{{ data.attack_params.dest_realpath }}</p>
            </template>
            
            <div class="h6">
                上传的文件内容 - 前4KB
            </div>
            <pre>{{ data.attack_params.content }}</pre>

            
        </div>

        <div v-if="data.attack_type == 'rename'">
            <div class="h6">
                源文件
            </div>
            <p>
                {{ data.attack_params.source }}
            </p>
            <div class="h6">
                目标文件
            </div>
            <p>
                {{ data.attack_params.dest }}
            </p>
        </div>        

        <div v-if="data.attack_type == 'command'">
            <div class="h6">
                要执行的命令
            </div>
            <p>
                {{ data.attack_params.command }}
            </p>
        </div>  

        <div v-if="data.attack_type == 'xxe'">
            <div class="h6">
                要加载的外部实体
            </div>
            <p>
                {{ data.attack_params.entity }}
            </p>
        </div>         

        <div v-if="data.attack_type == 'ognl'">
            <div class="h6">
                要执行的OGNL表达式
            </div>
            <p>
                {{ data.attack_params.expression }}
            </p>
        </div>   

        <div v-if="data.attack_type == 'deserialization'">
            <div class="h6">
                要生成的Java类名
            </div>
            <p>
                {{ data.attack_params.clazz }}
            </p>
        </div>         

        <div v-if="data.attack_type == 'ssrf' || data.attack_type == 'ssrfRedirect'">
            <div class="h6">
                要访问的 URL
            </div>
            <p>
                {{ data.attack_params.url }}
            </p>
            <div class="h6">
                目标 IP
            </div>
            <p>
                {{ data.attack_params.ip.join(", ") }}
            </p>                     
        </div>

        <div v-if="data.attack_type == 'xss_echo' || data.attack_type == 'xss_userinput'">
            <div class="h6" v-if="data.attack_params.name">
                XSS 参数名称
            </div>
            <p v-if="data.attack_params.name">
                {{ data.attack_params.name }}
            </p>

            <div class="h6" v-if="data.attack_params.value">
                XSS 利用代码
            </div>
            <p v-if="data.attack_params.value">
                {{ data.attack_params.value.length > 10000 ? data.attack_params.value + ' ...' : data.attack_params.value }}   
            </p>            
        </div>

        <div v-if="data.attack_type == 'response' && data.plugin_algorithm == 'response_dataLeak'">
            <div class="h6">
                泄露的敏感信息样例
            </div>
            <div>
                <table class="table table-bordered">
                    <thead>
                        <tr>
                            <th>类型</th>
                            <th>未打码信息</th>
                            <th>原始内容（片段）</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr v-for="(row, index) in data.attack_params.parts" :key="index">
                            <td nowrap>{{ row.type }}</td>
                            <td nowrap>{{ row.match }}</td>
                            <td>{{ row.parts }}</td>
                        </tr>
                    </tbody>
                </table>
            </div>            
        </div>  

        <!-- 以下为 php 原生 -->

        <div v-if="data.attack_type == 'webshell_eval'">
            <div class="h6">
                菜刀要 eval 的内容
            </div>
            <p>
                {{ data.attack_params.eval }}
            </p>
        </div>                  

        <div v-if="data.attack_type == 'webshell_command'">
            <div class="h6">
                WebShell 要执行的命令
            </div>
            <p>
                {{ data.attack_params.command }}
            </p>
        </div>  

        <div v-if="data.attack_type == 'webshell_file_put_contents'">
            <div class="h6">
                WebShell 要写入的文件
            </div>
            <p>
                {{ data.attack_params.name }}
            </p>
            <div class="h6">
                WebShell 要写入的文件 - 真实路径
            </div>
            <p>
                {{ data.attack_params.realpath }}
            </p>
        </div>
        
        <div v-if="data.attack_type == 'webshell_callable'">
            <div class="h6">
                后门要执行的函数
            </div>
            <p>
                {{ data.attack_params.function }}()
            </p>
        </div>

    </div>

</template>

<script>

export default {
    name: 'attack_params',
    data: function () {
        return {
            data: {
                message: 'wtf',
                attack_params: {}
            }
        }
    },
    methods: {
        setData: function (data) {
            this.data = data
        },
        onCopySucc: function(e) {
            console.log('succ')
        },
        onCopyError: function(e) {
            console.log('fail', e)
        }
    }
}
</script>

