--TEST--
hook move_uploaded_file
--SKIPIF--
<?php
if (php_sapi_name()=='cli') die('skip:  forces the use of the CGI binary');
$plugin = <<<EOF
plugin.register('fileUpload', params => {
    assert(params.name == 'pics')
    assert(params.filename == 'file1.txt')
    assert(params.dest_path == '/tmp/openrasp/file1.txt')
    assert(params.dest_realpath == '/tmp/openrasp/file1.txt')
    assert(params.content == 'abcdef123456789')
    return block
})
EOF;
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
    move_uploaded_file($tmp_name, "$uploads_dir/$name");
}

?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>