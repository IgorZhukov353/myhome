<?php
// izh 2018-05-07
 
function getCommand($hostname, $username, $password, $dbname)
{
$cmd = "";

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
	 
    	$result = mysqli_query($link,"select code, command_id from Command where Completed_Date is null order by command_id limit 1");
	$row = mysqli_fetch_row($result);
	//printf("Select вернул %d строк.\n". "MySQL_Error=". mysqli_error($link), mysqli_num_rows($result));

    	if($row){
        	$id = $row[1];
        	$cmd = $row[0];
        	mysqli_free_result($result);
        	$result = mysqli_query($link,"update Command set Completed_Date = now() where command_id =". $id );
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

 return $cmd;
}
?>