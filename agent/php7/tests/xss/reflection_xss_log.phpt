--TEST--
hook output detect (reflection xss)
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     xss_userinput: {
        action: 'log',
        filter_regex: "<![\\\\-\\\\[A-Za-z]|<([A-Za-z]{1,12})[\\\\/ >]",
        min_length: 15,
        max_detection_num: 10
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--CGI--
--GET--
a=<script>alert("xss")</script>
--FILE--
<?php
echo '<pre>' . $_GET[ 'a' ] . '</pre>';
?>
--EXPECT--
<pre><script>alert("xss")</script></pre>