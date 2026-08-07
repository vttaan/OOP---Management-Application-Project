# Tài Liệu Kỹ Thuật Hệ Thống — Optimus Management Application

> Tài liệu này mô tả **toàn bộ tính năng** của từng trang trong hệ thống, cấu trúc dữ liệu, và chi tiết luồng xử lý của từng tính năng. Phù hợp cho developer mới muốn hiểu luồng dữ liệu từ UI → Controller → Model → Database.

---

## Mục Lục

1. [Tổng Quan Kiến Trúc](#1-tổng-quan-kiến-trúc)
2. [Chi Tiết Tính Năng Theo Trang](#2-chi-tiết-tính-năng-theo-trang)
   - [Trang Đăng Nhập (Login)](#21-trang-đăng-nhập-login)
   - [Trang Dashboard](#22-trang-dashboard)
   - [Trang Hồ Sơ Cá Nhân (Profile)](#23-trang-hồ-sơ-cá-nhân-profile)
   - [Trang Nhân Sự (Employee)](#24-trang-nhân-sự-employee)
   - [Trang Đăng Ký / Xếp Lịch (Schedule)](#25-trang-đăng-ký--xếp-lịch-schedule)
   - [Trang Xem Lịch (View Schedule)](#26-trang-xem-lịch-view-schedule)
   - [Trang Lương (Salary)](#27-trang-lương-salary)
   - [Trang Thông Báo (Notification)](#28-trang-thông-báo-notification)
   - [Trang Cài Đặt (Setting)](#29-trang-cài-đặt-setting)
3. [Bảng Tổng Hợp Hàm Toàn Hệ Thống](#3-bảng-tổng-hợp-hàm-toàn-hệ-thống)

---

## 1. Tổng Quan Kiến Trúc

```text
┌─────────────────────────────────────────────────────────────────┐
│                          USER (Người dùng)                       │
└──────────────────────────────┬──────────────────────────────────┘
                               │ Tương tác (click, nhập liệu)
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                  VIEW (src/view/)                                │
│  Hiển thị UI, thu thập input, phát Signal đến Controller        │
└──────────────────────────────┬──────────────────────────────────┘
                               │ Qt Signal → Slot
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│               CONTROLLER (src/control/)                          │
│  Xử lý logic nghiệp vụ, điều phối giữa View và Model           │
└──────────┬────────────────────────────────────────┬─────────────┘
           │ Gọi hàm Model                          │ Cập nhật View
           ▼                                        ▼
┌──────────────────────┐               ┌────────────────────────┐
│    MODEL (src/model/)│               │  SessionManager        │
│  Truy vấn DB, tính   │               │  (Singleton - global)  │
│  toán nghiệp vụ      │               └────────────────────────┘
└──────────┬───────────┘
           │ Database::execQuery()
           ▼
┌──────────────────────┐
│  SQLite Database     │
│  (Systems.db)        │
└──────────────────────┘
```

---

## 2. Chi Tiết Tính Năng Theo Trang

### 2.1. Trang Đăng Nhập (Login)

#### Tính Năng: Đăng nhập hệ thống

- **Dữ liệu / Attribute sử dụng:**
  - Dữ liệu thu thập từ View để khởi tạo object/phiên đăng nhập:
    - `ui->inpUsername` (QLineEdit): Lấy tên đăng nhập (`userName`)
    - `ui->inpPassword` (QLineEdit): Lấy mật khẩu (`passWord`)
  - Database table `ACCOUNTS`: `userName`, `passWord`, `idEmployee`
  - Database table `PROFILES`: `role`, `idEmployee`, `name`, `gender`, `isFixed`, v.v.

- **Được gọi tới từ hàm nào (Called By):**
  - Trigger từ UI `Login_View::on_loginButton_clicked()` (phát signal `loginSubmitted`).
  - Hàm nhận: `Login_Control::handleLoginSubmission(username, password)`.

- **Sẽ gọi hàm nào (Calls):**
  - `Login_Model::verifyLogin(username, password)`
  - `Security::hashPassword(password)`
  - `UserFactory::createContainsUser(role, ...)`
  - `SessionManager::saveCurrentInfo(user*)`

- **Công dụng của các hàm:**
  - `handleLoginSubmission`: Hàm điều phối chính, gọi Model để xác thực và nếu thành công thì lưu vào Session rồi chuyển trang.
  - `verifyLogin`: Băm mật khẩu, truy vấn DB (kiểm tra `ACCOUNTS` khớp userName, passWord). Nếu khớp, lấy thông tin từ `PROFILES` để tạo object `User`.
  - `hashPassword`: Tăng cường bảo mật bằng SHA-256.
  - `createContainsUser`: Polymorphism, sinh ra object `Staff` hoặc `Manager` tùy theo `role`.

---

### 2.2. Trang Dashboard

Dashboard tự động cập nhật (refresh) theo chu kỳ.

#### Tính Năng: Hiển thị nhân viên trong ca hiện tại
- **Dữ liệu / Attribute sử dụng:**
  - `SHIFT`: `idEmployee`, `date`, `startTime`, `endTime`, `status`
  - So sánh `startTime` và `endTime` với `QTime::currentTime()` và `QDate::currentDate()`.
- **Được gọi tới từ:** `Dashboard_Control::loadEmployeeCards(list)` (gọi trong `init()` và `autoRefresh()`).
- **Sẽ gọi hàm:** `Dashboard_Model::getWorkingEmployeeIds()`.
- **Công dụng:** Trả về danh sách ID nhân viên có ca làm đang diễn ra để Controller tạo các thẻ `EmployeeCard` đẩy lên View.

#### Tính Năng: Hiển thị ca làm tiếp theo
- **Dữ liệu / Attribute sử dụng:**
  - `SHIFT`: `startTime`, `endTime`, `date`, `idEmployee` (cho các ca có `startTime > currentTime`)
  - `PROFILES`: `name`, `phoneNum`, `role`, `avatarPath`
- **Được gọi tới từ:** `Dashboard_Control::loadShiftPanel()`.
- **Sẽ gọi hàm:** `Dashboard_Model::getNextShiftEmployees()`.
- **Công dụng:** Lấy danh sách nhân viên sẽ làm ca kế tiếp trong ngày hôm nay, ánh xạ vào DTO `ShiftEmployeeInfo` để hiển thị trên UI.

#### Tính Năng: Biểu đồ thống kê lương (Manager only)
- **Dữ liệu / Attribute sử dụng:**
  - Bảng lương giả lập từ việc tổng hợp `SHIFT` (số giờ/ngày làm) và `PROFILES` (lương cơ bản `baseSalary`).
- **Được gọi tới từ:** `Dashboard_Control::loadSalaryChart()`.
- **Sẽ gọi hàm:** `Dashboard_Model::getSalaryStats(year)`.
- **Công dụng:** Trả về cấu trúc `SalaryChartData` chứa mảng 12 tháng của năm `year` và `year-1`, đồng thời View xử lý hover tooltip hiển thị số VNĐ.

#### Tính Năng: Yêu cầu nghỉ phép chờ duyệt (Manager only)
- **Dữ liệu / Attribute sử dụng:**
  - `LEAVE_REQUESTS`: `requestId`, `employeeId`, `date`, `reason`, `status` (pending).
- **Được gọi tới từ:** `Dashboard_Control::loadLeaveRequestPanel()` và `Dashboard_Control::reviewLeaveRequest()`.
- **Sẽ gọi hàm:** `LeaveRequest_Model::getPendingLeaveRequests()`, `LeaveRequest_Model::decideLeaveRequest()`.
- **Công dụng:**
  - Quản lý lấy danh sách chờ duyệt.
  - Xác nhận Duyệt/Từ chối: Cập nhật status vào `LEAVE_REQUESTS`, tạo Notification báo cho nhân viên.

#### Tính Năng: Xin nghỉ phép (Staff only)
- **Dữ liệu / Attribute sử dụng (khi tạo object LeaveRequest từ View):**
  - `shiftList` (QListWidget): Danh sách các ca làm, chọn để lấy `shiftId`.
  - `reasonEdit` (QPlainTextEdit): TextField để nhập lý do xin nghỉ (`reason`).
- **Được gọi tới từ:** Nút "Xin nghỉ phép" trên `Schedule_View` gọi tới `Schedule_Control::onLeaveRequested()`.
- **Sẽ gọi hàm:** `LeaveRequest_Model::submitLeaveRequest()`.
- **Công dụng:** Khởi tạo object `LeaveRequest` với trạng thái pending (chờ duyệt) và lưu vào bảng `LEAVE_REQUESTS`.

---

### 2.3. Trang Hồ Sơ Cá Nhân (Profile)

#### Tính Năng: Sửa thông tin cá nhân
- **Dữ liệu / Attribute sử dụng (khi tạo/cập nhật object Profile từ View):**
  - `ui->inpName` (QLineEdit): Lấy tên (`name`)
  - `ui->inpDob` (QDateEdit): Lấy ngày sinh (`dob`)
  - `ui->inpAddress` (QLineEdit): Lấy địa chỉ (`address`)
  - `ui->inpPhone` (QLineEdit): Lấy SĐT (`phoneNum`)
  - `ui->inpCitizenId` (QLineEdit): Lấy CCCD (`citizenId`)
  - `ui->cmbGender` (QComboBox): Lấy giới tính (`gender`)
  - Biến nội bộ `m_avatarPath`: Lấy đường dẫn ảnh đại diện (`avatarPath`)
- **Được gọi tới từ:** Sự kiện Save trên `EditProfile_Widget` phát signal tới `Profile_Control::handleProfileUpdate()`.
- **Sẽ gọi hàm:** `Profile_Model::updateProfile(id, name, dob, address, ...)`.
- **Công dụng:** Validate dữ liệu, lưu file ảnh avatar vào thư mục resources nội bộ, sau đó thực thi `UPDATE PROFILES` trong Database và cập nhật đối tượng `User` hiện tại trong `SessionManager`.

#### Tính Năng: Đổi mật khẩu
- **Dữ liệu / Attribute sử dụng (từ View):**
  - `ui->inpOldPassword` (QLineEdit): Lấy mật khẩu cũ
  - `ui->inpNewPassword` (QLineEdit): Lấy mật khẩu mới
  - `ui->inpConfirmPassword` (QLineEdit): Xác nhận mật khẩu mới
  - Database `ACCOUNTS`: `passWord`.
- **Được gọi tới từ:** Sự kiện Confirm trên `EditPassword_Widget` phát signal tới `Profile_Control::handlePasswordUpdate()`.
- **Sẽ gọi hàm:** `Profile_Model::updatePassword()` -> `Change_password::updatePassword()`.
- **Công dụng:**
  - Hash `oldPassword` để kiểm tra có khớp DB không.
  - Validate `newPassword` (tối thiểu 6 ký tự).
  - Hash `newPassword` và lưu lại vào DB thông qua `UPDATE ACCOUNTS`. Trả về Enum `PasswordChangeResult` (Thành công, Sai mật khẩu cũ, Mật khẩu quá yếu).

---

### 2.4. Trang Nhân Sự (Employee)

Chỉ Manager mới có quyền truy cập trang này.

#### Tính Năng: Xem và tìm kiếm nhân viên
- **Dữ liệu / Attribute sử dụng:**
  - Thuộc tính từ `PROFILES` (Tên, SDT, Chức vụ, ...) và `SHIFT` (tổng giờ làm trong tháng).
  - `contentSearch`, `contentFilter`, `contentSort` (từ ô tìm kiếm và Dropdown Filter UI).
- **Được gọi tới từ:** Ô tìm kiếm hoặc Dropdown Filter/Sort thay đổi, trigger `Employee_Control::handleUpdate()`.
- **Sẽ gọi hàm:** `Employee_Model::SearchSortFilter()`.
- **Công dụng:**
  - Sử dụng thuật toán **Rabin-Karp** để tìm kiếm trên chuỗi (tên, ID) sau khi đã bỏ dấu tiếng Việt (`removeAccent()`).
  - Lọc theo giới tính, chức vụ (`filterInEmployee`).
  - Sắp xếp tăng/giảm theo ID hoặc Tên (`sortInEmployee`).

#### Tính Năng: Thêm/Sửa/Xóa Nhân Viên (CRUD)
- **Dữ liệu / Attribute sử dụng (khi khởi tạo object Employee từ View `AddEmployee_Dialog` / `EditEmployee_Dialog`):**
  - `inpName` (QLineEdit): Tên nhân viên
  - `cmbRole` (QComboBox): Chức vụ (Quản lý, Thu ngân...)
  - `cmbGender` (QComboBox): Giới tính
  - `inpPhone` (QLineEdit): Số điện thoại
  - `inpDob` (QDateEdit): Ngày sinh
  - `inpAddress` (QLineEdit): Địa chỉ
  - `inpCitizenId` (QLineEdit): CCCD
  - `cmbIsFixedSalary` (QComboBox): Loại lương (Cố định / Theo giờ)
  - `inpSalary` (QLineEdit): Mức lương
  - `m_avatarPath` (QString): Đường dẫn ảnh đại diện
  - `inpUsername`, `inpPassword` (QLineEdit): Thông tin đăng nhập tự động sinh
  - Các thuộc tính View này map trực tiếp vào object `User/Employee` để update Database.
- **Được gọi tới từ:** Action buttons trên UI gọi các slot của `Employee_Control` (`handleAddEmployee`, `handleEditEmployee`, `handleDeleteEmployee`).
- **Sẽ gọi hàm:**
  - `Employee_Model::addEmployee(...)` / `updateEmployee(...)` / `deleteEmployee(...)`.
  - `Validator::isValidCitizenId()`, `Validator::isValidDate()`, `Validator::isValidPhoneNumber()`.
  - Sinh tự động Username/Password qua delegate.
- **Công dụng:** Thêm mới (INSERT), Cập nhật (UPDATE), hoặc Xóa (DELETE) nhân viên. Quá trình Thêm/Sửa được Validate cực kỳ khắt khe từ Controller thông qua delegate.

---

### 2.5. Trang Đăng Ký / Xếp Lịch (Schedule)

Trang này có 2 giao diện tùy vào quyền của user: Staff đăng ký ca, Manager xếp lịch.

#### Tính Năng: Đăng ký ca làm (Staff)
- **Dữ liệu / Attribute sử dụng:**
  - `PROFILES`: `isFixed` (True: Full-time lương cố định, False: Part-time tính giờ).
  - Lưới thời gian: `col` (0-6 cho Thứ 2 - CN), `row` (ca trong ngày).
- **Được gọi tới từ:** Staff bấm nút Lưu, trigger `Schedule_Control::onSaveGridRequested()`.
- **Sẽ gọi hàm:**
  - `Schedule_Model::ensurePendingCarryForwardForWeek()` (Copy lịch cũ qua tuần mới tự động).
  - `Schedule_Model::replacePendingShiftsForWeek()`.
- **Công dụng:** Xóa những ca Pending hiện tại của tuần đăng ký, thay thế bằng danh sách ca Staff vừa chọn trên Grid. Các ca đã Approved (status=1) sẽ bị khóa, không được xóa/sửa. Cuối cùng COMMIT Database.

#### Tính Năng: Xếp lịch tự động (Manager)
- **Dữ liệu / Attribute sử dụng:**
  - Thuộc tính `status`, `startTime`, `endTime`, `date` của bảng `SHIFT`.
  - Quota tối thiểu nhân viên/ca từ bảng cấu hình `SETTINGS`.
- **Được gọi tới từ:** Nút "Tự động xếp lịch", trigger `Schedule_Control::handleGenSchedule()`.
- **Sẽ gọi hàm:**
  - `Schedule_Model::previewGeneratedSchedule(weekStart)`.
  - `Optimizer::solve()` -> `Optimizer::minCostFlow()`.
- **Công dụng:** Áp dụng mô hình đồ thị và thuật toán Max Flow / Min Cost Flow để tự động xếp nhân viên vào các ca còn trống sao cho thỏa mãn điều kiện tối thiểu và không vi phạm giờ làm. Trả về preview trước khi apply.

#### Tính Năng: Duyệt ca / Hủy ca (Manager)
- **Dữ liệu / Attribute sử dụng:** `shiftId` từ bảng `SHIFT`.
- **Được gọi tới từ:** Manager click "Duyệt" / "Từ chối" trên một `ShiftBlock`, trigger `Schedule_Control::onApproveShift()` hoặc `onDeclineShift()`.
- **Sẽ gọi hàm:** `Schedule_Model::approveShift(shiftId)` hoặc `declineShift(shiftId)`.
- **Công dụng:** `UPDATE SHIFT SET status = 1` (Duyệt) hoặc `-1` (Từ chối).

---

### 2.6. Trang Xem Lịch (View Schedule)

#### Tính Năng: Hiển thị lịch (Staff vs Manager)
- **Dữ liệu / Attribute sử dụng:** Các dòng `SHIFT` map vào ma trận 7 ngày.
- **Được gọi tới từ:** Khi vào trang, gọi `ViewSchedule_Control::load()`.
- **Sẽ gọi hàm:**
  - (Với Staff) `Schedule_Model::getRawStaffShifts(id, monday, status)`. Lấy các ca approved, pending, declined riêng rẽ.
  - (Với Manager) `Schedule_Model::getManagerWeeklyGrid(monday)`.
- **Công dụng:** Hiển thị thời gian biểu. Với Manager, grid hiển thị các block (Đủ NV / Thiếu NV). Click vào block sẽ mở chi tiết những ai đang trực ca đó.

#### Tính Năng: Tìm người thay thế (Manager)
- **Dữ liệu / Attribute sử dụng:** `shiftId` bị nghỉ đột xuất, `role` yêu cầu.
- **Được gọi tới từ:** `ViewSchedule_Control::onShowReplacementsRequested()`.
- **Sẽ gọi hàm:**
  - `Schedule_Model::getEligibleReplacements(oldShiftId, role)`.
  - `Schedule_Model::replaceShift(oldShiftId, newShiftId)`.
- **Công dụng:** Khi một NV nghỉ, Manager bấm thay thế. Model truy vấn các NV đang rảnh giờ đó (Eligible). Sau khi chọn người thay, Model thực thi UPDATE DB đổi `idEmployee` của ca làm, bảo toàn giờ làm.

---

### 2.7. Trang Lương (Salary)

#### Tính Năng: Xem chi tiết lương (Tháng/Năm)
- **Dữ liệu / Attribute sử dụng:**
  - `PROFILES`: `baseSalary`, `isFixed`, `role`.
  - `SHIFT`: Trạng thái đã duyệt, tổng số giờ làm (Part-time) hoặc số ngày làm (Full-time).
- **Được gọi tới từ:** Mở trang hoặc đổi combo-box tháng/năm, trigger `Salary_Control::loadData()`.
- **Sẽ gọi hàm:**
  - `Salary_Model::getSalarySummary()`.
  - `Salary_Model::getNormalDaysData()`.
  - `Salary_Model::getHolidayDaysData()`.
- **Công dụng:** Tính toán tổng số giờ làm (ngày thường vs lễ), nhân hệ số (lễ x2), trừ tiền phạt (nếu có), và gom thành cấu trúc `SalaryData`. Dữ liệu đổ ra 3 bảng: Tổng kết, Chi tiết ngày thường, Chi tiết ngày lễ.

---

### 2.8. Trang Thông Báo (Notification)

#### Tính Năng: Quản lý thông báo
- **Dữ liệu / Attribute sử dụng:**
  - `NOTIFICATIONS`: `id`, `recipient_id`, `type`, `title`, `message`, `is_read`.
- **Được gọi tới từ:** Timer refresh hoặc người dùng click xóa/đọc. `Notification_Control` handle các action này.
- **Sẽ gọi hàm:**
  - `Notification_Model::getNotifications(empId)`.
  - `Notification_Model::markAsRead()`, `Notification_Model::deleteAllRead()`.
  - `Notification_Model::create()` (Hàm tĩnh để sinh thông báo từ các tính năng khác).
- **Công dụng:** Hiển thị badge số lượng thông báo chưa đọc. Hỗ trợ hệ thống gửi thông báo tự động (vd: thiếu người, đã duyệt nghỉ phép). Tính năng chống trùng lặp thông báo (Deduplication bằng `dedupeKey`).

---

### 2.9. Trang Cài Đặt (Setting)

Chỉ có Manager / Admin mới vào được trang này.

#### Tính Năng: Lưu cấu hình hoạt động của quán
- **Dữ liệu / Attribute sử dụng (khởi tạo cấu hình Settings từ View):**
  - `ui->spinBoxOpen` / `ui->spinBoxClose` (QSpinBox): Giờ mở / đóng cửa
  - `ui->comboBoxDateRegis` (QComboBox): Ngày mở đăng ký lịch làm việc
  - `ui->comboBoxFullTime` (QComboBox): Số ngày nghỉ phép tối đa
  - `ui->spinBoxDayMaxPartTime` / `ui->spinBoxHourMaxPartTime` (QSpinBox): Giới hạn part-time
  - `ui->tableRoles` (QTableWidget) có nhúng `QSpinBox`: Số nhân viên tối thiểu/tối đa cho từng chức vụ
- **Được gọi tới từ:** Nút Save, trigger `Setting_Control::handleSave()`.
- **Sẽ gọi hàm:**
  - `Setting_Model::saveSettings()`.
  - `Config::reload()`.
- **Công dụng:** Cập nhật DB (UPDATE SETTINGS), sau đó gọi `Config::reload()` để nạp lại biến static trong singleton, giúp toàn bộ hệ thống áp dụng giờ/lương mới ngay lập tức mà không cần restart app.

---

## 3. Bảng Tổng Hợp Hàm Toàn Hệ Thống

| Nhóm Tính Năng | Tên Tính Năng | Trigger (Signal/Action) | Controller Xử Lý (Callee) | Model Hàm (Called) | Mục Đích Database / Logic |
|---|---|---|---|---|---|
| **Auth** | Đăng nhập | `loginSubmitted` | `handleLoginSubmission` | `verifyLogin` | So sánh Hash, SELECT ACCOUNTS, PROFILES |
| **Dashboard** | Ca làm hiện tại | `init() / autoRefresh()` | `loadEmployeeCards` | `getWorkingEmployeeIds` | Lấy IDs từ SHIFT theo currentTime |
| **Dashboard** | Ca làm kế tiếp | `init() / autoRefresh()` | `loadShiftPanel` | `getNextShiftEmployees` | SELECT SHIFT cho startTime > currentTime |
| **Dashboard** | Thống kê lương | Tab năm thay đổi | `loadSalaryChart` | `getSalaryStats` | Gom lương 12 tháng, cache DTO |
| **Dashboard** | Duyệt nghỉ phép | Nút Duyệt/Từ chối | `reviewLeaveRequest` | `decideLeaveRequest` | UPDATE LEAVE_REQUESTS, sinh Noti |
| **Profile** | Sửa hồ sơ | Lưu cập nhật | `handleProfileUpdate` | `updateProfile` | Lưu ảnh nội bộ, UPDATE PROFILES |
| **Profile** | Đổi mật khẩu | Xác nhận mật khẩu | `handlePasswordUpdate` | `updatePassword` | Validate strength, Hash, UPDATE ACCOUNTS |
| **Nhân Sự** | Tìm/Lọc NV | Ô Search / Dropdown Filter | `handleUpdate` | `SearchSortFilter` | Xử lý Rabin-Karp trong bộ nhớ |
| **Nhân Sự** | Thêm NV | Nút Thêm NV | `handleAddEmployee` | `addEmployee` | Check valid, Auto-gen pass, INSERT |
| **Nhân Sự** | Sửa NV | Icon Edit trên hàng NV | `handleEditEmployee` | `updateEmployee` | Tiêm Validate delegate, UPDATE PROFILES |
| **Nhân Sự** | Xóa NV | Icon Delete trên hàng NV | `handleDeleteEmployee` | `deleteEmployee` | QMessageBox confirm, DELETE |
| **Xếp Lịch** | Đăng ký ca (Staff)| Nút Lưu (Schedule_View)| `onSaveGridRequested` | `replacePendingShifts...` | Xóa pending cũ, INSERT lịch mới đăng ký |
| **Xếp Lịch** | Duyệt/Hủy ca | Manager bấm duyệt/từ chối| `onApproveShift / onDecline` | `approveShift / declineShift`| UPDATE SHIFT set status = 1 / -1 |
| **Xếp Lịch** | Xếp tự động | Nút Tạo lịch tự động | `handleGenSchedule` | `previewGeneratedSchedule` | Gọi `Optimizer::solve` thuật toán MinCostFlow|
| **Xem Lịch** | Xem ca (Manager) | Chuyển trang/tuần | `loadManagerSchedule` | `getManagerWeeklyGrid` | Gom nhóm ca, tính toán thiếu/đủ NV |
| **Xem Lịch** | Thay thế NV | Chọn người thay ca | `onConfirmReplacement` | `replaceShift` | Đổi idEmployee của 1 ca trong DB |
| **Lương** | Tính lương NV | Thay đổi T/Năm trên View | `loadData`, `onMonthYearChanged`| `getSalarySummary`, ... | Nhân số giờ, áp dụng x2 ngày lễ, tính phạt |
| **Thông Báo**| Quản lý Noti | Click Đọc/Xóa | `load` | `getNotifications` | Tải NOTIFICATIONS và đánh dấu is_read |
| **Cài Đặt** | Đổi Setting | Nút Lưu config | `handleSave` | `saveSettings`, `Config::reload()`| Ghi DB và Reload in-memory global Config |

---
**Document Last Updated:** 06-Aug-2026.
