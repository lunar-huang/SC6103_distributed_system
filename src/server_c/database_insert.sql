DELIMITER $$

CREATE PROCEDURE insert_flight_data()
BEGIN
    DECLARE i INT DEFAULT 1;
    DECLARE src VARCHAR(100);
    DECLARE dest VARCHAR(100);

    -- 创建城市数组，您可以根据需要添加更多城市
    DECLARE cities ARRAY;
    SET cities = JSON_ARRAY('Singapore', 'Shanghai', 'Tokyo', 'Beijing', 'New York', 'Los Angeles', 'London', 'Paris', 'Sydney', 'Dubai');

    WHILE i <= 300 DO
        -- 随机选择 source_place 和 destination_place，确保不同
        SET src = JSON_UNQUOTE(JSON_EXTRACT(cities, FLOOR(RAND() * 10)));
        SET dest = JSON_UNQUOTE(JSON_EXTRACT(cities, FLOOR(RAND() * 10)));
        
        -- 确保 source_place 和 destination_place 不同
        WHILE src = dest DO
            SET dest = JSON_UNQUOTE(JSON_EXTRACT(cities, FLOOR(RAND() * 10)));
        END WHILE;

        -- 插入航班数据
        INSERT INTO flights (source_place, destination_place, departure_year, departure_month, departure_day, departure_hour, departure_minute, airfare, seat_availability, baggage_availability)
        VALUES (src, dest, 2024, FLOOR(RAND() * 12 + 1), FLOOR(RAND() * 30 + 1), FLOOR(RAND() * 24), FLOOR(RAND() * 60), FLOOR(RAND() * 1000 + 200), FLOOR(RAND() * 100 + 50), FLOOR(RAND() * 50 + 20));

        -- 递增计数器
        SET i = i + 1;
    END WHILE;
END$$

DELIMITER ;

-- 运行存储过程
CALL insert_flight_data();
