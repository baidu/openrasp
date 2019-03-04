<template>
    <div class="event-detail">
        <div v-if="data.attack_type == 'sql'">
            SQL注入大多是语句拼接造成的，请检查代码中是否有参数拼接的情况。在条件允许的情况下，尽量使用 prepare query 的形式来修复漏洞。
        </div>

        <div v-if="data.attack_type == 'sql_exception'">
            检查SQL语句异常是否来自正常操作，如果不是请检查应用是否存在漏洞。
        </div>        

        <div v-if="data.attack_type == 'directory'">
            通常应用不会读取敏感目录，请与业务线确认这个是否为正常行为。如果不是，请根据堆栈检查服务器是否存在 webshell 后门。
        </div>        

        <div v-if="data.attack_type == 'readFile'">
            通常应用不会读取敏感文件，请与业务线确认这个是否为正常行为。如果不是，请根据堆栈检查服务器是否存在 webshell 后门。
        </div>      

        <div v-if="data.attack_type == 'writeFile'">
            暂无
        </div>          

        <div v-if="data.attack_type == 'include'">
            请检查要包含的文件是否正常，否则表示服务器上存在文件包含漏洞。
        </div>     

        <div v-if="data.attack_type == 'webdav'">
            请检查服务器是否开放了 MOVE/PUT 操作，攻击者可利用特殊的HTTP方法上传 webshell 后门。此算法没有误报。           
        </div>                    

        <div v-if="data.attack_type == 'fileUpload'">
            当服务器允许用户上传文件，且没有过滤扩展名时，可能造成任意文件上传漏洞。修复方法是使用扩展名的白名单，比如只允许 txt/png/jpg 等后缀。此算法通常没有误报。
        </div>

        <div v-if="data.attack_type == 'rename'">
            当服务器存在漏洞，攻击者可以通过重命名的方式，将不可执行的 txt/rar 等文件名重命名为 php/jsp/.. 等可以执行的脚本文件。此算法在一些PHP框架下会误报。
        </div>        

        <div v-if="data.attack_type == 'command'">
            如果这个命令执行不是业务正常需求，则很有可能是 webshell 后门执行的操作，或者攻击者利用漏洞执行的命令。
        </div>  

        <div v-if="data.attack_type == 'xxe'">
            检查业务代码是否直接处理用户输入的 xml，且允许加载任意外部实体，若允许则可能存在 xxe 漏洞。
            在 v1.1 之后我们会增加XXE代码安全开关，自动帮你禁止xml实体加载，自动修复漏洞。
        </div>         

        <div v-if="data.attack_type == 'ognl'">
            尝试升级 struts 到新版，或者推动业务改造，使用其他的开发框架。在百度，我们已经禁止业务线使用 struts 框架。
        </div>   

        <div v-if="data.attack_type == 'deserialization'">
            检查系统是否存在已知的反序列化漏洞，并升级软件或者框架到最新版本。
        </div>         

        <div v-if="data.attack_type == 'ssrf'">
            检查业务代码，看是否可以直接向用户指定的内网URL，发起请求。
        </div>  

        <!-- 以下为 php 原生 -->

        <div v-if="data.attack_type == 'webshell_eval'">
            检查服务器上是否存在中国菜刀后门，后门也可能存在于业务代码中，具体请看应用堆栈。
        </div>                  

        <div v-if="data.attack_type == 'webshell_command'">
            检查服务器上是否存在 webshell 后门或者可以被利用的命令执行点，此算法没有误报。
        </div>  

        <div v-if="data.attack_type == 'webshell_file_put_contents'">
            检查服务器上是否存在 webshell 后门，或者可以被利用的后门上传点，此算法没有误报。
        </div>
        
        <div v-if="data.attack_type == 'webshell_callable'">
            检查服务器上是否存在 webshell 后门，即使用 call_user_func/array_walk/.. 调用 system/exec/... 等函数的方式。此算法没有误报。
        </div>

        <div v-if="data.attack_type == 'webshell_ld_preload'">
            检查服务器上是否存在 webshell 后门，即使用 putenv + LD_PRELOAD + mail 方式执行任意命令的后门。
        </div>        

        <div v-if="data.attack_type == 'xss_userinput' || data.attack_type == 'xss_echo'">
            检查应用是否直接输出了用户传入的参数，可以考虑使用 ESAPI 进行转义
        </div>

    </div>

</template>

<script>

export default {
    name: 'fix_solutions',
    data: function () {
        return {
            data: {
                attack_params: {}
            }
        }
    },
    methods: {
        setData: function (data) {
            this.data = data
        }
    }
}
</script>

