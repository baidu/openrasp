--TEST--
Prepare block case work environment
--SKIPIF--
<?
php if (!extension_loaded("openrasp")) print "skip"; 
if (!stristr(PHP_OS, "Linux")) die("skip this test is Linux platforms only");
?>
--FILE--
<?php 
function clearfiles($dirPath) {
    if (! is_dir($dirPath)) {
        throw new InvalidArgumentException("$dirPath must be a directory");
    }
    if (substr($dirPath, strlen($dirPath) - 1, 1) != '/') {
        $dirPath .= '/';
    }
    $files = glob($dirPath . '*', GLOB_MARK);
    foreach ($files as $file) {
        if (is_dir($file)) {
            clearfiles($file);
        } else {
            unlink($file);
        }
    }
}

function endsWith($haystack, $needle)
{
    $length = strlen($needle);
    return $length === 0 || 
    (substr($haystack, -$length) === $needle);
}

$excludedir =  realpath(ini_get('openrasp.root_dir'));
$needle = dirname(__FILE__);
if ((substr($excludedir, 0, strlen($needle)) === $needle))
{
  if (is_dir($excludedir))
  {
    clearfiles($excludedir);
    echo "clearfiles OK\n";
  }
  $rootdir =  ini_get('openrasp.root_dir') . '/plugins/block-test-plugin.js';
  $hook_types = array('sql','ssrf','directory','webdav','include','writeFile','fileUpload','command','xxe','ognl','deserialization','readFile');
  $plugin_header = "'use strict'\nvar plugin=new RASP('block-test-plugin')\n"
                  ."const clean = {action:'ignore', message:'action: ignore',confidence:0}\n"
                  ."const block = {action:'block', message:'action: block',confidence:100}\n";
  file_put_contents($rootdir, $plugin_header, LOCK_EX);
  $register_str = '';
  array_walk($hook_types, function ($item, $key, $excludedir) use (&$register_str)
  {
    if ($item=='writeFile' || $item=='readFile') {
      $register_str .= "plugin.register('" . $item . "', function (params, context) {if(params.realpath.lastIndexOf('$excludedir', 0) === 0){return clean}return block})\n";
    } else if ($item=='include'){
      $register_str .= "plugin.register('" . $item . "', function (params, context) {if(params.url.indexOf('.inc', params.url.length - '.inc'.length) !== -1){return clean}return block})\n";
    } else {
      $register_str .= "plugin.register('" . $item . "', function (params, context) {return block})\n";
    }
  }, $excludedir);
  file_put_contents($rootdir, $register_str, FILE_APPEND | LOCK_EX);
} else {
  echo "please set openrasp.root_dir=".__DIR__."/.work, and retry\n";
}
?>
--EXPECT--
clearfiles OK