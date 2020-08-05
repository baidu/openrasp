--TEST--
hook proc_open
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'cd')
    assert(params.stack[0].indexOf('proc_open') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$descriptorspec = array(  
            0 => array("pipe", "r"), //标准输入，子进程从此管道读取数据  
            1 => array("pipe", "w"), //标准输出，子进程向此管道写入数据  
            2 => array("file", "error-output.txt","a")    //标准错误，写入到指定文件  
            );
$process = proc_open("cd", $descriptorspec, $pipes); 
if (is_resource($process)) {
    // 切记：在调用 proc_close 之前关闭所有的管道以避免死锁。
    $return_value = proc_close($process);
}
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>