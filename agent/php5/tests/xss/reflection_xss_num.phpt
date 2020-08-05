--TEST--
hook output detect (reflection xss)
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     xss_userinput: {
        action: 'block',
        filter_regex: "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/ >]",
        min_length: 15,
        max_detection_num: 1
    }
}
EOF;
$conf = <<<CONF
block.redirect_url: "/block?request_id="

CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--CGI--
--GET--
a=<script>alert("xss")</script>&b=<script>alert("xss")</script>&c=<script>alert("xss")</script>
--FILE--
<?php
echo '<pre>just kidding</pre>';
?>
--EXPECT--
<pre>just kidding</pre>
