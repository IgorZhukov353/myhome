<?php
// izh 2018-12-19
// changed 2020-12-22

require_once "SendMailSmtpClass.php"; // подключаем класс

// f0195241.xsph.ru/send_mail.php

//header('Location: http://f0195241.xsph.ru/send_mail.php');
//$sended2 = mail("igorjukov353@ya.ru", "from site","test");
//echo "sended=" . $sended2;
//$sended = sendYandexMail("test");
//echo "sended=" . $sended;
//exit();

include("log/login_info.php");
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    	exit();
	}

try {
	mysqli_set_charset($link, 'utf8');
	$result = mysqli_query($link, "SET time_zone = '+03:00'");
	
	// Execute multi query
	if (mysqli_multi_query($link,"CALL Get_Sensor_Activity();")){
		do {
	    	// Store first result set
    		if($result = mysqli_store_result($link)){
        		$msg = '';
			    while ($row = mysqli_fetch_row($result)){
				    $msg = $msg . "\n" . $row[0]. "\n" . $row[1]. "\n" . $row[2];
				    }

			    mysqli_free_result($result);
      			}
    		} while (mysqli_more_results($link) && mysqli_next_result($link));
	}
 
	if(isset($msg) && strlen($msg) > 0){
    		//$sended = mail("igorjukov353@ya.ru", "from site", $msg);
    		$sended = sendYandexMail($msg);
    		
	    	if($sended){
			    $result = mysqli_query($link,"INSERT INTO Mail_Sended(Msg) VALUES ('" . mysqli_real_escape_string($link, $msg) . "')");
			    if(!$result){
				    throw new Exception("Ошибка добавления записи в Mail_Sended." . PHP_EOL . "MySQL_Error=". mysqli_error($link), 1);
				    }
//			    mysqli_free_result($result);
		
			    $Mail_Sended_ID = mysqli_insert_id($link);
			    //echo "$Mail_Sended_ID". $Mail_Sended_ID;
		
			    $result = mysqli_query($link, "CALL Link_Sensor_Activity_2_Mail_Sended(". $Mail_Sended_ID .");");
			    if(!$result)
				    throw new Exception("Ошибка вызова Link_Sensor_Activity_2_Mail_Sended(". $Mail_Sended_ID .")." . PHP_EOL . "MySQL_Error=". mysqli_error($link) , 1);

	    		echo "Letter sended.";
		    } 
		else {
	    		$err = error_get_last()[message];
	    		throw new Exception("Ошибка отсылки сообщения." . PHP_EOL . "Error=".  $err, 1);
			}
	}
	else {
    		echo "Letter not sended.";
    	}
    
	$result = mysqli_query($link, "INSERT INTO Event(Event_Type_ID) VALUES (6)");
	if(!$result)
    		throw new Exception("Ошибка добавления записи в Event." . PHP_EOL . "Значения:6 !"  . PHP_EOL . "MySQL_Error=". mysqli_error($link) , 1);    
}
catch (Exception $e) {
    echo 'Error :' . $e->getMessage() . '<br />';
    echo 'File :' . $e->getFile() . '<br />';
    echo 'Line :' . $e->getLine() . '<br />';

    $result = mysqli_query($link, "INSERT INTO Errors(Message) VALUES (\"". $e->getMessage() . "\")")
       or die("Cannot insert into Errors!". PHP_EOL . "MySQL_Error=". mysqli_error($link));
    }
 
 mysqli_close($link);

//------------------------------------------------------------------- 
function sendYandexMail($msg)
{
$mailSMTP = new SendMailSmtpClass('igorjukov353@yandex.ru', 'UJMiop890', 'ssl://smtp.yandex.ru', 'igorjukov353@yandex.ru', 465);
// $mailSMTP = new SendMailSmtpClass('логин', 'пароль', 'хост', 'имя отправителя');
  
// заголовок письма
$headers= "MIME-Version: 1.0\r\n";
$headers .= "Content-type: text/html; charset=utf-8\r\n"; // кодировка письма
$headers .= "From: MyHome <igorjukov353@yandex.ru>\r\n"; // от кого письмо
$server_name = "igorzhukov353.h1n.ru"; // откуда устанавливаем соединение
$result =  $mailSMTP->send('igorjukov353@yandex.ru', 'from igorzhukov353.h1n.ru', $msg, $headers, $server_name); // отправляем письмо

//$mailSMTP = new SendMailSmtpClass('igorjukov353@yandex.ru', 'UJMiop890', 'ssl://smtp.yandex.ru', 'Igor', 465);
// $mailSMTP = new SendMailSmtpClass('логин', 'пароль', 'хост', 'имя отправителя');
  
// заголовок письма
//$headers= "MIME-Version: 1.0\r\n";
//$headers .= "Content-type: text/html; charset=utf-8\r\n"; // кодировка письма
//$headers .= "From: MyHome <MyHome@ya.ru>\r\n"; // от кого письмо
//$server_name = "igorzhukov353.h1n.ru"; // откуда устанавливаем соединение
//$result =  $mailSMTP->send('igorjukov353@yandex.ru', 'from site', $msg, $headers, $server_name); // отправляем письмо
//$result =  $mailSMTP->send('igor.zhukov@eaeconsult.ru', 'from site', $msg, $headers); // отправляем письмо
// $result =  $mailSMTP->send('Кому письмо', 'Тема письма', 'Текст письма', 'Заголовки письма');
/*if($result === true){
    echo "Письмо успешно отправлено";
}
else{
    echo "Письмо не отправлено. Ошибка: " . $result;
    }
*/    
return $result;    
}
 
?>