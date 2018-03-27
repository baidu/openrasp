--TEST--
Check for ini baseline inspection
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!stristr(PHP_OS, "Linux")) die("skip this test is Linux platforms only");
?>
--FILE--
<?php 
$policy_ids = array(4001, 4002, 4003);
$policy_id_num = sizeof($policy_ids);
$policy_file_today = ini_get('openrasp.root_dir')."/logs/policy/policy.log.".date("Y-m-d");
$policy_file_yesterday = ini_get('openrasp.root_dir')."/logs/policy/policy.log.".date("Y-m-d", strtotime("-1 day"));
$policy_logs = array();
if (file_exists($policy_file_today)) {
    $today_logs =array_reverse(file($policy_file_today));
    $today_id_num = sizeof($today_logs);
    if ($today_id_num >= $policy_id_num) {
        $policy_logs = array_slice($today_logs, 0, $policy_id_num);
    } else {
        $policy_logs = $today_logs;
        if (!file_exists($policy_file_yesterday)) {
            die("policy log incomplete.");
        }
        $yesterday_logs =array_reverse(file($policy_file_yesterday));
        $tmp_arr = range(0, $policy_id_num - $today_id_num);
        foreach ($tmp_arr as $key => $value) {
            $policy_logs[] = $yesterday_logs[$value];
        }
    }
} else {
    if (!file_exists($policy_file_yesterday)) {
        die("policy log not exist.");
    }
    $yesterday_logs =array_reverse(file($policy_file_yesterday));
    $policy_logs = array_slice($yesterday_logs, 0, $policy_id_num);
}
$policy_content = implode(" ", $policy_logs);
foreach ($policy_ids as $key => $value) {
    if (stripos($policy_content, "\"policy_id\":".$value) === FALSE) {
        die("Fail to check policy id: ".$value);
    }
}
?>
--EXPECT--