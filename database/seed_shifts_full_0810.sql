-- ============================================================
-- DATASET DAY DU -- DANG KY LICH LAM TUAN 10/08/2026 - 16/08/2026
-- Tong so nhan vien: 60
-- Ca Sang : 07:00 - 12:00
-- Ca Chieu: 12:00 - 17:00
-- Ca Toi  : 17:00 - 22:00
-- status = 0 : Dang cho thuat toan xep lich
-- ============================================================

DELETE FROM SHIFT WHERE workDate BETWEEN '2026-08-10' AND '2026-08-16' AND status = 0;

-- ============================================================
-- PHASE 1 & 2 -- NHAN VIEN FULL-TIME (39 nguoi)
-- ============================================================

-- 1004 kim ri cha (Cashier - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1004, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1005 Hung Mel Bel (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1005, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1005, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1005, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1005, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1005, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang

-- 1006 Hồ Tuấn Phúc (Cashier - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1006, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1006, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1006, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1006, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1006, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1009 Dương Thị Nga (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1009, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1009, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1009, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1009, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1009, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1011 Lý Xuân Dũng (Cashier - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1011, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1011, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1011, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1011, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1011, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1011, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1014 Trần Thanh Lan (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1014, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1015 Huỳnh Thu Phúc (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1015, '2026-08-10', '07:00:00', '12:00:00', 0); -- T2 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1015, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1015, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1015, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1015, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1016 Đặng Hữu Phong (Cashier - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1016, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1016, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1016, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1016, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1016, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1016, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1017 Huỳnh Đức Uyên (KitchenAssistant - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1017, '2026-08-10', '07:00:00', '12:00:00', 0); -- T2 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1017, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1017, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1017, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1017, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1020 Hồ Thanh Trung (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1020, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1020, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1020, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1020, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1020, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1021 Vũ Gia Trang (Cashier - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1021, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1021, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1021, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1021, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1021, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang

-- 1022 Ngô Thị Tú (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1022, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1023 Lý Thị Nhi (HallStaff - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1023, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1023, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1023, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1023, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1023, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1023, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1024 Lê Hải Uyên (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1024, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1024, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1024, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1024, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1024, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1025 Huỳnh Hoài Anh (Cashier - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1025, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1026 Ngô Thu Thắng (KitchenAssistant - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1026, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1026, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1026, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1026, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1026, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1026, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1027 Phạm Đức Trung (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1027, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1028 Đặng Hữu Lan (Cashier - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1028, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1029 Ngô Hải Thảo (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1029, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1030 Đỗ Hải Trang (Cashier - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1030, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1033 Huỳnh Bích Trung (Cashier - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1033, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1033, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1033, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1033, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1033, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1033, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu

-- 1034 Đỗ Quang Hương (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1034, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1036 Nguyễn Bích Quỳnh (KitchenAssistant - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1036, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1036, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1036, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1036, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1036, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1038 Đỗ Hữu Nhi (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1038, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1039 Phan Văn Trang (KitchenAssistant - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1039, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1040 Võ Xuân Vân (HallStaff - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1040, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1040, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1040, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1040, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1040, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1040, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1041 Phan Hữu Linh (KitchenAssistant - Full-time) -- 6 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1041, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1041, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1041, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1041, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1041, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1041, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1042 Đỗ Bích Thành (KitchenAssistant - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1042, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1043 Lê Thị Hà (KitchenAssistant - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1043, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1043, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1043, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1043, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1043, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi

-- 1045 Huỳnh Bích Linh (KitchenAssistant - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1045, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1045, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1045, '2026-08-14', '17:00:00', '22:00:00', 0); -- T6 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1045, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1045, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1047 Phạm Quang Thắng (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1047, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1048 Lê Thanh Uyên (KitchenAssistant - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1048, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1049 Phan Thanh Quỳnh (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1049, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1049, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1049, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1049, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1049, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi

-- 1050 Phạm Tuấn Phúc (HallStaff - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1050, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1050, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1050, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1050, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1050, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1052 Nguyễn Tuấn Khang (HallStaff - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-10', '07:00:00', '12:00:00', 0); -- T2 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1052, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1054 Trần Xuân Thịnh (KitchenAssistant - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1054, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1054, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1054, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1054, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1054, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu

-- 1055 Trần Gia Hà (KitchenAssistant - Full-time) -- 5 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1055, '2026-08-12', '12:00:00', '17:00:00', 0); -- T4 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1055, '2026-08-13', '17:00:00', '22:00:00', 0); -- T5 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1055, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1055, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1055, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1059 V T T (Cashier - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-10', '07:00:00', '12:00:00', 0); -- T2 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1059, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1060 V N T (Cashier - Full-time) -- 7 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1060, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- ============================================================
-- PHASE 3 -- NHAN VIEN PART-TIME (21 nguoi)
-- ============================================================

-- 1001 Ho Van Y (HallStaff - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1001, '2026-08-11', '07:00:00', '17:00:00', 0); -- T3 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1001, '2026-08-13', '07:00:00', '17:00:00', 0); -- T5 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1001, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1002 Đào Thị Tý (Cashier - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1002, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1002, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1002, '2026-08-15', '07:00:00', '17:00:00', 0); -- T7 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1002, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1003 Minh Nhật Bel (KitchenAssistant - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1003, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1003, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1003, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang

-- 1007 Bùi Đức Nam (Cashier - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1007, '2026-08-12', '12:00:00', '22:00:00', 0); -- T4 Chieu+Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1007, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1007, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1007, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang

-- 1008 Bùi Hữu Khang (HallStaff - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1008, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1008, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1008, '2026-08-15', '12:00:00', '17:00:00', 0); -- T7 Ca Chieu

-- 1010 Phan Hoài Quang (HallStaff - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1010, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1010, '2026-08-14', '07:00:00', '17:00:00', 0); -- T6 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1010, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1012 Phạm Gia Hùng (KitchenAssistant - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1012, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1012, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1012, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1012, '2026-08-15', '12:00:00', '22:00:00', 0); -- T7 Chieu+Toi

-- 1013 Hồ Xuân Nga (HallStaff - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1013, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1013, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1013, '2026-08-16', '12:00:00', '17:00:00', 0); -- CN Ca Chieu

-- 1018 Hồ Gia Nam (KitchenAssistant - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1018, '2026-08-10', '07:00:00', '17:00:00', 0); -- T2 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1018, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1018, '2026-08-16', '07:00:00', '12:00:00', 0); -- CN Ca Sang

-- 1019 Trần Ngọc Vân (HallStaff - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1019, '2026-08-12', '12:00:00', '22:00:00', 0); -- T4 Chieu+Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1019, '2026-08-14', '12:00:00', '22:00:00', 0); -- T6 Chieu+Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1019, '2026-08-15', '07:00:00', '17:00:00', 0); -- T7 Sang+Chieu

-- 1031 Phan Thu Khang (KitchenAssistant - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1031, '2026-08-10', '07:00:00', '12:00:00', 0); -- T2 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1031, '2026-08-12', '07:00:00', '12:00:00', 0); -- T4 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1031, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1031, '2026-08-16', '07:00:00', '17:00:00', 0); -- CN Sang+Chieu

-- 1032 Đỗ Quang Hương (Cashier - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1032, '2026-08-10', '07:00:00', '12:00:00', 0); -- T2 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1032, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1032, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang

-- 1035 Hồ Bích Minh (KitchenAssistant - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1035, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1035, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1035, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1037 Trần Hữu Thảo (KitchenAssistant - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1037, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1037, '2026-08-13', '07:00:00', '17:00:00', 0); -- T5 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1037, '2026-08-14', '07:00:00', '17:00:00', 0); -- T6 Sang+Chieu

-- 1044 Đỗ Quang Vân (KitchenAssistant - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1044, '2026-08-11', '07:00:00', '17:00:00', 0); -- T3 Sang+Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1044, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1044, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1044, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1046 Bùi Quang Thành (KitchenAssistant - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1046, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1046, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1046, '2026-08-14', '12:00:00', '22:00:00', 0); -- T6 Chieu+Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1046, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1051 Trần Văn Hùng (KitchenAssistant - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1051, '2026-08-10', '17:00:00', '22:00:00', 0); -- T2 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1051, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1051, '2026-08-13', '07:00:00', '12:00:00', 0); -- T5 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1051, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu

-- 1053 Hồ Hải Uyên (KitchenAssistant - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1053, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1053, '2026-08-11', '12:00:00', '17:00:00', 0); -- T3 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1053, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1053, '2026-08-15', '07:00:00', '12:00:00', 0); -- T7 Ca Sang

-- 1056 Nguyễn Hồng Hiệp (Cashier - Part-time) -- 4 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1056, '2026-08-11', '07:00:00', '12:00:00', 0); -- T3 Ca Sang
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1056, '2026-08-12', '17:00:00', '22:00:00', 0); -- T4 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1056, '2026-08-14', '12:00:00', '17:00:00', 0); -- T6 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1056, '2026-08-16', '17:00:00', '22:00:00', 0); -- CN Ca Toi

-- 1057 Bảo Trình (Cashier - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1057, '2026-08-10', '12:00:00', '17:00:00', 0); -- T2 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1057, '2026-08-14', '12:00:00', '22:00:00', 0); -- T6 Chieu+Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1057, '2026-08-15', '17:00:00', '22:00:00', 0); -- T7 Ca Toi

-- 1058 Đỗ thanks son (Cashier - Part-time) -- 3 ngay
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1058, '2026-08-11', '17:00:00', '22:00:00', 0); -- T3 Ca Toi
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1058, '2026-08-13', '12:00:00', '17:00:00', 0); -- T5 Ca Chieu
INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES (1058, '2026-08-14', '07:00:00', '12:00:00', 0); -- T6 Ca Sang

-- TONG CONG: 308 ban ghi dang ky ca