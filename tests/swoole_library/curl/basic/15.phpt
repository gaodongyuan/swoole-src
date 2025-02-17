--TEST--
Test curl_init() function with $url parameter defined
--CREDITS--
Jean-Marc Fontaine <jmf@durcommefaire.net>
--SKIPIF--
<?php if (!extension_loaded("curl")) exit("skip curl extension not loaded"); ?>
--FILE--
<?php

require __DIR__ . '/../../../include/bootstrap.php';

$cm = new \SwooleTest\CurlManager();
$cm->run(function ($host) {
    $url = 'http://www.example.com/';
    $ch = curl_init($url);

    Assert::eq($url, curl_getinfo($ch, CURLINFO_EFFECTIVE_URL));
}, false);

?>
===DONE===
--EXPECT--
===DONE===
