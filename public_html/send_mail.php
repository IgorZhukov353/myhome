<?php
// izh 2018-03-19

// 24683.databor.pw/send_mail.php
// f0195241.xsph.ru/send_mail.php
// $sended2 = mail("igorjukov353@ya.ru", "from site","test");
// echo "sended=" . $sended2;
// exit();

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
			//echo "сообщ=". str_replace("\n","<br />",$msg) . "<br />";	
			mysqli_free_result($link);
      			}
    		} while (mysqli_next_result($link));
	}
 
	if(isset($msg) && strlen($msg) > 0){
    		$sended = mail("igorjukov353@ya.ru", "from site", $msg);
	    	if($sended){
			$result = mysqli_query($link,"INSERT INTO Mail_Sended(Msg) VALUES ('" . mysqli_real_escape_string($link, $msg) . "')");
			if(!$result){
				throw new Exception("Ошибка добавления записи в Mail_Sended." . PHP_EOL . "MySQL_Error=". mysqli_error($link), 1);
				}
			mysqli_free_result($link);
		
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
?>