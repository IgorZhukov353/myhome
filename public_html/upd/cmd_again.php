<?php
// izh 2018-03-27

include("../log/login_info.php");
 
//connection to the database
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
	exit();
    	}

try 
{
	mysqli_set_charset($link, 'utf8');
	$result = mysqli_query($link, "SET time_zone = '+03:00'");
	 
        if(isset($_GET['id']) != false){
		$id = $_GET['id'];		
       		$result = mysqli_query($link,"update Command set Completed_Date = NULL where command_id =". $id );
		if(!$result){
			throw new Exception("Ошибка обновления записи в Сommand." . PHP_EOL . "id=" . $id. "!" . PHP_EOL . "MySQL_Error=". mysqli_error($link), 1);
			}
	    	}
}

catch (Exception $e) {
    echo 'Error :' . $e->getMessage() . '<br />';
    echo 'File :'  . $e->getFile() . '<br />';
    echo 'Line :'  . $e->getLine() . '<br />';

    $result = mysqli_query($link, "INSERT INTO Errors(Message) VALUES (\"". $e->getMessage() . "\")")
       or die("Cannot insert into Errors!". PHP_EOL . "MySQL_Error=". mysqli_error($link));
    }
 
mysqli_close($link);
//header ('Location: myhome.php');  // перенаправление на нужную страницу
header("Location: ".$_SERVER['HTTP_REFERER']);
?>