<html>
<head>
<title>MyHome</title>
<meta content="text/html; charset=UTF-8" http-equiv="Content-Type">
<link rel="stylesheet" href="mystyle.css" type="text/css"/>
</head>
<body>

<?php 
date_default_timezone_set("Europe/Moscow");
include("log/login_info.php");
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    exit();
}
mysqli_set_charset($link, 'utf8');
?>
<div id=demotext style="text-align: center;width: 400px;">Привет, сегодня 
<?php 
$now = new DateTime(date("Y-m-d H:i:s"));
echo $now->format("d.m.Y H:i:s");
?>
</div>
<br/>
<table class="blueTable" style="width: 400px;" border="1">
  <thead>
<tr>
<th style="width: 100px; text-align: center;">Дата</td>
<th style="width: 300px; text-align: center;">Команда</td>
</tr>
  </thead>  
<tbody>
<?php
try {
    $sql = "SELECT *,case when online=1 then concat(case when d > 0 then concat(d,'d ') else '' end, case when h > 0 then concat(h,'h ') else '' end, case when m > 0 then concat(h,'m ') else '' end, s, 's') else null end str, date, code, command_id , online , change_online
        from ( SELECT 
        ONLINE_TOTAL_TIME div (24 * 60 * 60 * 1000) d,
        (ONLINE_TOTAL_TIME % (24 * 60 * 60 * 1000)) div (60 * 60000) h,
        ((ONLINE_TOTAL_TIME % (24 * 60 * 60 * 1000)) % (60 * 60000)) div (60000) m,
        (((ONLINE_TOTAL_TIME % (24 * 60 * 60 * 1000)) % (60 * 60000)) % (60000)) div 1000 s,
        DATE_FORMAT( case when change_online=1 then LAST_CHANGE_ONLINE_DATE else LAST_START_DATE end,'%d.%m.%y %T') date, 
        code, command_id , online , change_online FROM COMMAND2 limit 20) src";
        $result = mysqli_query($link,$sql);
        while ($row = mysqli_fetch_object($result)) {
?>
<tr>
<td style="width: 100px;text-align: center;"><?php echo $row->date?></td>
<td style="width: 300px;text-align: left;">
    <div style="display:table-cell;text-align:left; width:300px;height:20px;vertical-align: middle;">
    <div style="display:inline-block;text-align:left; width:20px; height:20px;vertical-align: middle;">
        <?php echo ($row->change_online)? '<img src="gif/gif_sec.gif" width="20px" height="20px" vertical-align= "middle"/>': (($row->online)? '<img src="gif/gif_gear.gif" width="20px" height="20px" vertical-align= "middle"/>':'');?>
        
    </div>
	<div style="display:inline-block;border: 0px solid #AAAAAA;text-align:left; width:180px; height:20px;vertical-align: middle;">
	    <?php echo $row->code, ' ','<span style="color:red;font-size: 10px;">',$row->str,'</span>' ?>
	</div>
	<div style="display:inline-block;border: 0px solid #AAAAAA;text-align:left;vertical-align: middle; width:70px; height:20px;">
	    <form action="upd/cmd_again2.php">
	    <input type="hidden" name="id" value="<?php echo $row->command_id; ?>" />
	    <input type="hidden" name="change_online" value="<?php echo $row->change_online; ?>" />
	    <button class=myButton style="align-self: center;line-height: 10px;" type="submit"><?php echo ($row->change_online)? "Отмена":(($row->online)?"Остановить":"Выполнить");?> </button>
	    </form>
	</div>
	</div>
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