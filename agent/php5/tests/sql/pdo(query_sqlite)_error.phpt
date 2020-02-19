--TEST--
hook PDO::query sqlite error
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
    sql_exception: {
        name:      '算法3 - 记录数据库异常',
        action:    'log',
        reference: 'https://rasp.baidu.com/doc/dev/official.html#sql-exception',
        sqlite: {
            error_code: [
                1
            ]
        }
    }
}
plugin.register('sql_exception', params => {
    assert(params.server == 'sqlite')
    assert(params.query == 'select \'')
    assert(params.error_code == '1')
    return block
})
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$con = new PDO('sqlite:'.realpath('test.db'));
$con->query('select \'');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>