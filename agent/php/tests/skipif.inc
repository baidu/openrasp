<?php
if (!extension_loaded("openrasp")) die("Skipped: openrasp extension required.");

$js_file = ini_get("openrasp.root_dir") . "/plugins/test.js";
$plugin_header = <<<EOF
const plugin = new RASP('test')
const block = {action: 'block'}
function assert(flag)
{
    if (!flag) throw new Error('assertion error')
}

EOF;
if (!file_put_contents($js_file, $plugin_header)) die("Skipped: can not create " . $js_file);
if (isset($plugin)) file_put_contents($js_file, $plugin, FILE_APPEND);
?>