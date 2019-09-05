--TEST--
hook mysql_pconnect error
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
$plugin = <<<EOF
plugin.register('sql_exception', params => {
    assert(params.hostname == '127.0.0.1')
    assert(params.username == 'nonexistentusername')
    assert(params.error_code == '1045')
    return block
})
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
mysql_pconnect('127.0.0.1', 'nonexistentusername');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>