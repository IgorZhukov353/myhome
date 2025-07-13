<?php
// izh 2018-03-19
// https://igorzhukov353.000webhostapp.com/get_param.php

include("log/login_info.php");
 
//connection to the database
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    	exit();
	}

try {  
	mysqli_set_charset($link, 'utf8');
	$result = mysqli_query($link, "SET time_zone = '+03:00'");
  
        $result = mysqli_query($link, "SELECT Name,VALUE FROM Setup_Parameter WHERE Active_Flag = 1");
        while ($row = mysqli_fetch_array($result, MYSQLI_NUM)) {
            printf("%s=%s\n\r", $row[0], $row[1]);
            }
        mysqli_free_result($result);

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