-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Хост: localhost
-- Время создания: Окт 27 2018 г., 01:12
-- Версия сервера: 10.0.35-MariaDB
-- Версия PHP: 5.4.16

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- База данных: igorzhuk_myhome
--

DELIMITER $$
--
-- Процедуры
--
CREATE DEFINER=`igorzhuk_user`@`localhost` PROCEDURE `Get_Sensor_Activity`()
    NO SQL
SELECT * FROM V_LAST_ACTIVITY$$

CREATE DEFINER=`igorzhuk_user`@`localhost` PROCEDURE `Link_Sensor_Activity_2_Mail_Sended`(IN `pMail_Sended_ID` INT)
    NO SQL
begin
UPDATE Sensor_Activity
set Mail_Sended_ID = pMail_Sended_ID
where 
	Mail_Sended_ID is null
    and Sensor_ID in (select Sensor_ID from Sensor s 
	where COALESCE(s.Send_Alarm,0) = 1);
    
UPDATE Thermometer_Info
set Mail_Sended_ID = pMail_Sended_ID
where 
	Mail_Sended_ID is null
    and Thermometer_ID in (select Thermometer_ID from Thermometer t	where COALESCE(t.Send_Alarm,0) = 1	and Temp_Value < t.Threshold_Temp);
end$$

DELIMITER ;

-- --------------------------------------------------------

--
-- Структура таблицы Command
--

