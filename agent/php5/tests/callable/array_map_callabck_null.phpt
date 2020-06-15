--TEST--
hook array_map callback null
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     webshell_callable: {
        name:   '算法4 - 拦截简单的 PHP array_map/walk/filter 后门',
        action: 'block',
        functions: [
            'system', 'exec'
        ]
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
print_r(array_map(null, array(1, 2), array('ls', 'ls')));
?>
--EXPECT--
Array
(
    [0] => Array
        (
            [0] => 1
            [1] => ls
        )

    [1] => Array
        (
            [0] => 2
            [1] => ls
        )

)