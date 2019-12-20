--TEST--
hook MongoDB\Driver\BulkWrite::delete object
--SKIPIF--
<?php
if (PHP_VERSION_ID < 50500) die('Skipped: not supported (version < 5.5.0)');
$plugin = <<<EOF
plugin.register('mongodb', params => {
    assert(params.query == '{ "_id" : { "\$oid" : "5a2493c33c95a1281836eb6a" } }')
    assert(params.server == 'mongodb')
    assert(params.class.endsWith('Bulkwrite'))
    assert(params.method == 'delete')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mongodb")) die("Skipped: mongodb extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
class MyDocument implements MongoDB\BSON\Serializable
{
    private $id;

    function __construct()
    {
        $this->id = new MongoDB\BSON\ObjectId("5a2493c33c95a1281836eb6a");
    }

    function bsonSerialize()
    {
        return ['_id' => $this->id];
    }
}
$manager = new MongoDB\Driver\Manager("mongodb://openrasp:rasp#2019@localhost:27017/test");
$bulk = new MongoDB\Driver\BulkWrite;
$bulk->delete(new MyDocument);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>