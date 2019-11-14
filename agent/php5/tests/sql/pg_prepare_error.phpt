--TEST--
hook pg_prepare error
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
    sql_exception: {
        name:      '算法3 - 记录数据库异常',
        action:    'log',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#sql-exception',
        pgsql: {
            error_code: [
                '42P01'
            ]
        }
    }
}
plugin.register('sql_exception', params => {
    plugin.log(params)
    assert(params.server == 'pgsql')
    assert(params.query == 'SELECT a FROM nonexisttable WHERE c=$1')
    assert(params.error_code == '42P01')
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
pg_prepare($con, 'my_query', 'SELECT a FROM nonexisttable WHERE c=$1');
pg_close($con);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>