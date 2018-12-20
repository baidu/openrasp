--TEST--
hook proc_open (webshell)
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_command: {
        name:   '算法2 - 拦截简单的 PHP 命令执行后门',
        action: 'block'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=cd
--FILE--
<?php
$descriptorspec = array(  
            0 => array("pipe", "r"), //标准输入，子进程从此管道读取数据  
            1 => array("pipe", "w"), //标准输出，子进程向此管道写入数据  
            2 => array("file", "error-output.txt","a")    //标准错误，写入到指定文件  
            );
$process = proc_open($_GET['a'], $descriptorspec, $pipes); 
if (is_resource($process)) {
    // 切记：在调用 proc_close 之前关闭所有的管道以避免死锁。
    $return_value = proc_close($process);
}
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>