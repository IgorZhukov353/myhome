<?php
// izh 2019-11-14 (C)
// last update 2024-07-22
// через POST или GET
/* пример запроса
POST /upd/send_info.php HTTP/1.1
Host: f0195241.xsph.ru
Content-Type: application/x-www-form-urlencoded
Authorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==
Cache-Control: no-cache

str=[{"type":"S","id":1,"v":1},{"type":"T","id":1,"temp":12,"hum":80},{"type":"E","text":"hello, world!"},{"type":"E","id":6,"text":"test 6"}]
*/

if(isset($_POST["str"]) == true)
	$str = $_POST["str"];
else
	if(isset($_GET["str"]) == true)
		$str = $_GET["str"];
	else	{
		printf("Не правильный формат запроса!\n");
		exit();
	}

$result_parse = json_decode($str);

if( json_last_error() != JSON_ERROR_NONE){
	printf("Ошибочный формат JSON! json_last_error = %d.\n", json_last_error());
	exit();
	}

/*
for($i = 0; $i < count($result_parse); $i++){
$date = (isset($result_parse[$i]->date)==true)?$result_parse[$i]->date: date("Y-m-d H:i:s");
echo "1.type=".$result_parse[$i]->type . " date=" . $date . "\n";
}
exit();
*/

include("../log/login_info.php");
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    	exit();
	}
mysqli_set_charset($link, 'utf8');
$result = mysqli_query($link, "SET time_zone = '+03:00'");

