--TEST--
hook include
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=force_to_cgi
--ENV--
return <<<END
DOCUMENT_ROOT=/tmp/openrasp
END;
--FILE--
<?php
class IT extends ArrayIterator {
    private $n = 0;

    function __construct($trap = null) {
    	parent::__construct([0, 1]);
    	$this->trap = $trap;
    }

    function trap($trap) {
    	if ($trap === $this->trap) {
    		throw new Exception($trap);
    	}
    }

    function rewind()  {$this->trap(__FUNCTION__); return parent::rewind();}
    function valid()   {$this->trap(__FUNCTION__); return parent::valid();}
    function key()     {$this->trap(__FUNCTION__); return parent::key();}
    function next()    {$this->trap(__FUNCTION__); return parent::next();}
}

foreach(['rewind', 'valid', 'key', 'next'] as $trap) {
	$obj = new IT($trap);
	try {
		// IS_CV
		foreach ($obj as $key => &$val) @include($val);
	} catch (Exception $e) {
	}
	unset($obj);
	try {
		// IS_VAR
		foreach (new IT($trap) as $key => &$val) @include($val);
        // IS_TMP_VAR
        foreach ((object)new IT($trap) as $key => &$val) @include($val);
	} catch (Exception $e) {
	}
}
?>
--EXPECTREGEX--
