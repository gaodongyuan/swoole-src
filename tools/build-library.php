#!/usr/bin/env php
<?php
define('SRC_DIR', dirname(__DIR__));
define('LIB_DIR', SRC_DIR . '/library');
define('LIB_H', SRC_DIR . '/php_swoole_library.h');
define('PHP_TAG', '<?php');

if (!defined('SWOOLE_LIBRARY')) {
    require LIB_DIR . '/functions.php';
    require LIB_DIR . '/core/ArrayObject.php';
    require LIB_DIR . '/core/StringObject.php';
}

require __DIR__ . '/functions.php';

$list = require LIB_DIR . '/config.inc';
if (empty($list)) {
    swoole_error('can not read library config');
}
$source_str = $eval_str = '';
foreach ($list as $file) {

    $php_file = LIB_DIR . '/' . $file;
    if (!swoole_string(`/usr/bin/env php -n -l $php_file`)->contains('No syntax errors detected')) {
        swoole_error("syntax error in [$php_file]");
    } else {
        swoole_ok("syntax correct in [$php_file]");
    }
    $code = file_get_contents($php_file);
    if ($code === false) {
        swoole_error("can not read file {$file}");
    }
    if (strpos($code, PHP_TAG) !== 0) {
        swoole_error('swoole library php file must start with "<?php"');
    }

    $name = unCamelize(str_replace(['/', '.php'], ['_', ''], $file));
    // keep line breaks to align line numbers
    $code = rtrim(substr($code, strlen(PHP_TAG)));
    $code = str_replace(['\\', '"', "\n"], ['\\\\', '\\"', "\\n\"\n\""], $code);
    $code = implode("\n" . space(4), explode("\n", $code));
    $filename = "@swoole-src/library/{$file}";
    $source_str .= "static const char* swoole_library_source_{$name} =\n" . space(4) . "\"{$code}\\n\";\n\n";
    $eval_str .= space(4) . "zend::eval(swoole_library_source_{$name}, \"{$filename}\");\n";
}
$source_str = rtrim($source_str);
$eval_str = rtrim($eval_str);

$generator = basename(__FILE__);
$content = <<<PHP
/**
 * Generated by {$generator}, Please DO NOT modify!
 */

{$source_str}

static void php_swoole_load_library()
{
{$eval_str}
}

PHP;

if (file_put_contents(LIB_H, $content) != strlen($content)) {
    swoole_error('Can not write source codes to ' . LIB_H);
}
swoole_success("Generated swoole php library successfully!");
