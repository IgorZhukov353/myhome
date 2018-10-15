<?php
// izh 2018-03-19
// http://24683.databor.pw/send_event.php?id=1&text=ehbchbehbchebhbh

include("log/login_info.php");
 
$link = mysqli_connect($hostname, $username, $password);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    	exit();
	}

try {  
	$selected = mysqli_select_db($link , $dbname);
	mysqli_set_charset($link, 'utf8');
	$result = mysqli_query($link, "SET time_zone = '+03:00'");
  

        if(isset($_GET['id']) != false || isset($_GET['text']) != false){
            	$id = (isset($_GET['id']) != false)? $_GET['id'] : "0";
            	$text = (isset($_GET['text']) != false)? "\"". $_GET['text']. "\"" : "NULL" ;
            	$result = mysqli_query($link,"INSERT INTO Event(Event_Type_ID,Dop_Info) VALUES (" .$id. "," . $text . ")");
            	if($result <> true)
                	throw new Exception("Ошибка добавления записи в Event." . PHP_EOL . "Значения:" . $id. "," . $text . "!"  . PHP_EOL . "MySQL_Error=". mysqli_error($link) , 1);
        	}
    }
catch (Exception $e) {
    echo 'Error :' . $e->getMessage() . '<br />';
    echo 'File :'  . $e->getFile()    . '<br />';
    echo 'Line :'  . $e->getLine()    . '<br />';

    $result = mysqli_query($link, "INSERT INTO Errors(Message) VALUES (\"". $e->getMessage() . "\")")
       or die("Cannot insert into Errors!". PHP_EOL . "MySQL_Error=". mysqli_error($link));
    }
 
 mysqli_close($link);
?>