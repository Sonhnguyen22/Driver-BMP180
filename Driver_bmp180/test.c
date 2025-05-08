#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <errno.h> // Include errno header

// IOCTL commands
#define DEVICE_PATH "/dev/bmp180"
#define bmp180_IOCTL_MAGIC 'b'
#define bmp180_IOCTL_WRITE_OSS  _IOR(bmp180_IOCTL_MAGIC, 1, int)
#define bmp180_IOCTL_READ_CALIB _IOR(bmp180_IOCTL_MAGIC, 2, int)
#define bmp180_IOCTL_READ_T     _IOR(bmp180_IOCTL_MAGIC, 3, int)
#define bmp180_IOCTL_READ_P     _IOR(bmp180_IOCTL_MAGIC, 4, int)
#define bmp180_IOCTL_READ_TP    _IOR(bmp180_IOCTL_MAGIC, 5, int)

struct {
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int16_t MB;
    int16_t MC;
    int16_t MD;

    int32_t UT;
    int32_t UP;
}Calib;

int fd;
int32_t T, P, TP[2];
uint8_t OSS = 1;
uint8_t Test_Driver(void);

float bmp180_calculate_temp(int Clien, uint8_t Oss);
int32_t bmp180_calculate_pres(int Clien, uint8_t Oss);
int32_t bmp180_calculate_temp_and_pres(int Clien, uint8_t Oss, float *Temp, int32_t *Pres);
float bmp180_calculate_alti(int Clien, uint8_t Oss);

int main() {
    
    // Open the device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    Test_Driver();
    
    while(1){
        
        bmp180_calculate_alti(fd, 1);

    }

    // Close the device
    close(fd);
    return 0;
}

uint8_t Test_Driver(void){
    // ghi gia tri oss xuong kernel
    if (ioctl(fd, bmp180_IOCTL_WRITE_OSS, &OSS) < 0) {
        perror("Failed to write oss");
        close(fd);
        return errno;
    }

    // Read Calib theo thu tu AC1 AC2 AC3 AC4 AC5 AC6 B1 B2 MB MC MD UT UP
    if (ioctl(fd, bmp180_IOCTL_READ_CALIB, &Calib) < 0) {
        perror("Failed to read Calib");
        close(fd);
        return errno;
    }
    printf("AC1: %d UP: %d", Calib.AC1, Calib.UP);

    // Read Temp
    if (ioctl(fd, bmp180_IOCTL_READ_T, &T) < 0) {
        perror("Failed to read Temp");
        close(fd);
        return errno;
    }
    printf("Tempx10: %d\n", T);

    // Read Pres
    if (ioctl(fd, bmp180_IOCTL_READ_P, &P) < 0) {
        perror("Failed to read Pres");
        close(fd);
        return errno;
    }
    printf("Pres: %d\n", P);

    // Read Temp and Pres
    if (ioctl(fd, bmp180_IOCTL_READ_TP, &TP) < 0) {
        perror("Failed to read TP");
        close(fd);
        return errno;
    }
    printf("Tempx10: %d Pres: %d\n", TP[0], TP[1]);

    sleep(10);

    return 0;
}

float bmp180_calculate_temp(int Clien, uint8_t Oss){
    int32_t Temp;
    // ghi gia tri oss xuong kernel
    if (ioctl(Clien, bmp180_IOCTL_WRITE_OSS, &Oss) < 0) {
        perror("Failed to write oss");
        close(Clien);
        return errno;
    }
    if (ioctl(Clien, bmp180_IOCTL_READ_T, &Temp) < 0) {
        perror("Failed to read Temp");
        close(Clien);
        return errno;
    }
    return (float)(Temp/10.0f);
}

int32_t bmp180_calculate_pres(int Clien, uint8_t Oss){
    int32_t Press;
    // ghi gia tri oss xuong kernel
    if (ioctl(Clien, bmp180_IOCTL_WRITE_OSS, &Oss) < 0) {
        perror("Failed to write oss");
        close(Clien);
        return errno;
    }
    if (ioctl(Clien, bmp180_IOCTL_READ_P, &Press) < 0) {
        perror("Failed to read Temp");
        close(Clien);
        return errno;
    }
    return Press;
}

int32_t bmp180_calculate_temp_and_pres(int Clien, uint8_t Oss, float *Temp, int32_t *Pres){
    int32_t Data[2];
    // ghi gia tri oss xuong kernel
    if (ioctl(Clien, bmp180_IOCTL_WRITE_OSS, &Oss) < 0) {
        perror("Failed to write oss");
        close(Clien);
        return errno;
    }
    if (ioctl(Clien, bmp180_IOCTL_READ_TP, &Data) < 0) {
        perror("Failed to read Temp");
        close(Clien);
        return errno;
    }
    *Temp = (float)(Data[0]/10.0f);
    *Pres = Data[1];
}

float bmp180_calculate_alti(int Clien, uint8_t Oss){
    float alti_abs, Pres, Po = 101325.0f;
    
    Pres = bmp180_calculate_pres(Clien, Oss);
    alti_abs = 44330 * (1 - powf((P/Po),1/5.255f));
    return alti_abs;
}



