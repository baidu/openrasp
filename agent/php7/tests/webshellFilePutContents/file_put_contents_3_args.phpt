--TEST--
hook file_put_contents (webshell)
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_file_put_contents: {
        name:   '算法3 - 拦截简单的 PHP 文件上传后门',
        action: 'block'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
file=/tmp/openrasp/tmpfile&content=test
--FILE--
<?php
file_put_contents($_GET['file'], $_GET['content'], FILE_APPEND);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>