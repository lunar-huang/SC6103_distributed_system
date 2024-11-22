


CREATE TABLE flights (
    flight_id INT AUTO_INCREMENT PRIMARY KEY,
    source_place VARCHAR(100) NOT NULL,
    destination_place VARCHAR(100) NOT NULL,
    departure_year INT NOT NULL,
    departure_month INT NOT NULL,
    departure_day INT NOT NULL,
    departure_hour INT NOT NULL,
    departure_minute INT NOT NULL,
    airfare FLOAT NOT NULL,
    seat_availability INT NOT NULL,
    baggage_availability INT NOT NULL
);
