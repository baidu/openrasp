--TEST--
hook curl_exec bad url
--SKIPIF--
<?php
if (!function_exists("curl_init")) die("Skipped: curl is disabled.");
$plugin = <<<EOF
plugin.register('ssrf', params => {
    assert(params.url == '/1q2w3e4r5t6y7u8i9o0p')
    assert(params.function == 'curl_exec')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
url=/1q2w3e4r5t6y7u8i9o0p
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
		if(!empty($data)){
			if (curl_getinfo($ch, CURLINFO_HTTP_CODE) == '200') {
				$headerSize = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
				$header = substr($data, 0, $headerSize);
				$body = substr($data, $headerSize);
				echo $body;
			} else {
				echo "no check";	
			}
		} else {
			echo "no check";
		}
	}
?>
--EXPECTREGEX--
no check