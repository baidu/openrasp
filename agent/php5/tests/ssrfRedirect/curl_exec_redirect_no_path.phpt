--TEST--
hook curl_exec redirect no path
--SKIPIF--
<?php
if (!function_exists("curl_init")) die("Skipped: curl is disabled.");
$plugin = <<<EOF
plugin.register('ssrfRedirect', params => {
	console.log(params)
    assert(params.url == 'http://www.example.com')
	assert(params.url2 == 'http://www.example.com/')
    assert(params.function == 'curl_exec')
    assert(params.hostname == 'example.com')
	assert(params.hostname2 == 'example.com')
    assert(Array.isArray(params.ip))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
url=http://www.example.com
--FILE--
<?php 
	$url = @$_GET['url'];
	if(!empty($url)){
		$ch = curl_init($url);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
		curl_setopt($ch, CURLOPT_NOBODY, FALSE); 
		curl_setopt($ch, CURLOPT_TIMEOUT_MS, 200);
		$data = curl_exec($ch);
	}
?>
--EXPECT--