try {

	for($i = 0; $i < count($result_parse); $i++){
		$type = $result_parse[$i]->type;
		if( isset($type) == false)
			break;
		if( isset($result_parse[$i]->id) == false)
			if($type == "E")
				$id = 7;
			else
				break;
		else
			$id = $result_parse[$i]->id;
        	
        $date = (isset($result_parse[$i]->date) == true)? $result_parse[$i]->date: date("Y-m-d H:i:s");
        //echo $result_parse[$i]->date . '---'. date("Y-m-d H:i:s");

		if($type == "S"){
			$value = $result_parse[$i]->v;
			if( isset($value) == false)
				break;
			if( $id == 0)
				break;
			
			//echo "type=S id=". $id. " date=" . $date . " v=".$value . "\n";
			if ($stmt = mysqli_prepare($link, "INSERT INTO Sensor_Activity(Sensor_ID,Value,Date) VALUES (?,?,?)")) {
    				mysqli_stmt_bind_param($stmt, "iis", $id, $value, $date); /* bind parameters for markers */
				mysqli_stmt_execute($stmt);	/* execute query */
				if(mysqli_stmt_errno($stmt))
					throw new Exception("Ошибка добавления записи в Sensor_Activity." . PHP_EOL . "Значения: id=" . $id. "," . $value . "!"  . PHP_EOL . "MySQL_Error=".  mysqli_stmt_error($stmt) , 1);
    				mysqli_stmt_close($stmt);	/* close statement */
				} 
			}
		else if($type == "T"){
			$t = $result_parse[$i]->temp;
			$h = $result_parse[$i]->hum;
			if( isset($t) == false || isset($h) == false)
				break;
			if(  $id == 0)
				break;
			
			//echo "type=T id=". $id. " date=" . $date . " t=".$t . " h=".$h ."\n";
			if ($stmt = mysqli_prepare($link, "INSERT INTO Thermometer_Info(Thermometer_ID,Temp_Value,Humidity_Value,Date) VALUES (?,?,?,?)")) {
    				mysqli_stmt_bind_param($stmt, "iiis", $id, $t, $h, $date); /* bind parameters for markers */
				mysqli_stmt_execute($stmt);	/* execute query */
				if(mysqli_stmt_errno($stmt))
					throw new Exception("Ошибка добавления записи в Thermometer_Info." . PHP_EOL . "Значения: id=" . $id. "," . $t. "," .$h  . "!"  . PHP_EOL . "MySQL_Error=".  mysqli_stmt_error($stmt) , 1);
    				mysqli_stmt_close($stmt);	/* close statement */
				} 
			}
		else if($type == "E"){	
            if (isset($result_parse[$i]->text))
            {
                if ($id == 8 && is_object($result_parse[$i]->text)) // обработка команды
                {
                    $command = $result_parse[$i]->text;
					
					/* [{"type":"E","id":8,"text":{"id":"fill_tank","l":1799998,"cnt":1,"w":1,"a":1,"actt":1},"date":"2024-06-24 19:11:10"}] */
					
					$command_id = $command->id;
	                $REMAIN_TIME = round($command->l / 1000,0);
					$CURRENT_ACTIVE_COUNT = $command->cnt;
					$ONLINE = $command->w;
					$ACTIVE = $command->a;
					$CURRENT_ACTIVE_TIME = round($command->actt / 1000,0);
					$CURRENT_ONLINE_TIME = round($command->ont / 1000,0);
					if(isset($command->dopopt))
					    $DOP_INFO = json_encode($command->dopopt);
					
					$sql = "SELECT change_online,online FROM COMMAND2 WHERE COMMAND_ID=" . $command_id;
					$stmt = mysqli_query($link, $sql);
					$row = mysqli_fetch_object($stmt);
					$table_change_online = $row->change_online;
					$table_online = $row->online;
					if (($stmt instanceof mysqli_stmt)) 
					    mysqli_stmt_close($stmt);

					$sql = "update COMMAND2 set ";
					if($ONLINE){
						if(!$table_online){ // запуск команды
							$sql .= "ONLINE=1,LAST_START_DATE=sysdate(),CHANGE_ONLINE=0,ONLINE_COUNT=ONLINE_COUNT+1,ACTIVE=$ACTIVE";
						}
						else {	// продолжение работы команды
							$sql .= "REMAIN_TIME=$REMAIN_TIME,CURRENT_ACTIVE_TIME=$CURRENT_ACTIVE_TIME,CURRENT_ACTIVE_COUNT=$CURRENT_ACTIVE_COUNT,CURRENT_ONLINE_TIME=$CURRENT_ONLINE_TIME,ACTIVE=$ACTIVE";
						}
					}
					else{
						if($table_online){ // остановка команды
							$sql = $sql . "ONLINE=0,CHANGE_ONLINE=0,REMAIN_TIME=0,CURRENT_ACTIVE_TIME=0,CURRENT_ACTIVE_COUNT=0,ACTIVE=0,CURRENT_ONLINE_TIME=0,".
							"ONLINE_TOTAL_TIME=ONLINE_TOTAL_TIME+$CURRENT_ONLINE_TIME,".
							"ACTIVE_TOTAL_TIME=ACTIVE_TOTAL_TIME+$CURRENT_ACTIVE_TIME,".
							"ACTIVE_COUNT=ACTIVE_COUNT+$CURRENT_ACTIVE_COUNT";
						}
					}
					if(isset($$DOP_INFO))
				        $sql .= ",DOP_INFO='$DOP_INFO'";
					$sql .= " where COMMAND_ID =$command_id";
					//echo( $sql);
					$stmt = mysqli_query($link,$sql);
					if (($stmt instanceof mysqli_stmt)) 
					    mysqli_stmt_close($stmt); /* close statement */
					
					$text = json_encode($result_parse[$i]->text);
					}
                else
                {
                    if (is_object($result_parse[$i]->text)) $text = json_encode($result_parse[$i]->text);
                    else $text = $result_parse[$i]->text;
                }
            }
            else $text = null;

                    
			//echo "type=E id=". $id. " date=" . $date . " text=". $text ."\n";
			if ($stmt = mysqli_prepare($link, "INSERT INTO Event(Event_Type_ID,Dop_Info,Date) VALUES (?,?,?)")) {
    				mysqli_stmt_bind_param($stmt, "iss", $id, $text, $date); /* bind parameters for markers */
				mysqli_stmt_execute($stmt);	/* execute query */
				if(mysqli_stmt_errno($stmt))
					throw new Exception("Ошибка добавления записи в Event." . PHP_EOL . "Значения: id=" . $id. "," . $text . "!"  . PHP_EOL . "MySQL_Error=".  mysqli_stmt_error($stmt) , 1);
    				mysqli_stmt_close($stmt);	/* close statement */
				} 
        		}
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