--TEST--
hook move_uploaded_file bad param
--SKIPIF--
<?php
if (php_sapi_name()=='cli') die('skip:  forces the use of the CGI binary');
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--POST_RAW--
Content-type: multipart/form-data, boundary=AaB03x

--AaB03x
content-disposition: form-data; name="field1"

Joe Blow
--AaB03x
content-disposition: form-data; name="pics"; filename="file1.txt"
Content-Type: text/plain

abcdef123456789
--AaB03x--
--FILE--
<?php
$uploads_dir = '/tmp/openrasp';
if ($_FILES["pics"]["error"] == UPLOAD_ERR_OK) {
    $tmp_name = $_FILES["pics"]["tmp_name"];
    $name = $_FILES["pics"]["name"];
    move_uploaded_file();
}

?>
--EXPECTREGEX--
Warning: move_uploaded_file\(\) expects exactly 2 parameters, 0 given.*