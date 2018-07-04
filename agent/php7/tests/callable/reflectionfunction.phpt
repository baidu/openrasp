--TEST--
hook reflectionfunction
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.callable_blacklists=system,exec
--FILE--
<?php
$func = new ReflectionFunction('system'); 
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>