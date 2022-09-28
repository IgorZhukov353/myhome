<?php
// izh 2019-11-01

require_once "bbt.php"; // подключаем класс
try{
$api_key    = 'qrkhWIhtxkYu8KYwmtyIxyAc';
$secret_key = 'KJxy285VsrVWG0rJ0yhiEzfcxCHiWWOx';
$bbt = new Beebotte($api_key, $secret_key);

//$val = $bbt->read("Arduino", "temp");
$bbt->write("Arduino", "temp", 99);
echo 'sended! val=' . $val;
}
catch (Exception $e) {
    echo 'Error :' . $e->getMessage() . '<br />';
    echo 'File :' . $e->getFile() . '<br />';
    echo 'Line :' . $e->getLine() . '<br />';
}

?>