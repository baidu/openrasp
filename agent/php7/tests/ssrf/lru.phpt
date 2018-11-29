--TEST--
hook curl_exec lru
--SKIPIF--
<?php
$plugin = <<<EOF
let f = false
plugin.register('ssrf', params => {
	if (f) {
		return block
	} else {
		f = true
	}
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
url=http://requestb.in
--FILE--
<?php 
	$url = @$_GET['url'];
	if(!empty($url)){
		$ch = curl_init($url);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
		curl_setopt($ch, CURLOPT_NOBODY, FALSE); 
		curl_setopt($ch, CURLOPT_TIMEOUT_MS, 200);
		$data = curl_exec($ch);


		$ch = curl_init($url);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
		curl_setopt($ch, CURLOPT_NOBODY, FALSE); 
		curl_setopt($ch, CURLOPT_TIMEOUT_MS, 200);
		$data = curl_exec($ch);	
	}
	echo 'ok'
?>
--EXPECTREGEX--
ok