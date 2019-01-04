--TEST--
inject urlprefix
--SKIPIF--
<?php
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
header('Content-type: text/html; charset=UTF-8', true, 200);
unlink('/tmp/openrasp/assets/inject.html');
?>
ok
--EXPECTHEADERS--
Content-type: text/html; charset=UTF-8
--EXPECT--
ok
inject content