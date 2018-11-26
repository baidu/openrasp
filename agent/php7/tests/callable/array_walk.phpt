--TEST--
hook array_walk
--SKIPIF--
<?php
$conf = <<<CONF
webshell_callable.blacklist=["system", "exec"]
webshell_callable.action="block"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$arr = array('ls', 'ls');
array_walk($arr, "system");
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>