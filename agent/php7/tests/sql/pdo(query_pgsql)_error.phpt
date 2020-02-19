--TEST--
hook PDO::query pgsql error
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
                '22P02'
            ]
        }
    }
}
plugin.register('sql_exception', params => {
    assert(params.server == 'pgsql')
    assert(params.query == 'select 1=1 AND 7778=CAST((SELECT version())::text AS NUMERIC)')
    assert(params.error_code == '22P02')
    return block
})
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
@$con = pg_connect('host=127.0.0.1 port=5432 user=postgres password=postgres');
if (!$con) die("connection failed");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include('pdo_pgsql.inc');
$con->query('select 1=1 AND 7778=CAST((SELECT version())::text AS NUMERIC)');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>