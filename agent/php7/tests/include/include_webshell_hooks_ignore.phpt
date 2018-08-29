--TEST--
hook include webshell
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.hooks_ignore=webshell_include
--GET--
a=/tmp/openrasp/tmpfile
--ENV--
return <<<END
DOCUMENT_ROOT=/tmp/openrasp
END;
--FILE--
<?php
include($_GET['a']);
?>
--EXPECTREGEX--
temp