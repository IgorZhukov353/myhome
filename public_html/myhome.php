<html>
<head>
<title>MyHome V1</title>
<meta content="text/html; charset=UTF-8" http-equiv="Content-Type">
<link rel="stylesheet" href="mystyle.css" type="text/css"/>
</head>
<body>
<!--
<div id="demo2">
<img src="http://f0195241.xsph.ru/river1.png" alt="" width="100" height="100" />
</div>
-->
<div id=demotext style="text-align: center;width: 400px;">Привет, сегодня 
<?php 
/* Igor Zhukov (C) 2018-04-13 
	Last changed: 22-06-2024*/
date_default_timezone_set("Europe/Moscow");
$now = new DateTime(date("Y-m-d H:i:s"));
echo $now->format("d.m.Y H:i:s");
?>
</div>
<br/>
<table class="blueTable" style="width: 400px;" border="1">
<caption>Текущие температура и влажность.</caption>
  <thead>
    <tr>
<td style="width: 140px; text-align: center;">Дата</td>
<td style="width: 130px; text-align: center;">Место</td>
<td style="width: 140px; text-align: center;">Темп/Влажн</td>
<td style="width: 140px; text-align: center;">Мin/Max/Avg</td>
    </tr>
  </thead>
<tbody>
<?php
// izh 2018-03-19
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
        $result = mysqli_query($link, "SELECT Date_Str,Name,Temp_Value,Humidity_Value,Alarm, min_temp, max_temp, avg_temp FROM V_LAST_TEMP_HUM");
        while ($row = mysqli_fetch_array($result, MYSQLI_NUM)) {
?>
<tr>
<td style="width: 80px; text-align: center;"><?php echo $row[0] ?></td>
<td style="width: 130px; text-align: center;">
<?php echo $row[1] ?></td>
<td style="width: 140px; text-align: center;">
<div class=bblock2 style="width: 30px; text-align: center;">
<?php 
if($row[4] == 1)
 	echo "<span style=\"color:red\">&#128226;</span>";
?>
</div>
<div class=bblock2>
<?php 
if($row[4] == 1)
 	echo "<span style=\"color:red\">". $row[2] ."&#186;</span>";
else 	
    if($row[2] == '--')
        echo '--';
    else
        echo $row[2]. "&#186;";
?> / <?php echo ($row[3] == 0)? "--": $row[3] . "%" ?>
</div></td>

<td style="width: 140px; text-align: center;">
<?php 
if(isset($row[5])) 
    echo $row[5] . "&#186 / ". $row[6] . "&#186 / ". $row[7] . "&#186" 
?>
</td>
</tr>
<?php            }
        mysqli_free_result($result);
?>
</tbody>
</table>
<br/>
<!--details> <summary>Срабатывания датчиков.</summary-->
<table class="blueTable" style="width: 400px;" border="1">
<caption>Срабатывания датчиков.</caption>
  <thead>
<tr>
<!--td style="width: 60px; text-align: center;">Дата</td-->
<td style="width: 120px; text-align: center;">Место</td>
<td style="width: 50px; text-align: center;">Кол-во</td>
<td style="width: 80px; text-align: center;">Посл/Перв</td>
</tr>
  </thead>  

<tbody>
<?php
        $result = mysqli_query($link, "SELECT DATE_FORMAT(Date,'%d.%m.%Y') as Date,Sensor_Name,Count,Last,First,LastValue,FirstValue FROM V_LAST_SENSOR_ACTIVITY3 where Date = CURDATE() limit 10");
        while ($row = mysqli_fetch_array($result, MYSQLI_NUM)) {
?>
<tr>
<!--td style="width: 60px;text-align: center;"><?php echo $row[0] ?></td-->
<td style="width: 120px;text-align: center;"><?php echo $row[1] ?></td>
<td style="width: 50px;text-align: center;"><?php echo $row[2] ?></td>
<td style="width: 80px;text-align: center;"><?php echo $row[3];
if($row[5] == 1)
 	echo "<span style=\"color:green\">①</span>";
else
    echo "<span style=\"color:red\">⓪</span>";
echo " / ".$row[4];
if($row[6] == 1)
 	echo "<span style=\"color:green\">①</span>";
else
    echo "<span style=\"color:red\">⓪</span>";

?></td>
</tr>
<?php            }
        mysqli_free_result($result);
?>
</tbody>
</table>
<!--/details-->
<br/>
<?php
        $result = mysqli_query($link, "SELECT DATE_FORMAT(Date,'%d.%m.%y %T') as Date,Dop_Info, TIMESTAMPDIFF(minute,Date,SYSDATE()) as diff FROM V_LAST_WATCHDOG_EVENT limit 5");
        $row = mysqli_fetch_array($result, MYSQLI_NUM);
?>

<details> <summary>Деж сообщения. Последнее: 
<?php 
$diff = $row[2];
if($diff > 60)
	$str ="red";
else
	$str ="green";
echo "<span style=\"color:". $str . ";\">". $row[0]. "</span>";
?></summary>
<table class="blueTable" style="width: 400px;" border="1">
  <thead>
<tr>
<td style="width: 130px; text-align: center;">Дата</td>
<td style="width: 270px; text-align: center;">Инфо</td>
</tr>
  </thead>  
<tbody>
<?php
        while ($row) {
?>
<tr>
<td style="width: 130px;text-align: center;"><?php echo $row[0] ?></td>
<td style="width: 270px;"><?php echo $row[1] ?></td>
</tr>
<?php          
		$row = mysqli_fetch_array($result, MYSQLI_NUM);
  		}
        mysqli_free_result($result);
?>
</tbody>
</table>
</details>
<br/>
<?php
        $result = mysqli_query($link, "SELECT DATE_FORMAT(Date,'%d.%m.%y %T') as Date,TIMESTAMPDIFF(minute,Date,SYSDATE()) as diff, TIMESTAMPDIFF(minute,SYSDATE(),ADDTIME (Date,'0:10:0.0')) + 1 FROM Event where Event_Type_ID=10");
        $row = mysqli_fetch_array($result, MYSQLI_NUM);
?>

<details> <summary>Выполн. команд. Послед:
<?php 
$diff = $row[1];
$str2 = "";
if($diff > 10)
	$str ="red";
else{
    $str2 = " (". $row[2] ."...)";
	$str ="green";
}
echo "<span style=\"color:". $str . ";\">". $row[0]. $str2."</span>";
?>
</summary>
<table class="blueTable" style="width: 400px;" border="1">
  <thead>
<tr>
<td style="width: 130px; text-align: center;">Дата выполнения</td>
<td style="width: 270px; text-align: center;">Команда</td>
</tr>
  </thead>  

<tbody>
<?php
        $result = mysqli_query($link, "SELECT DATE_FORMAT(Completed_Date,'%d.%m.%y %T') as Completed_Date, Code, Command_id FROM Command order by cast(Completed_Date as datetime) desc limit 20");
        while ($row = mysqli_fetch_array($result, MYSQLI_NUM)) {
?>
<tr>
<td style="width: 130px;text-align: center;"><?php echo $row[0] ?></td>
<td style="width: 270px;text-align: center;">
	<div class=bblock14><?php echo $row[1] ?></div>
	<div class=bblock14><form action="upd/cmd_again.php">
	    <input type="hidden" name="id" value="<?php echo $row[2] ?>" />
	    <button class=myButton type="submit">Повторить</button>
	</form></div>
    </td>
</tr>
<?php            }
        mysqli_free_result($result);
?>  
</tbody>
</table>
</details>
<?php
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

</body>
</html>