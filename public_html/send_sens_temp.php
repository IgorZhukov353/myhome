<?php
// izh 2018-04-03

// 24683.databor.pw//send_sens_temp.php?t=S&id1=1&v1=1
// 24683.databor.pw//send_sens_temp.php?t=T&id1=1&temp1=10&hum1=80


include("log/login_info.php");
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    	exit();
	}
mysqli_set_charset($link, 'utf8');
$result = mysqli_query($link, "SET time_zone = '+03:00'");

try {
    	for($i=1; $i < 100; $i++){
		$type = "t".$i;
		if( isset($_GET[$type]) == false)
				break;
        	$id = "id" .$i;
		if( isset($_GET[$id]) == false)
				break;
		$type = $_GET[$type];				
		if($type == "S"){
			$v = "v" .$i;
			if( isset($_GET[$id]) == false || isset($_GET[$v]) == false)
				break;
			$result = mysqli_query($link,"INSERT INTO Sensor_Activity(Sensor_ID,Value) VALUES (" . $_GET[$id]. "," . $_GET[$v] . ")");
			if(!$result)
				throw new Exception("Ошибка добавления записи в Sensor_Activity." . PHP_EOL . "Значения: id=" . $_GET[$id]. "," . $_GET[$v] . "!"  . PHP_EOL . "MySQL_Error=".  mysqli_error($link) , 1);
		}
		else if($type == "T"){
			$t = "temp" .$i;
			$h = "hum" .$i;
			if( isset($_GET[$t]) == false || isset($_GET[$h]) == false)
				break;
			$result = mysqli_query($link,"INSERT INTO Thermometer_Info(Thermometer_ID,Temp_Value,Humidity_Value) VALUES (" . $_GET[$id]. "," . $_GET[$t]. "," .$_GET[$h] . ")");
            		if(!$result)
                		throw new Exception("Ошибка добавления записи в Thermometer_Info." . PHP_EOL . "Значения: id=" . $_GET[$id]. "," . $_GET[$t]. "," .$_GET[$h] . "!"  . PHP_EOL . "MySQL_Error=". mysqli_error($link) , 1);
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