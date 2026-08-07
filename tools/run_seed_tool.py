import sqlite3
import datetime

db_path = 'database/Systems.db'
c = sqlite3.connect(db_path)
cur = c.cursor()

cur.execute("SELECT idEmployee FROM PROFILES WHERE status IS NULL OR lower(status) = 'active' ORDER BY idEmployee")
employees = [row[0] for row in cur.fetchall()]

if not employees:
    print("No active employees found.")
    exit(1)

blocks = [
    ("07:00:00.000", "12:00:00.000"),
    ("12:00:00.000", "17:00:00.000"),
    ("17:00:00.000", "22:00:00.000")
]

start_date = datetime.date(2026, 8, 10)

inserted = 0
for day in range(7):
    current_date = start_date + datetime.timedelta(days=day)
    date_str = current_date.isoformat()
    for emp_idx, emp_id in enumerate(employees):
        block = blocks[(day + emp_idx) % 3]
        
        # Check overlap
        cur.execute("""
            SELECT 1 FROM SHIFT WHERE idEmployee = ? 
            AND workDate = ? AND status IN (0, 1) 
            AND startTime < ? AND endTime > ? LIMIT 1
        """, (emp_id, date_str, block[1], block[0]))
        if cur.fetchone():
            continue
            
        cur.execute("""
            INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) 
            VALUES (?, ?, ?, ?, 0, 0)
        """, (emp_id, date_str, block[0], block[1]))
        inserted += 1

c.commit()
c.close()
print(f"Seeded week {start_date} to {start_date + datetime.timedelta(days=6)}")
print(f"Active employees: {len(employees)}")
print(f"Pending rows inserted: {inserted}")
