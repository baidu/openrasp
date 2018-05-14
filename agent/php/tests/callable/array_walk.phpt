--TEST--
hook array_walk
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.callable_blacklists=system
--FILE--
<?php
$arr = array('ls', 'ls');
array_walk($arr, "system");
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>