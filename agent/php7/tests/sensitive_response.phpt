--TEST--
policy response
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('response', params => {
  console.log(params)
  return {
    action: 'log',
    message: 'sensitive',
    policy_params: {
      a:1
    }
  }
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
expose_php=false
display_errors=false
--FILE--
<?php
echo "1234567890";
flush();
include(__DIR__.'/timezone.inc');
passthru('tail -n 1 /tmp/openrasp/logs/policy/policy.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*sensitive.*