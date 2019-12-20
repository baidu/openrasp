--TEST--
policy sensitive response
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('response', params => {
  assert(params.content.indexOf('1234567890') != -1)
  return {
    action: 'log',
    message: 'sensitive',
    params: {
      a: 1
    }
  }
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
expose_php=false
display_errors=false
--FILE--
<?php
echo "1234567890";
?>
--EXPECT--
1234567890