SELECT id, 1 - percent_rank() OVER (ORDER BY money DESC) AS r INTO temp_money_rank FROM users;
UPDATE all_users u SET rank = r FROM temp_money_rank t WHERE u.id = t.id;
DROP TABLE temp_money_rank;
