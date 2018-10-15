<?php
// 24683.databor.pw/get_command.php

include("log/login_info.php");
include("get_command_func.php");

$r = getCommand($hostname, $username, $password, $dbname);
if($r != "")
    echo "command=" . $r .";";
?>