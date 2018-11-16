--TEST--
hook assert (webshell)
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: assert() is now a language construct and not a function in PHP7.');
$conf = <<<CONF
webshell_eval.action="block"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=cd
--FILE--
<?php
assert($_GET['a']);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>