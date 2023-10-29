<?php
// izh 2019-11-14 (C)
// через POST или GET
/* пример запроса
POST /upd/send_info.php HTTP/1.1
Host: f0195241.xsph.ru
Content-Type: application/x-www-form-urlencoded
Authorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==
Cache-Control: no-cache

str=[{"type":"S","id":1,"v":1},{"type":"T","id":1,"temp":12,"hum":80},{"type":"E","text":"hello, world!"},{"type":"E","id":6,"text":"test 6"}]
*/
//require_once "bbt.php"; // подключаем класс

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
//	$api_key    = 'qrkhWIhtxkYu8KYwmtyIxyAc';	/*izh 2019-11-01*/
//	$secret_key = 'KJxy285VsrVWG0rJ0yhiEzfcxCHiWWOx';
//	$bbt = new Beebotte($api_key, $secret_key);

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
        	
        $date = (isset($result_parse[$i]->date) == true)? $result_parse[$i]->date: null;

		if($type == "S"){
			$value = $result_parse[$i]->v;
			if( isset($value) == false)
				break;
			if( $id == 0)
				break;
//			if($id > 2)	
//			    $bbt->write("MyHome", "s".$id, $value); /*izh 2019-11-14*/
			
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
			
//			if( $t > -100) /* izh 2019-12-2 ошибка датчика */
//			    $bbt->write("MyHome", "t".$id, $t); /*izh 2019-11-01*/
			
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
            		$text = (isset($result_parse[$i]->text) == true)? $result_parse[$i]->text : null ;

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