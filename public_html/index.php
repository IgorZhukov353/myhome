<html>
<title>MyHome</title>
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
/* Igor Zhukov (C) 2018-04-13 */
date_default_timezone_set("Europe/Moscow");
$now = new DateTime(date("Y-m-d H:i:s"));
echo $now->format("d.m.Y H:i:s");
?>
</div>
<img src="river1.png" alt="" width="400" height="200" />
</body>
</html>