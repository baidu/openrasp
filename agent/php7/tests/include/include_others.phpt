--TEST--
hook include
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--ENV--
return <<<END
DOCUMENT_ROOT=/tmp/openrasp
END;
--FILE--
<?php
class A
{
    public $var = 'test.inc';

    public function getVar() {
        return $this->var;
    }
}
$a = new A();
@include($a->getVar());
$b='test.inc';
include($b);
?>
--EXPECTREGEX--