CREATE TABLE IF NOT EXISTS Command (
  Command_ID int(10) unsigned NOT NULL,
  `Code` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  Started_Date datetime DEFAULT NULL,
  Completed_Date datetime DEFAULT NULL,
  Note varchar(200) COLLATE utf8mb4_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Команды для системы';

-- --------------------------------------------------------

--
-- Структура таблицы Errors
--

CREATE TABLE IF NOT EXISTS `Errors` (
  Error_ID int(10) unsigned NOT NULL,
  Message varchar(1024) COLLATE utf8mb4_unicode_ci NOT NULL,
  `Date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Ошибки возникшие в системе';

-- --------------------------------------------------------

--
-- Структура таблицы Event
--

CREATE TABLE IF NOT EXISTS `Event` (
  Event_ID int(10) unsigned NOT NULL,
  Event_Type_ID int(10) unsigned NOT NULL,
  `Date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  Dop_Info varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='События возникшие в системе';

-- --------------------------------------------------------

--
-- Структура таблицы Event_Type
--

CREATE TABLE IF NOT EXISTS Event_Type (
  Event_Type_ID int(10) unsigned NOT NULL,
  `Name` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='События, возникающие в системе';

-- --------------------------------------------------------

--
-- Структура таблицы Mail_Sended
--

CREATE TABLE IF NOT EXISTS Mail_Sended (
  Mail_Sended_ID int(10) unsigned NOT NULL,
  Msg varchar(4096) COLLATE utf8mb4_unicode_ci NOT NULL,
  `Date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Структура таблицы Sensor
--

CREATE TABLE IF NOT EXISTS Sensor (
  Sensor_ID smallint(5) unsigned NOT NULL,
  `Name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  Pin tinyint(1) NOT NULL,
  True_Value tinyint(1) NOT NULL,
  Send_Alarm tinyint(1) DEFAULT NULL,
  Priority tinyint(1) NOT NULL DEFAULT '10',
  False_Value tinyint(1) NOT NULL,
  Analog_Pin tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Датчики';

-- --------------------------------------------------------

--
-- Структура таблицы Sensor_Activity
--

CREATE TABLE IF NOT EXISTS Sensor_Activity (
  Sensor_Activity_ID int(10) unsigned NOT NULL,
  Sensor_ID smallint(5) unsigned NOT NULL,
  `Date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `Value` tinyint(4) NOT NULL,
  Mail_Sended_ID int(10) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Факты срабатывания датчиков';

-- --------------------------------------------------------

--
-- Структура таблицы Setup_Parameter
--

CREATE TABLE IF NOT EXISTS Setup_Parameter (
  Parameter_ID bigint(20) unsigned NOT NULL,
  `Name` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `VALUE` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT 'Текстовое значение',
  Note varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL COMMENT 'Примечание',
  Active_Flag tinyint(1) NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Набор параметров для системы';

-- --------------------------------------------------------

--
-- Структура таблицы Thermometer
--

CREATE TABLE IF NOT EXISTS Thermometer (
  Thermometer_ID smallint(5) unsigned NOT NULL,
  `Name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  Send_Alarm tinyint(1) NOT NULL,
  Threshold_Temp smallint(6) DEFAULT NULL,
  Threshold_Hum smallint(6) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Термометры в системе';

-- --------------------------------------------------------

--
-- Структура таблицы Thermometer_Info
--

CREATE TABLE IF NOT EXISTS Thermometer_Info (
  Thermometer_Info_ID int(10) unsigned NOT NULL,
  Thermometer_ID smallint(5) unsigned NOT NULL,
  Temp_Value smallint(6) NOT NULL,
  Humidity_Value smallint(6) NOT NULL,
  `Date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  Mail_Sended_ID int(10) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Показания термометров';

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_GET_MAX_MIN_TEMP_DAY
--
CREATE TABLE IF NOT EXISTS `V_GET_MAX_MIN_TEMP_DAY` (
`Thermometer_ID` smallint(5) unsigned
,`Date` date
,`max_temp` smallint(6)
,`min_temp` smallint(6)
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_GET_MAX_MIN_TEMP_LAST_24h
--
CREATE TABLE IF NOT EXISTS `V_GET_MAX_MIN_TEMP_LAST_24h` (
`Thermometer_ID` smallint(5) unsigned
,`max_temp` smallint(6)
,`min_temp` smallint(6)
,`avg_temp` decimal(6,0)
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_LAST_ACTIVITY
--
CREATE TABLE IF NOT EXISTS `V_LAST_ACTIVITY` (
`id` varchar(109)
,`date` varchar(59)
,`detail_info` text
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_LAST_SENSOR_ACTIVITY
--
CREATE TABLE IF NOT EXISTS `V_LAST_SENSOR_ACTIVITY` (
`Date` date
,`Sensor_ID` smallint(5) unsigned
,`Sensor_Name` varchar(100)
,`Count` bigint(21)
,`Last` time
,`First` time
,`Detail_info` text
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_LAST_SENSOR_ACTIVITY2
--
CREATE TABLE IF NOT EXISTS `V_LAST_SENSOR_ACTIVITY2` (
`Date` date
,`Sensor_ID` smallint(5) unsigned
,`Sensor_Name` varchar(100)
,`Count` bigint(21)
,`Last` time
,`First` time
,`Detail_info` text
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_LAST_TEMP_DATE
--
CREATE TABLE IF NOT EXISTS `V_LAST_TEMP_DATE` (
`Thermometer_ID` smallint(5) unsigned
,`Date` timestamp
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_LAST_TEMP_HUM
--
CREATE TABLE IF NOT EXISTS `V_LAST_TEMP_HUM` (
`Date` timestamp
,`Thermometer_ID` smallint(5) unsigned
,`name` varchar(100)
,`Temp_Value` smallint(6)
,`Humidity_Value` smallint(6)
,`Alarm` int(1)
,`min_temp` smallint(6)
,`max_temp` smallint(6)
,`avg_temp` decimal(6,0)
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления V_LAST_WATCHDOG_EVENT
--
CREATE TABLE IF NOT EXISTS `V_LAST_WATCHDOG_EVENT` (
`Date` timestamp
,`Dop_Info` varchar(255)
);

-- --------------------------------------------------------

--
-- Структура для представления V_GET_MAX_MIN_TEMP_DAY
--
DROP TABLE IF EXISTS `V_GET_MAX_MIN_TEMP_DAY`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_GET_MAX_MIN_TEMP_DAY AS select igorzhuk_myhome.Thermometer_Info.Thermometer_ID AS Thermometer_ID,cast(igorzhuk_myhome.Thermometer_Info.`Date` as date) AS `Date`,max(igorzhuk_myhome.Thermometer_Info.Temp_Value) AS max_temp,min(igorzhuk_myhome.Thermometer_Info.Temp_Value) AS min_temp from igorzhuk_myhome.Thermometer_Info group by igorzhuk_myhome.Thermometer_Info.Thermometer_ID,cast(igorzhuk_myhome.Thermometer_Info.`Date` as date);

-- --------------------------------------------------------

--
-- Структура для представления V_GET_MAX_MIN_TEMP_LAST_24h
--
DROP TABLE IF EXISTS `V_GET_MAX_MIN_TEMP_LAST_24h`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_GET_MAX_MIN_TEMP_LAST_24h AS select igorzhuk_myhome.Thermometer_Info.Thermometer_ID AS Thermometer_ID,max(igorzhuk_myhome.Thermometer_Info.Temp_Value) AS max_temp,min(igorzhuk_myhome.Thermometer_Info.Temp_Value) AS min_temp,round(avg(igorzhuk_myhome.Thermometer_Info.Temp_Value),0) AS avg_temp from igorzhuk_myhome.Thermometer_Info where (igorzhuk_myhome.Thermometer_Info.`Date` between (sysdate() - interval 24 hour) and sysdate()) group by igorzhuk_myhome.Thermometer_Info.Thermometer_ID;

-- --------------------------------------------------------

--
-- Структура для представления V_LAST_ACTIVITY
--
DROP TABLE IF EXISTS `V_LAST_ACTIVITY`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_LAST_ACTIVITY AS select concat('"',s.`Name`,'"','(',cast(s.Sensor_ID as char charset utf8mb4),')') AS `id`,concat('Дата=',cast(sa.`Date` as date),' Count=',cast(count(0) as char charset utf8mb4),' Last=',max(cast(sa.`Date` as time))) AS `date`,group_concat(concat(cast(sa.`Date` as time),'(',cast(sa.`Value` as char charset utf8mb4),')') order by cast(sa.`Date` as time) ASC separator '; ') AS detail_info from (igorzhuk_myhome.Sensor_Activity sa join igorzhuk_myhome.Sensor s on((s.Sensor_ID = sa.Sensor_ID))) where ((coalesce(s.Send_Alarm,0) = 1) and isnull(sa.Mail_Sended_ID)) group by concat('id=',cast(s.Sensor_ID as char charset utf8mb4),' name=',s.`Name`),concat(' date=',cast(sa.`Date` as date)) union (select (concat('"',t.`Name`,'"','(',cast(t.Thermometer_ID as char charset utf8mb4),')') collate utf8mb4_unicode_ci) AS `id`,(concat('Дата=',ti.`Date`) collate utf8mb4_unicode_ci) AS `Date`,(concat('Темп=',cast(ti.Temp_Value as char charset utf8mb4),' Влажн=',cast(ti.Humidity_Value as char charset utf8mb4)) collate utf8mb4_unicode_ci) AS detail_info from (igorzhuk_myhome.Thermometer_Info ti join igorzhuk_myhome.Thermometer t on((t.Thermometer_ID = ti.Thermometer_ID))) where ((coalesce(t.Send_Alarm,0) = 1) and (ti.Temp_Value < t.Threshold_Temp) and isnull(ti.Mail_Sended_ID)) order by (concat('"',t.`Name`,'"','(',cast(t.Thermometer_ID as char charset utf8mb4),')') collate utf8mb4_unicode_ci),(concat('Дата=',ti.`Date`) collate utf8mb4_unicode_ci) desc limit 10);

-- --------------------------------------------------------

--
-- Структура для представления V_LAST_SENSOR_ACTIVITY
--
DROP TABLE IF EXISTS `V_LAST_SENSOR_ACTIVITY`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_LAST_SENSOR_ACTIVITY AS select cast(sa.`Date` as date) AS `Date`,s.Sensor_ID AS Sensor_ID,s.`Name` AS Sensor_Name,count(0) AS Count,max(cast(sa.`Date` as time)) AS `Last`,min(cast(sa.`Date` as time)) AS `First`,group_concat(concat(cast(sa.`Date` as time),'(',cast(sa.`Value` as char charset utf8mb4),')') order by cast(sa.`Date` as time) ASC separator '; ') AS Detail_info from (igorzhuk_myhome.Sensor_Activity sa join igorzhuk_myhome.Sensor s on((s.Sensor_ID = sa.Sensor_ID))) group by cast(sa.`Date` as date),s.Sensor_ID,s.Priority order by cast(sa.`Date` as date) desc,s.Priority,s.Sensor_ID limit 20;

-- --------------------------------------------------------

--
-- Структура для представления V_LAST_SENSOR_ACTIVITY2
--
DROP TABLE IF EXISTS `V_LAST_SENSOR_ACTIVITY2`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_LAST_SENSOR_ACTIVITY2 AS select cast(sa.`Date` as date) AS `Date`,s.Sensor_ID AS Sensor_ID,s.`Name` AS Sensor_Name,count(0) AS Count,max(cast(sa.`Date` as time)) AS `Last`,min(cast(sa.`Date` as time)) AS `First`,group_concat(concat(cast(sa.`Date` as time),'(',(case when (sa2.`Date` is not null) then cast((unix_timestamp(sa2.`Date`) - unix_timestamp(sa.`Date`)) as char charset utf8mb4) else '-' end),')') order by cast(sa.`Date` as time) DESC separator '; ') AS Detail_info from ((igorzhuk_myhome.Sensor_Activity sa join igorzhuk_myhome.Sensor s on((s.Sensor_ID = sa.Sensor_ID))) left join igorzhuk_myhome.Sensor_Activity sa2 on(((sa2.Sensor_Activity_ID = (select igorzhuk_myhome.Sensor_Activity.Sensor_Activity_ID from igorzhuk_myhome.Sensor_Activity where ((igorzhuk_myhome.Sensor_Activity.Sensor_ID = sa.Sensor_ID) and (igorzhuk_myhome.Sensor_Activity.`Date` > sa.`Date`)) order by igorzhuk_myhome.Sensor_Activity.`Date` limit 1)) and (sa2.`Value` = s.True_Value)))) where (sa.`Value` = s.False_Value) group by cast(sa.`Date` as date),sa.Sensor_ID,s.Priority order by cast(sa.`Date` as date) desc,s.Priority,s.Sensor_ID limit 10;

-- --------------------------------------------------------

--
-- Структура для представления V_LAST_TEMP_DATE
--
DROP TABLE IF EXISTS `V_LAST_TEMP_DATE`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_LAST_TEMP_DATE AS select igorzhuk_myhome.Thermometer_Info.Thermometer_ID AS Thermometer_ID,max(igorzhuk_myhome.Thermometer_Info.`Date`) AS `Date` from igorzhuk_myhome.Thermometer_Info group by igorzhuk_myhome.Thermometer_Info.Thermometer_ID;

-- --------------------------------------------------------

--
-- Структура для представления V_LAST_TEMP_HUM
--
DROP TABLE IF EXISTS `V_LAST_TEMP_HUM`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_LAST_TEMP_HUM AS select t.`Date` AS `Date`,t.Thermometer_ID AS Thermometer_ID,t2.`Name` AS `name`,t1.Temp_Value AS Temp_Value,t1.Humidity_Value AS Humidity_Value,(case when ((t2.Send_Alarm = 1) and (t2.Threshold_Temp >= t1.Temp_Value)) then 1 else 0 end) AS Alarm,tmax.min_temp AS min_temp,tmax.max_temp AS max_temp,tmax.avg_temp AS avg_temp from (((igorzhuk_myhome.V_LAST_TEMP_DATE t join igorzhuk_myhome.Thermometer_Info t1 on(((t1.Thermometer_ID = t.Thermometer_ID) and (t1.`Date` = t.`Date`)))) join igorzhuk_myhome.Thermometer t2 on((t2.Thermometer_ID = t.Thermometer_ID))) join igorzhuk_myhome.V_GET_MAX_MIN_TEMP_LAST_24h tmax on((tmax.Thermometer_ID = t.Thermometer_ID)));

-- --------------------------------------------------------

--
-- Структура для представления V_LAST_WATCHDOG_EVENT
--
DROP TABLE IF EXISTS `V_LAST_WATCHDOG_EVENT`;

CREATE ALGORITHM=UNDEFINED DEFINER=igorzhuk_user@localhost SQL SECURITY DEFINER VIEW igorzhuk_myhome.V_LAST_WATCHDOG_EVENT AS select igorzhuk_myhome.`Event`.`Date` AS `Date`,igorzhuk_myhome.`Event`.Dop_Info AS Dop_Info from igorzhuk_myhome.`Event` where (igorzhuk_myhome.`Event`.Event_Type_ID = 3) order by igorzhuk_myhome.`Event`.`Date` desc limit 12;

--
-- Индексы сохранённых таблиц
--

--
-- Индексы таблицы Command
--
ALTER TABLE Command
  ADD PRIMARY KEY (Command_ID);

--
-- Индексы таблицы Errors
--
ALTER TABLE Errors
  ADD PRIMARY KEY (Error_ID),
  ADD UNIQUE KEY Error_ID (Error_ID);

--
-- Индексы таблицы Event
--
ALTER TABLE Event
  ADD PRIMARY KEY (Event_ID),
  ADD KEY Event_Type_ID (Event_Type_ID);

--
-- Индексы таблицы Event_Type
--
ALTER TABLE Event_Type
  ADD PRIMARY KEY (Event_Type_ID),
  ADD UNIQUE KEY Event_Type_ID (Event_Type_ID);

--
-- Индексы таблицы Mail_Sended
--
ALTER TABLE Mail_Sended
  ADD PRIMARY KEY (Mail_Sended_ID);

--
-- Индексы таблицы Sensor
--
ALTER TABLE Sensor
  ADD PRIMARY KEY (Sensor_ID),
  ADD UNIQUE KEY Detector_ID (Sensor_ID);

--
-- Индексы таблицы Sensor_Activity
--
ALTER TABLE Sensor_Activity
  ADD PRIMARY KEY (Sensor_Activity_ID),
  ADD KEY Sensor_ID (Sensor_ID),
  ADD KEY Mail_Sended_ID (Mail_Sended_ID);

--
-- Индексы таблицы Setup_Parameter
--
ALTER TABLE Setup_Parameter
  ADD PRIMARY KEY (Parameter_ID),
  ADD UNIQUE KEY Parameter_ID (Parameter_ID);

--
-- Индексы таблицы Thermometer
--
ALTER TABLE Thermometer
  ADD PRIMARY KEY (Thermometer_ID),
  ADD UNIQUE KEY Thermometer_ID (Thermometer_ID);

--
-- Индексы таблицы Thermometer_Info
--
ALTER TABLE Thermometer_Info
  ADD PRIMARY KEY (Thermometer_Info_ID),
  ADD UNIQUE KEY Thermometer_ID (Thermometer_ID,`Date`) USING BTREE,
  ADD KEY Mail_Sended_ID (Mail_Sended_ID);

--
-- AUTO_INCREMENT для сохранённых таблиц
--

--
-- AUTO_INCREMENT для таблицы Errors
--
ALTER TABLE Errors
  MODIFY Error_ID int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Event
--
ALTER TABLE Event
  MODIFY Event_ID int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Event_Type
--
ALTER TABLE Event_Type
  MODIFY Event_Type_ID int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Mail_Sended
--
ALTER TABLE Mail_Sended
  MODIFY Mail_Sended_ID int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Sensor
--
ALTER TABLE Sensor
  MODIFY Sensor_ID smallint(5) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Sensor_Activity
--
ALTER TABLE Sensor_Activity
  MODIFY Sensor_Activity_ID int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Setup_Parameter
--
ALTER TABLE Setup_Parameter
  MODIFY Parameter_ID bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Thermometer
--
ALTER TABLE Thermometer
  MODIFY Thermometer_ID smallint(5) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы Thermometer_Info
--
ALTER TABLE Thermometer_Info
  MODIFY Thermometer_Info_ID int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- Ограничения внешнего ключа сохраненных таблиц
--

--
-- Ограничения внешнего ключа таблицы Event
--
ALTER TABLE Event
  ADD CONSTRAINT Event_ibfk_1 FOREIGN KEY (Event_Type_ID) REFERENCES Event_Type (Event_Type_ID);

--
-- Ограничения внешнего ключа таблицы Sensor_Activity
--
ALTER TABLE Sensor_Activity
  ADD CONSTRAINT Sensor_Activity_ibfk_1 FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID);

--
-- Ограничения внешнего ключа таблицы Thermometer_Info
--
ALTER TABLE Thermometer_Info
  ADD CONSTRAINT Thermometer_Info_ibfk_1 FOREIGN KEY (Thermometer_ID) REFERENCES Thermometer (Thermometer_ID);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;