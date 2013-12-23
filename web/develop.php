<?php

error_reporting(E_ALL ^ E_NOTICE);

function handle_exception(Exception $ex)
{
    header('HTTP/1.0 404 Not Found');
    header('Content-type: text/html');
    echo '<html>',
           '<head><title>Error</title></head>',
           '<body>',
             '<h1>Error</h1>',
             '<p>', htmlentities($ex->getMessage()), '</p>',
           '</body>',
         '</html>';

    exit;
}

set_exception_handler('handle_exception');
date_default_timezone_set('UTC');

require 'smasher/smasher.php';

$options = array(
    'conf'     => 'smasher/smasher.xml',
    'type'     => NULL,
    'group'    => NULL,
    'nominify' => false
);

$options['group'] = $_POST['group'];
$options['method'] = $_POST['method'];
$url = parse_url ($_SERVER["REQUEST_URI"]);
$path = substr($url["path"], 1);

$baseName = str_replace('/web', '', $url["path"]);

$smasher = new Smasher($options['conf']);

if(preg_match("/(common|ie)\.js$/i", $path)) {
    header("Content-Type:application/javascript");
    $group = preg_replace("/\//", '.', str_replace(array('/js/', '.js'), '', $baseName));

    echo $smasher->build($group, 'js');
}
else if(preg_match("/common\.css$/i", $path)) {
    header("Content-Type:text/css");
    $group = preg_replace("/\//", '.', str_replace(array('/css/', '.css'), '', $baseName));

    echo $smasher->build($group, 'css');
}
else if(preg_match("/tpl\.html$/i", $path)){
    header("Content-Type:text/html");
    $group = preg_replace("/\//", '.', str_replace(array('/tpl/', '.html'), '', $baseName));

    echo $smasher->build($group, 'tpl');
}
else if(preg_match("/\.js$/i", $path)) {
    if (!file_exists($path)) {
        throw new Exception("File: ".$path." doesn't exists!");
    }
    header("Content-Type: application/javascript");
    echo file_get_contents($path);
}
else if(preg_match("/\.css$/i", $path)) {
    if (!file_exists($path)) {
        throw new Exception("File: ".$path." doesn't exists!");
    }
    header("Content-Type: text/css");
    echo file_get_contents($path);
}
else if($path == "web/")
{
    if (!file_exists("index.html")) {
        throw new Exception("File: index.html doesn't exists!");
    }
    header("Content-type: text/html");
    echo file_get_contents('index.html');
}
else {
    throw new Exception("Page Not Found!");
}
