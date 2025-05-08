# BT Driver Raspberry Pi 5
Driver BMP180 
Chức năng driver:  
  1 Đọc và gửi các giá trị Calibration  
  2 Tính và gửi giá trị nhiệt độ (đơn vị 0.1 độ vd: T = 150 ==> 15 độ C)  
  3 Tính và gửi giá trị áp suất (đơn vị Pa)  
  4 Tính và gửi đồng thời giá trị nhiệt độ và áp suất  
Cách dùng driver:  
  1 Đọc các giá trị Calibration AC1->MD và UT, UP  
    tạo 1 struc như ví dụ để lưu giá trị đọc về  
     sủ dụng hàm ioctl(fd, bmp180_IOCTL_READ_TP, &TP)  
