import sqlite3
import random
import hashlib
from datetime import datetime, timedelta

def main():
    db_path = "database/Systems.db"
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Step 1: Update Võ Nam Thục Đoan
    password_hash = hashlib.sha256("123456".encode('utf-8')).hexdigest()
    cursor.execute("SELECT idEmployee FROM PROFILES WHERE name LIKE '%Võ Nam Thục Đoan%'")
    rows = cursor.fetchall()
    for row in rows:
        emp_id = row[0]
        cursor.execute("UPDATE ACCOUNTS SET userName='admin', password=? WHERE idEmployee=?", (password_hash, emp_id))
    
    print(f"Updated {len(rows)} user(s) to 'admin'")

    # Step 2: Delete Đinh Ngọc Thanh
    cursor.execute("SELECT idEmployee FROM PROFILES WHERE name LIKE '%Đinh Ngọc Thanh%'")
    rows_to_delete = cursor.fetchall()
    for row in rows_to_delete:
        emp_id = row[0]
        cursor.execute("DELETE FROM PROFILES WHERE idEmployee=?", (emp_id,))
        cursor.execute("DELETE FROM ACCOUNTS WHERE idEmployee=?", (emp_id,))
        # You may also want to delete from SHIFT, etc., but since we just generated them, it's fine.
    
    print(f"Deleted {len(rows_to_delete)} user(s) named Đinh Ngọc Thanh")

    # Step 3: Insert 50 employees
    first_names = ["Nguyễn", "Trần", "Lê", "Phạm", "Hoàng", "Huỳnh", "Phan", "Vũ", "Võ", "Đặng", "Bùi", "Đỗ", "Hồ", "Ngô", "Dương", "Lý"]
    middle_names = ["Thị", "Văn", "Hữu", "Thanh", "Minh", "Thu", "Ngọc", "Hải", "Xuân", "Hoài", "Quốc", "Tuấn", "Đức", "Gia", "Bảo"]
    last_names = ["Anh", "Bình", "Châu", "Dũng", "Duy", "Giang", "Hà", "Hải", "Hiếu", "Hòa", "Hùng", "Hương", "Khánh", "Kiên", "Lan", "Linh", "Long", "Minh", "Nam", "Nga", "Ngọc", "Nhung", "Phong", "Phú", "Phương", "Quân", "Quang", "Quỳnh", "Sơn", "Tài", "Thảo", "Thắng", "Thành", "Thủy", "Tiến", "Trang", "Trí", "Trung", "Tuấn", "Tú", "Vân", "Việt"]
    roles = ["Manager", "Cashier", "HallStaff", "KitchenAssistant"]

    added = 0
    for i in range(50):
        name = f"{random.choice(first_names)} {random.choice(middle_names)} {random.choice(last_names)}"
        phone = "0" + "".join([str(random.randint(0, 9)) for _ in range(9)])
        role = random.choice(roles)
        
        start_date = datetime.strptime("1980-01-01", "%Y-%m-%d")
        end_date = datetime.strptime("2003-12-31", "%Y-%m-%d")
        random_days = random.randint(0, (end_date - start_date).days)
        dob = (start_date + timedelta(days=random_days)).strftime("%Y-%m-%d")
        
        gender = random.choice(["Male", "Female", "Other"])
        address = "123 Street"
        
        salary = 3000000 if role == "Manager" else (1500000 if role == "HallStaff" else 2000000)
        is_fixed = 1 if role == "Manager" else 0
        
        # Insert into PROFILES
        cursor.execute('''
            INSERT INTO PROFILES (name, phoneNum, role, DOB, gender, Address, AvatarPath, Salary, isFixed, status)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (name, phone, role, dob, gender, address, "", salary, is_fixed, "active"))
        
        emp_id = cursor.lastrowid
        
        # Insert into ACCOUNTS
        username = f"user{emp_id}"
        pass_hash = hashlib.sha256("password".encode('utf-8')).hexdigest()
        cursor.execute('''
            INSERT INTO ACCOUNTS (idEmployee, userName, password)
            VALUES (?, ?, ?)
        ''', (emp_id, username, pass_hash))
        
        added += 1

    print(f"Added {added} random employees.")

    conn.commit()
    conn.close()

if __name__ == "__main__":
    main()
