--TEST--
hook array_map
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
array_map("system", array('ls', 'ls'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>