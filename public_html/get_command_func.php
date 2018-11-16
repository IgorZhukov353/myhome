<?php
// izh 2018-11-16
 
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

	// Execute multi query
	if (mysqli_multi_query($link,"CALL Get_Command();")){
		do {
	    	// Store first result set
    		if($result = mysqli_store_result($link)){
			    while ($row = mysqli_fetch_row($result)){
				    $cmd = $row[0];
				    }
			    mysqli_free_result($result);
      			}
    		} while (mysqli_next_result($link));
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