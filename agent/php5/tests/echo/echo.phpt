--TEST--
hook echo
--SKIPIF--
<?php
$conf = <<<CONF
xss_echo.action="block"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=<b>test</b>
--FILE--
<?php
echo $_GET['a'];
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>