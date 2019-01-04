--TEST--
inject urlprefix
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '5.6.0', '<')) die("Skipped: PHP_VERSION < 5.6.0");
$conf = <<<CONF
inject.urlprefix: "/prefix"
CONF;
include(__DIR__.'/skipif.inc');
file_put_contents('/tmp/openrasp/assets/inject.html', "inject content");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--ENV--
return <<<END
REQUEST_URI=/prefix/index.php
END;
--FILE--
<?php
unlink('/tmp/openrasp/assets/inject.html');
?>
ok
--EXPECTHEADERS--
Content-type: text/html; charset=UTF-8
--EXPECT--
ok
inject content