<?php
// =============================================
// Author:   <Жуков Игорь>
// Create date: <22.11.2023>
// Change date: <22.11.2023>
// Description: <Выдать список команд у которых есть запрос на изменение статуса работы (стартовать/осьановить)>
// Параметр: http://igorzhukov353.h1n.ru/get_command2.php?init=0
// init=0 - получить список команд для старта или остановки
// init=1 - получить список команд находящихся в работе в моммент перезагрузки системы
// =============================================

include("log/login_info.php");
if(isset($_GET["init"]) == true)
	$init = $_GET["init"];
else
	$init = 0;
 
//connection to the database
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    exit();
	}

try {  
	mysqli_set_charset($link, 'utf8');
	$result = mysqli_query($link, "SET time_zone = '+03:00'");

	if( $init == 1) // выполнять после перезагрузки системы, чтобы возобновить выполнения работающих до перезагрузки команд
		$sql = "SELECT COMMAND_ID id,CODE code,OPTIONS opt, REMAIN_TIME remain, 0 online FROM COMMAND2 WHERE S_FLAG = 1 and ONLINE = 1";
	else    // обработка запросов на старт и стоп команд
		$sql = "SELECT COMMAND_ID id,CODE code,case when ONLINE=1 then null else OPTIONS end opt,ONLINE online FROM COMMAND2 WHERE S_FLAG = 1 and CHANGE_ONLINE = 1";
//	$sql="SELECT Date,Event_ID FROM Event where Event_Type_ID=10";
	
	$result = mysqli_query($link, $sql);
	$output = array();
	while ($row = mysqli_fetch_object($result)) {
//	    echo "Date=$row->Date";
        $output[] = ['id'=>$row->id,'code'=>$row->code,'opt'=>json_decode($row->opt),'online'=>$row->online,'remain'=>$row->remain];
    }
	mysqli_free_result($result);

	//$output = filter_nulls($output);
    echo 'cmd='.json_encode($output); 
    }
    
catch (Exception $e) {
    echo 'Error :' . $e->getMessage() . '<br />';
    echo 'File :'  . $e->getFile()    . '<br />';
    echo 'Line :'  . $e->getLine()    . '<br />';

    $result = mysqli_query($link, "INSERT INTO Errors(Message) VALUES (\"". $e->getMessage() . "\")")
       or die("Cannot insert into Errors!". PHP_EOL . "MySQL_Error=". mysqli_error($link));
    }
 mysqli_close($link);

function filter_nulls($input) {
    $result = new stdClass();
    foreach ($input as $k => $v) {
        if (!is_null($v)) {
            $result->$k = is_scalar($v) ? $v : filter_nulls($v);
        }
    }
    return is_array($input) ? (array) $result : $result;
}
?>