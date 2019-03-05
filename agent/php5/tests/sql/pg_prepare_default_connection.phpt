--TEST--
hook pg_prepare default connection
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('sql', params => {
    assert(params.query == 'SELECT a FROM b WHERE c=$1')
    assert(params.server == 'pgsql')
    return block
})
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
include('pg_connect.inc');
if (!$con) die("Skipped: can not connect to postgresql");
pg_close($con);
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include('pg_connect.inc');
pg_prepare('my_query', 'SELECT a FROM b WHERE c=$1');
pg_close($con);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>