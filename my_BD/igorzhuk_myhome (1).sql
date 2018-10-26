-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Хост: localhost
-- Время создания: Окт 27 2018 г., 00:04
-- Версия сервера: 10.0.35-MariaDB
-- Версия PHP: 5.4.16

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- База данных: `igorzhuk_myhome`
--

-- --------------------------------------------------------

--
-- Дублирующая структура для представления `V_GET_MAX_MIN_TEMP_DAY`
--
CREATE TABLE IF NOT EXISTS `V_GET_MAX_MIN_TEMP_DAY` (
`Thermometer_ID` smallint(5) unsigned
,`Date` date
,`max_temp` smallint(6)
,`min_temp` smallint(6)
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления `V_GET_MAX_MIN_TEMP_LAST_24h`
--
CREATE TABLE IF NOT EXISTS `V_GET_MAX_MIN_TEMP_LAST_24h` (
`Thermometer_ID` smallint(5) unsigned
,`max_temp` smallint(6)
,`min_temp` smallint(6)
,`avg_temp` decimal(6,0)
);

-- --------------------------------------------------------

--
-- Дублирующая структура для представления `V_LAST_TEMP_HUM`
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
-- Структура для представления `V_GET_MAX_MIN_TEMP_DAY`
--
DROP TABLE IF EXISTS `V_GET_MAX_MIN_TEMP_DAY`;

CREATE ALGORITHM=UNDEFINED DEFINER=`igorzhuk_user`@`localhost` SQL SECURITY DEFINER VIEW `V_GET_MAX_MIN_TEMP_DAY` AS select `Thermometer_Info`.`Thermometer_ID` AS `Thermometer_ID`,cast(`Thermometer_Info`.`Date` as date) AS `Date`,max(`Thermometer_Info`.`Temp_Value`) AS `max_temp`,min(`Thermometer_Info`.`Temp_Value`) AS `min_temp` from `Thermometer_Info` group by `Thermometer_Info`.`Thermometer_ID`,cast(`Thermometer_Info`.`Date` as date);

-- --------------------------------------------------------

--
-- Структура для представления `V_GET_MAX_MIN_TEMP_LAST_24h`
--
DROP TABLE IF EXISTS `V_GET_MAX_MIN_TEMP_LAST_24h`;

CREATE ALGORITHM=UNDEFINED DEFINER=`igorzhuk_user`@`localhost` SQL SECURITY DEFINER VIEW `V_GET_MAX_MIN_TEMP_LAST_24h` AS select `Thermometer_Info`.`Thermometer_ID` AS `Thermometer_ID`,max(`Thermometer_Info`.`Temp_Value`) AS `max_temp`,min(`Thermometer_Info`.`Temp_Value`) AS `min_temp`,round(avg(`Thermometer_Info`.`Temp_Value`),0) AS `avg_temp` from `Thermometer_Info` where (`Thermometer_Info`.`Date` between (sysdate() - interval 24 hour) and sysdate()) group by `Thermometer_Info`.`Thermometer_ID`;

-- --------------------------------------------------------

--
-- Структура для представления `V_LAST_TEMP_HUM`
--
DROP TABLE IF EXISTS `V_LAST_TEMP_HUM`;

CREATE ALGORITHM=UNDEFINED DEFINER=`igorzhuk_user`@`localhost` SQL SECURITY DEFINER VIEW `V_LAST_TEMP_HUM` AS select `t`.`Date` AS `Date`,`t`.`Thermometer_ID` AS `Thermometer_ID`,`t2`.`Name` AS `name`,`t1`.`Temp_Value` AS `Temp_Value`,`t1`.`Humidity_Value` AS `Humidity_Value`,(case when ((`t2`.`Send_Alarm` = 1) and (`t2`.`Threshold_Temp` >= `t1`.`Temp_Value`)) then 1 else 0 end) AS `Alarm`,`tmax`.`min_temp` AS `min_temp`,`tmax`.`max_temp` AS `max_temp`,`tmax`.`avg_temp` AS `avg_temp` from (((`V_LAST_TEMP_DATE` `t` join `Thermometer_Info` `t1` on(((`t1`.`Thermometer_ID` = `t`.`Thermometer_ID`) and (`t1`.`Date` = `t`.`Date`)))) join `Thermometer` `t2` on((`t2`.`Thermometer_ID` = `t`.`Thermometer_ID`))) join `V_GET_MAX_MIN_TEMP_LAST_24h` `tmax` on((`tmax`.`Thermometer_ID` = `t`.`Thermometer_ID`)));

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
