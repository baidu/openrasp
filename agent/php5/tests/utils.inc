<?php
function cleanup_directory($dir) {
  $iter = new RecursiveDirectoryIterator($dir);
  foreach (new RecursiveIteratorIterator($iter, RecursiveIteratorIterator::CHILD_FIRST)
 as $f) {
    if ($f->isDir()) {
      rmdir($f->getPathname());
    } else {
      unlink($f->getPathname());
    }
  }
  rmdir($dir);
}
?>