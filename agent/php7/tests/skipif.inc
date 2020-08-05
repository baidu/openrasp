<?php
if (!extension_loaded("openrasp")) die("Skipped: openrasp extension required.");

$js_file = ini_get("openrasp.root_dir") . "/plugins/test.js";
$plugin_header = <<<EOF
const plugin = new RASP('test')
const block = {action: 'block'}
const assert = function(f) {
  if (!f) {
    throw new Error()
  }
}

EOF;
if (!file_put_contents($js_file, $plugin_header)) die("Skipped: can not create " . $js_file);
if (isset($plugin)) file_put_contents($js_file, $plugin, FILE_APPEND);

$conf_file = ini_get("openrasp.root_dir") . "/conf/openrasp.yml";
$conf_header = <<<CONFHEADER
#config for test

CONFHEADER;
if (!file_put_contents($conf_file, $conf_header)) die("Skipped: can not create " . $conf_file);
if (isset($conf)) file_put_contents($conf_file, $conf, FILE_APPEND);
?>