<?php
require_once "SendMailSmtpClass.php"; // подключаем класс

// created izh 2018-10-31
// changed izh 2022-09-28

//header('Location: http://igorzhukov353.h1n.ru/send_mail.php');
//$sended2 = mail("igorjukov353@ya.ru", "from site","test");
//$sended2 = mail("igorjukov353@ya.ru", "igorjukov353@ya.ru","test");
//echo "sended=" . $sended2;

$sended = sendYandexMail("test");
echo "sended=" . $sended;
exit();

function sendYandexMail($msg)
{
include("log/login_info.php");
$mailSMTP = new SendMailSmtpClass($mail_login, $mail_pass, $mail_host, $mail_from, $mail_port);

// заголовок письма
$headers= "MIME-Version: 1.0\r\n";
$headers .= "Content-type: text/html; charset=utf-8\r\n"; // кодировка письма
$headers .= "From: MyHome <igorjukov353@yandex.ru>\r\n"; // от кого письмо
$server_name = "igorzhukov353.h1n.ru"; // откуда устанавливаем соединение
$result =  $mailSMTP->send($mail_login, 'from igorzhukov353.h1n.ru', $msg, $headers, $server_name); // отправляем письмо
if($result === true){
    echo "Письмо успешно отправлено";
}
else{
    echo "Письмо не отправлено. Ошибка: " . $result;
    }
}
?>