--TEST--
hook file_put_contents (webshell)
--SKIPIF--
<?php
$dir = __DIR__;
$conf = <<<CONF
webshell_file_put_contents.action="log"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
file=/tmp/openrasp/tmpfile&content=test
--FILE--
<?php
file_put_contents($_GET['file'], $_GET['content']);
echo 'ok';
?>
--EXPECTREGEX--
ok