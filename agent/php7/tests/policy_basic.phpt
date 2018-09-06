--TEST--
policy check (basic)
--SKIPIF--
<?php
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
allow_url_include=true
expose_php=true
display_errors=true
yaml.decode_php=no
--FILE--
<?php
include(__DIR__.'/timezone.inc');
passthru('tail -n 3 /tmp/openrasp/logs/policy/policy.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*4001.*
.*4002.*
.*4003.*