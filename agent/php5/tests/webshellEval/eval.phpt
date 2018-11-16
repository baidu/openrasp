--TEST--
hook eval (webshell)
--SKIPIF--
<?php
$conf = <<<CONF
webshell_eval.action="block"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=$val=1;
--FILE--
<?php
$val=0;
eval($_GET['a']);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>