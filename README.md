# Driver Raspberry Pi 5  
Driver BMP180  
This is Driver for Bosch BMP180 barometric pressure & temperature sensor
- Supply voltage:         1.8v - 3.6v  
- Range:                  30,000Pa..110,000Pa at 0°C..+65°C
- Typ. resolution:        1Pa    / 0.1°C
- Typ. accuracy:          ±100Pa / ±1.0°C
- Typ. relative accuracy: ±12Pa  / xx°C

Chức năng driver:  
- 1 Đọc và gửi các giá trị Calibration  
- 2 Tính và gửi giá trị nhiệt độ (đơn vị 0.1 độ vd: T = 150 ==> 15 độ C)  
- 3 Tính và gửi giá trị áp suất (đơn vị Pa)  
- 4 Tính và gửi đồng thời giá trị nhiệt độ và áp suất
  
Cách dùng driver:  
- 1 Đọc các giá trị Calibration AC1->MD và UT, UP:
  - tạo 1 struc như ví dụ để lưu giá trị đọc về
  - sử dụng hàm ioctl(fd, _IOW('b', 1, int), & oss) để ghi giá trị oss (0->3)  
  - sử dụng hàm ioctl(fd, _IOR('b', 2, int), & your struc) để đọc data và lưu vào your struc  
- 2 đọc giá trị Temp
  - tạo một biến int32_t Temp
  - sử dụng hàm ioctl(fd, _IOR('b', 3, int), & Temp) để đọc data và lưu vào Temp
- 3 đọc giá trị Pres
  - Tạo một biến int32_t Press  
  - sủ dụng hàm ioctl(fd, _IOW('b', 1, int), & oss) để ghi giá trị oss  
  - sủ dụng hàm ioctl(fd, _IOR('b', 4, int), & Temp) để đọc data và lưu vào Pres  
- 4 đọc gia trị nhiêt độ và áp suất đồng thời  
  - tạo một mảng 2 phần từ int32_t data[2]
  - sủ dụng hàm ioctl(fd, _IOW('b', 1, int), & oss) để ghi giá trị oss
  - sử dụng hàm ioctl(fd, _IOR('b', 5, int), & data) để đọc data và lưu vào data (data[0]: Temp (0.1 độ), data[2]: Pres (Pa))
 
