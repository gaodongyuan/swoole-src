--TEST--
Test curl_error() & curl_errno() function with problematic host
--CREDITS--
TestFest 2009 - AFUP - Perrick Penet <perrick@noparking.net>
--SKIPIF--
<?php
	if (!extension_loaded("curl")) print "skip";
	$addr = "www.".uniqid().".".uniqid();
	if (gethostbyname($addr) != $addr) {
		print "skip catch all dns";
	}
?>
--FILE--
<?php
require __DIR__ . '/../../../include/bootstrap.php';

$cm = new \SwooleTest\CurlManager();
$cm->run(function ($host) {

    $url = "http://www.".uniqid().".".uniqid();
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);

    curl_exec($ch);
    var_dump(curl_error($ch));
    var_dump(curl_errno($ch));
    curl_close($ch);
}, false);



?>
--EXPECTF--
%s resolve%s
int(6)
