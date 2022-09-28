<?php
// izh 2020-06-03 (C)
// регулярная отправка информации в beebote

require_once "bbt.php"; // подключаем класс


include("log/login_info.php");
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    	exit();
	}
mysqli_set_charset($link, 'utf8');
$result = mysqli_query($link, "SET time_zone = '+03:00'");

try {
	$bbt = new Beebotte($beebotte_api_key, $beebotte_secret_key);

	// Execute multi query
	if (mysqli_multi_query($link,"CALL Get_Info_Beebote();")){
		do {
	    	// Store first result set
    		if($result = mysqli_store_result($link)){
			    while ($row = mysqli_fetch_row($result)){
			        $res = $row[0];
			        $val = (int)$row[1];
				    //echo $res . "=" . $val;
					$bbt->write("MyHome", $res, $val); /*izh 2019-11-14*/
				    }
			    mysqli_free_result($result);
      			}
      		if(mysqli_more_results($link))	
      		    mysqli_next_result($link);
      		else
      		    break;
    		} while (1);
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