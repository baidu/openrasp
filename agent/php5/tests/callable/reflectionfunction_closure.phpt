--TEST--
hook reflectionfunction
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
$counter2 = function()
{
    static $d = 0;
    return ++$d;

};

function dumpReflectionFunction($func)
{
    // Print out basic information
    printf(
        "The %s function '%s'\n",
        $func->isInternal() ? 'internal' : 'user-defined',
        $func->getName()
    );
}

// Create an instance of the ReflectionFunction class
dumpReflectionFunction(new ReflectionFunction($counter2));
?>
--EXPECT--
The user-defined function '{closure}'