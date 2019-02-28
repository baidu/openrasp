--TEST--
hook curl_exec with port
--SKIPIF--
<?php
if (!function_exists("curl_init")) die("Skipped: curl is disabled.");
$plugin = <<<EOF
plugin.register('ssrf', params => {
    assert(params.url == 'http://127.0.0.1:80')
    assert(params.function == 'curl_exec')
    assert(params.hostname == '127.0.0.1')
	assert(params.port == '80')
    assert(Array.isArray(params.ip))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
url=http://127.0.0.1:80
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
			}
		}
		
	}
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>