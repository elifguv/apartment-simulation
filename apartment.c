/* 
  Bu program, işletim sistemleri dersi kapsamında
  çoklu işlem (process), çoklu iş parçacığı (thread)
  ve senkronizasyon mekanizmalarını modellemek amacıyla
  10 katlı bir apartman inşaatı simülasyonu gerçekleştirir.
  Her kat bir process, her daire ise bir thread olarak modellenmiştir.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>

#define NUM_FLOORS 10 //toplam kat sayısı
#define NUM_APARTMENTS 4 //her kattaki daire sayısı
#define LOG_FILE "master_log.txt"

//terminal çıktı renkleri
#define YELLOW   "\033[1;33m"
#define GREEN    "\033[1;32m"
#define CYAN     "\033[1;36m"
#define MAGENTA  "\033[1;35m"
#define RESET    "\033[0m"

//global kaynaklar
pthread_mutex_t elevator_mutex; //tüm dairelerin ortak kullandığı tek bir asansör için mutex
pthread_mutex_t single_crane_mutex; //tek bir özel vinç için oluşturulan mutex
sem_t multi_crane_sem; //aynı anda birden fazla thread'in kullanabileceği vinçler için semaphore 
sem_t window_door_sem; //2 işçinin aynı anda kapı/pencere takabileceği semaphore

typedef struct {
    int floor_no;
    int apt_no;
    FILE *log;
} ThreadData;

//log dosyasına zaman yazan fonksiyon
void log_with_time(FILE *log, const char *format, ...) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);

    va_list args;
    va_start(args, format);
    vfprintf(log, format, args);
    va_end(args);
    fprintf(log, "\n");
    fflush(log);
}

//terminale renkli ve zaman damgalı çıktı veren fonksiyon
void log_important_colored(const char *color, const char *format, ...) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);

    printf("%s%s", color, timestamp);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s\n", RESET);
    fflush(stdout);
}

//thread fonksiyonu - dairenin inşa süreci
void *build_apartment(void *arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL); //süre ölçümü için başlangıç zamanı alır

    ThreadData *data = (ThreadData *)arg;
    FILE *log = data->log;

    char apt_label[8];
    snprintf(apt_label, sizeof(apt_label), "%d.%d", data->floor_no, data->apt_no); //daire etiketi oluşturur

    log_with_time(log, "[Apartment %s] Starting", apt_label);
    log_important_colored(YELLOW, "🟢 Apartment %s started", apt_label);

    pthread_mutex_lock(&elevator_mutex); //asansör kilitlenir, kullanımı başlatılır
    log_with_time(log, "[Apartment %s] Using elevator for wiring", apt_label);
    usleep((rand() % 400 + 100) * 1000); //işlem süresi rastgele belirlenir
    pthread_mutex_unlock(&elevator_mutex);

    log_with_time(log, "[Apartment %s] Plumbing", apt_label);
    usleep((rand() % 400 + 100) * 1000);

    log_with_time(log, "[Apartment %s] Painting", apt_label);
    usleep((rand() % 400 + 100) * 1000);

    sem_wait(&window_door_sem); //kapı&pencere montajı için semafor kilitlenir
    log_with_time(log, "[Apartment %s] Installing windows/doors", apt_label);
    usleep((rand() % 400 + 100) * 1000);
    sem_post(&window_door_sem);

    sem_wait(&multi_crane_sem);
    log_with_time(log, "[Apartment %s] Using one of the multi-cranes", apt_label);
    usleep((rand() % 400 + 100) * 1000);
    sem_post(&multi_crane_sem);

    pthread_mutex_lock(&single_crane_mutex);
    log_with_time(log, "[Apartment %s] Using single crane", apt_label);
    usleep((rand() % 400 + 100) * 1000);
    pthread_mutex_unlock(&single_crane_mutex);

    log_with_time(log, "[Apartment %s] Finished", apt_label);

    gettimeofday(&end, NULL); //süre ölçümü için bitiş zamanı alır
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6; //geçen süre saniye cinsinden hesaplanır
    log_important_colored(GREEN, "✅ Apartment %s finished in %.2fs", apt_label, elapsed);
    return NULL;
}

//kat süreci
void construct_floor(int floor_no) {
    FILE *floor_log = fopen(LOG_FILE, "a"); //log dosyasını ekleme modunda açar
    if (!floor_log) {
        perror("Failed to open log file");
        exit(1);
    }

    log_with_time(floor_log, "[Floor %d] Construction started", floor_no); //kat inşaatı başlatıldığında log dosyasına yazar
    log_important_colored(CYAN, "🏗️ Floor %d started", floor_no);

    pthread_t threads[NUM_APARTMENTS]; //kat içi daireler için thread dizisi
    ThreadData thread_data[NUM_APARTMENTS];

    for (int i = 0; i < NUM_APARTMENTS; i++) {
        thread_data[i].floor_no = floor_no;
        thread_data[i].apt_no = i + 1;
        thread_data[i].log = floor_log;
        pthread_create(&threads[i], NULL, build_apartment, &thread_data[i]); //her daireye ait thread kendi kat&daire bilgileriyle başlatılır
    }

    for (int i = 0; i < NUM_APARTMENTS; i++) {
        pthread_join(threads[i], NULL); //thread tamamlanana kadar bekler
    }

    log_with_time(floor_log, "[Floor %d] Construction complete", floor_no);
    log_important_colored(MAGENTA, "🏁 Floor %d completed", floor_no);
    fclose(floor_log);
}

// 
int main() {
    pthread_mutex_init(&elevator_mutex, NULL);
    pthread_mutex_init(&single_crane_mutex, NULL);
    sem_init(&multi_crane_sem, 0, 3);
    sem_init(&window_door_sem, 0, 2);

    FILE *clear_log = fopen(LOG_FILE, "w");
    if (clear_log) fclose(clear_log);

    printf("Starting foundation...\n");
    sleep(2); //temel atımı için gecikme süresi
    printf("Foundation complete!\n");

    for (int i = 0; i < NUM_FLOORS; i++) {
        pid_t pid = fork(); //her kat için yeni bir process oluşturur

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) { //child process
            construct_floor(i + 1);
            exit(0);
        } else {
            waitpid(pid, NULL, 0); //üst kata geçmeden önce alt katın inşaasının bitmesini bekler
        }
    }

    pthread_mutex_destroy(&elevator_mutex);
    pthread_mutex_destroy(&single_crane_mutex);
    sem_destroy(&multi_crane_sem);
    sem_destroy(&window_door_sem);

    printf("\n🏢 Apartment Building Construction Complete!\n");
    return 0;
}
