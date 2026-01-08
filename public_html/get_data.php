<?php
// =============================================
// Author:   <Жуков Игорь>
// Create date: <01.12.2023>
// Change date: <31.07.2024>
// Description: <>
// Параметр:
// =============================================

include("log/login_info.php");
 
$link = mysqli_connect($hostname, $username, $password, $dbname);
if (mysqli_connect_errno()) {
	printf("Не удалось подключиться: %s\n", mysqli_connect_error());
    exit();
}

try {  
	mysqli_set_charset($link, 'utf8');
	$result = mysqli_query($link, "SET time_zone = '+03:00'");
	//echo date("d-m-Y H:i:s");

	$output = array();
	$output['curdate'] = date("d-m-Y H:i:s");
    $result = mysqli_query($link, "SELECT Date_Str,Name,Temp_Value,Humidity_Value,Alarm, min_temp, max_temp, avg_temp FROM V_LAST_TEMP_HUM");
    while ($row = mysqli_fetch_object($result)) {
        $temp = $row->Temp_Value . (($row->Temp_Value != "--")? "°":"");
        $hum = ($row->Humidity_Value > 0)? ($row->Humidity_Value . "%"):"--";
		$output['temp'][] = ['time'=>$row->Date_Str,"name"=>$row->name,'alarm'=>$row->Alarm,'temp'=>$temp,'hum'=>$hum,'avg'=>(isset($row->min_temp))?($row->min_temp."°/". $row->max_temp."°/". $row->avg_temp."°"):""];
		//echo $row->Date_Str,' ',$row->name,' ',$row->Temp_Value,(($row->Temp_Value!='--')?"°":"") . "/". (($row->Humidity_Value > 0)?$row->Humidity_Value . "%":""),( $row->min_temp."°/". $row->max_temp."°/". $row->avg_temp."°");
	}
    mysqli_free_result($result);
    

    //$result = mysqli_query($link, "SELECT Sensor_Name,Count,Last,First FROM V_LAST_SENSOR_ACTIVITY where Date = CURDATE() limit 10");
            $result = mysqli_query($link, "SELECT DATE_FORMAT(Date,'%d.%m.%Y') as Date,Sensor_Name,Count,Last,First,LastValue,FirstValue FROM V_LAST_SENSOR_ACTIVITY3 where Date = CURDATE() limit 10");
    while ($row = mysqli_fetch_object($result)) {
            $output['sens'][] = ["name"=>$row->Sensor_Name,'cnt'=>$row->Count,'last'=>$row->Last,'first'=>$row->First,
            'firstvaluetrue'=>$row->FirstValue==1?1:NULL,
            //'firstvaluefalse'=>$row->FirstValue==0?1:NULL,
            'lastvaluetrue'=>$row->LastValue==1?1:NULL
            //'lastvaluefalse'=>$row->LastValue==0?1:NULL
            ];
		}
    mysqli_free_result($result);

    $result = mysqli_query($link, "SELECT DATE_FORMAT(Date,'%d.%m.%y %T') as Date,Dop_Info, TIMESTAMPDIFF(minute,Date,SYSDATE()) as diff FROM V_LAST_WATCHDOG_EVENT limit 5");
    for($i=0;;$i++)
    {
        $row = mysqli_fetch_object($result);
        if(!$row)
            break;
        if($i==0){
            $output['last_watchdog_color'] = ($row->diff > 60)?"red":"green";
            $output['last_watchdog_time']  = $row->Date;
        }
        $output['watchdog'][] = ["time"=>$row->Date,'info'=>$row->Dop_Info];
  	}
    mysqli_free_result($result);

    $result = mysqli_query($link, "SELECT DATE_FORMAT(Date,'%d.%m.%y %T') as Date,TIMESTAMPDIFF(minute,Date,SYSDATE()) as diff, TIMESTAMPDIFF(minute,SYSDATE(),ADDTIME (Date,'0:10:0.0')) + 1 FROM Event where Event_Type_ID=10");
    $row = mysqli_fetch_array($result, MYSQLI_NUM);
    if($row){
        $str2 = $row[0];
        $diff = $row[1];
        if($diff > 10)
	        $str ="red";
        else{
            $str2 .= " (". $row[2] ."...)";
	        $str ="green";
        }
        $output['last_getcmd_color']  = $str;
        $output['last_getcmd_time']  = $str2;
    }
    mysqli_free_result($result);

    $sql = "SELECT *,get_date_str(ONLINE_TOTAL_TIME) str, DATE_FORMAT( case when change_online=1 then LAST_CHANGE_ONLINE_DATE else LAST_START_DATE end,'%d.%m.%y %T') date
            FROM COMMAND2 where s_flag=1 order by LAST_START_DATE desc limit 20";
/*    $sql = "SELECT *,case when online=1 then concat(case when d > 0 then concat(d,'d ') else '' end, case when h > 0 then concat(h,'h ') else '' end, case when m > 0 then concat(h,'m ') else '' end, s, 's') else null end str, date, code, command_id , online+0 as online , change_online
        from ( SELECT 
        ONLINE_TOTAL_TIME div (24 * 60 * 60 * 1000) d,
        (ONLINE_TOTAL_TIME % (24 * 60 * 60 * 1000)) div (60 * 60000) h,
        ((ONLINE_TOTAL_TIME % (24 * 60 * 60 * 1000)) % (60 * 60000)) div (60000) m,
        (((ONLINE_TOTAL_TIME % (24 * 60 * 60 * 1000)) % (60 * 60000)) % (60000)) div 1000 s,
        DATE_FORMAT( case when change_online=1 then LAST_CHANGE_ONLINE_DATE else LAST_START_DATE end,'%d.%m.%y %T') date, 
        code, command_id , online , change_online FROM COMMAND2 limit 20) src";
*/        
    //echo $sql;
    // "SELECT DATE_FORMAT(Completed_Date,'%d.%m.%y %T') as Completed_Date, Code, Command_id FROM Command order by cast(Completed_Date as datetime) desc limit 20"
    $result = mysqli_query($link, $sql);
    while ($row = mysqli_fetch_object($result)) {
        $output['cmd'][] = ["time"=>$row->date,'text'=>$row->CODE, 'str'=>$row->str,'online'=>$row->ONLINE , 'change_online'=>$row->CHANGE_ONLINE, 'command_id'=>$row->COMMAND_ID];
        
		}        
	mysqli_free_result($result);
	
    echo json_encode($output, JSON_UNESCAPED_UNICODE | JSON_NUMERIC_CHECK); 
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
