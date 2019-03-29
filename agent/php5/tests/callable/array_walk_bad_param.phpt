--TEST--
hook array_walk bad param
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
array_walk();
?>
--EXPECTREGEX--
Warning: array_walk\(\) expects at least 2 parameters, 0 given in.*
