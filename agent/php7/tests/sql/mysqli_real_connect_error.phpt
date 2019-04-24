--TEST--
hook mysqli_real_connect error
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     sql_exception: {
        action: 'block'
    }
}
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php

$link = mysqli_init();
if (!$link) {
    die('mysqli_init failed');
}

if (!mysqli_real_connect($link, '127.0.0.1', 'nonexistentusername')) {
    die('Connect Error (' . mysqli_connect_errno() . ') '
            . mysqli_connect_error());
}

mysqli_close($link);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>