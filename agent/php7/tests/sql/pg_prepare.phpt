--TEST--
hook pg_prepare
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('sql', params => {
    assert(params.query == 'SELECT a FROM b WHERE c=$1')
    assert(params.server == 'pgsql')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
@$con = pg_connect('host=127.0.0.1 port=5432 user=postgres');
if (!$con) die("Skipped: can not connect to postgresql");
pg_close($con);
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.enforce_policy=Off
--FILE--
<?php
@$con = pg_connect('host=127.0.0.1 port=5432 user=postgres');
pg_prepare($con, 'my_query', 'SELECT a FROM b WHERE c=$1');
pg_close($con);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>