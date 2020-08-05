--TEST--
hook assert (webshell)
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: assert() is now a language construct and not a function in PHP7.');
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_eval: {
        name:   '算法1 - 拦截简单的 PHP 中国菜刀后门',
        action: 'block'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=1
--FILE--
<?php
assert($_GET['a']);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>