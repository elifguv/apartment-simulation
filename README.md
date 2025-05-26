# 🏢 Apartman İnşaatı Simülasyonu – İşletim Sistemleri Dönem Projesi 

Bu proje, işletim sistemleri dersi kapsamında çoklu işlem (process), çoklu iş parçacığı (thread) ve senkronizasyon (mutex, semaphore) kavramlarını modellemek amacıyla C dili kullanılarak geliştirilmiştir.

---

## Proje Tanımı

Projede 10 katlı ve her katta 4 daire bulunan bir apartmanın inşaat süreci simüle edilmiştir.

- Her kat bir process'i ve her daire o kat içinde çalışan bir thread'i temsil eder. 
- Apartman inşaat işlemleri elektrik tesisatı, su tesisatı, boyama, kapı-pencere montajı ve vinç kullanımı olarak tasarlanmıştır. 
- Tüm adımlar paylaşılan kaynaklara senkronize erişim ile gerçekleştirilmiştir.

---

## Senkronizasyon Detayları

| Kaynak                 | Tür           | Erişim Mekanizması      |
|------------------------|---------------|--------------------------|
| Asansör               | Tekli kaynak  | `pthread_mutex_t`       |
| Tek vinç              | Özel kullanım | `pthread_mutex_t`       |
| Çoklu vinç            | Çoklu kaynak | `sem_t` (3 kapasiteli)  |
| Kapı/Pencere Montajı  | Kısıtlı ekip  | `sem_t` (2 kapasiteli)  |

---

## Simülasyon Akışı 

1. Temel atılır (`sleep(2)`).
2. Her kat için bir process oluşturulur.
3. Her katta 4 thread başlatılır.
4. Tüm işlemler log dosyasına yazılır.
5. Her kaynak (asansör, vinç vs.) yarış durumlarına karşı senkronize edilir.

---

## Terminal Çıktısı

Terminalde renkli ve simgeli bilgiler gösterilir.
- Başlama
- Kat tamamlandı
- Daire tamamlandı

Tüm çıktılar `[saat:dakika:saniye]` formatında zaman bilgisiyle birlikte gösterilir. Ayrıca `master_log.txt` dosyasına tüm işlem adımları zaman damgalı olarak kaydedilir.

---

## Derleme ve Çalıştırma

```bash
gcc -o apartment apartment.c -lpthread
./apartment
