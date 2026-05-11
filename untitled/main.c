#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KRITIK_STOK_ESIGI 10
#define MAX_HEAP_BOYUT 100

typedef struct {
    int barkod;
    char ilacAdi[50];
    int stokAdedi;
    int receteTipi;
} HeapDugum;

// Min-Heap'in ana yapısı
struct MinHeap {
    HeapDugum harr[MAX_HEAP_BOYUT];
    int capacity;
    int heap_size;
};

struct MinHeap kritikHeap;

// Ana Envanter için İkili Arama Ağacı Yapısı
typedef struct Ilac {
    int barkodNo;
    char ilacAdi[50];
    int stokAdedi;
    float fiyat;
    int receteTipi;
    struct Ilac* sol;
    struct Ilac* sag;
} Ilac;

// İade işlemleri
typedef struct Satis {
    int barkodNo;
    char ilacAdi[50];
    int adet;
    struct Satis* alt;
} Satis;

float gunlukCiro = 0.0;
int toplamSatisAdedi = 0;
Satis* sonSatislar = NULL;  // Stack'te en son yapılan işlem

//Heap Yardımcı Fonksiyonları
int parent(int i) { return (i - 1) / 2; }
int left(int i) { return (2 * i + 1); }
int right(int i) { return (2 * i + 2); }

//Değer olarak sadece sayı girilmesini sağlayan fonksiyon
int sayiAl(const char* mesaj) {
    int sayi;
    printf("%s", mesaj);

    while (scanf("%d", &sayi) != 1) {
        printf("Hatali giris.\nLutfen sadece sayi giriniz: ");
        while(getchar() != '\n');
    }
    return sayi;
}

void MinHeapify(struct MinHeap* minHeap, int i) {
    int l = left(i);
    int r = right(i);
    int smallest = i;

    //Stok adetine göre en küçüğü bulan kontrol döngüleri
    if (l < minHeap->heap_size && minHeap->harr[l].stokAdedi < minHeap->harr[smallest].stokAdedi)
        smallest = l;
    if (r < minHeap->heap_size && minHeap->harr[r].stokAdedi < minHeap->harr[smallest].stokAdedi)
        smallest = r;

    if (smallest != i) {
        HeapDugum temp = minHeap->harr[i];
        minHeap->harr[i] = minHeap->harr[smallest];
        minHeap->harr[smallest] = temp;
        MinHeapify(minHeap, smallest);
    }
}

// Heap'in sonuna yeni ilaç ekler ve gerekirse yukarı doğru kaydırır
void insertKey(struct MinHeap* minHeap, HeapDugum k) {
    if (minHeap->heap_size == minHeap->capacity) {
        printf("\nİnsert key eklenemedi.\n");
        return;
    }

    int i = minHeap->heap_size++;
    minHeap->harr[i] = k;

    while (i != 0 && minHeap->harr[parent(i)].stokAdedi > minHeap->harr[i].stokAdedi) {
        HeapDugum temp = minHeap->harr[i];
        minHeap->harr[i] = minHeap->harr[parent(i)];
        minHeap->harr[parent(i)] = temp;
        i = parent(i);
    }
}

// Heap'in tepesindeki en küçük stoklu ilacı alır
HeapDugum extractMin(struct MinHeap* minHeap) {
    if (minHeap->heap_size == 1) {
        minHeap->heap_size--;
        return minHeap->harr[0];
    }

    HeapDugum root = minHeap->harr[0];
    minHeap->harr[0] = minHeap->harr[minHeap->heap_size - 1];
    minHeap->heap_size--;
    MinHeapify(minHeap,0);

    return root;
}

// Tüm İkili Arama Ağacını gezip stoğu 10'dan az olanları Heap'e atar
void kritikStokAra(Ilac* kok, struct MinHeap* heap) {
    if (kok == NULL) return;

    kritikStokAra(kok->sol, heap);

    if (kok->stokAdedi < KRITIK_STOK_ESIGI) {
        HeapDugum yeni;
        yeni.barkod = kok->barkodNo;
        strcpy(yeni.ilacAdi, kok->ilacAdi);
        yeni.stokAdedi = kok->stokAdedi;
        yeni.receteTipi = kok->receteTipi;

        insertKey(heap, yeni);
    }
    kritikStokAra(kok->sag, heap);
}

//Heap'i sıfırlayıp baştan güvenli bir şekilde doldurur
void heapGuncelle(Ilac* kok) {
    kritikHeap.heap_size = 0;   //Sıfırlama yaparak listeyi temizler
    kritikStokAra(kok, &kritikHeap);   //Ağacı tekrardan tarar
}

// Min-Heap yapısını kullanarak ilaçları küçükten büyüğe yazdırır
void kritikStoklariGoster(struct MinHeap* heap) {
    if (heap->heap_size == 0) {
        printf("\n Kritik stok uyarisi bulunmuyor.\n");
        return;
    }

    printf("\nKRITIK STOK UYARILARI :\n");
    struct MinHeap gecici = *heap;
    int sira = 1;

    while (gecici.heap_size > 0) {
        HeapDugum en_az = extractMin(&gecici);

        char tip[15];
        if (en_az.receteTipi == 1) strcpy(tip, "YESIL");
        else if (en_az.receteTipi == 2) strcpy(tip, "KIRMIZI");
        else strcpy(tip, "NORMAL");

        printf("%d. %-15s | Barkod: %-6d | Stok: %2d adet | [%s]\n",sira++, en_az.ilacAdi, en_az.barkod, en_az.stokAdedi, tip);
    }
}

// Barkod numarasına göre ağaca yeni ilaç ekler
Ilac* ilacEkle(Ilac* kok, int barkod, const char* isim, int stok, float fiyat, int recete) {
    if (kok == NULL) {
        Ilac* yeniIlac = (Ilac*)malloc(sizeof(Ilac));
        if (yeniIlac == NULL) {
            return NULL;
        }
        yeniIlac->barkodNo = barkod;
        strcpy(yeniIlac->ilacAdi, isim);
        yeniIlac->stokAdedi = stok;
        yeniIlac->fiyat = fiyat;
        yeniIlac->receteTipi = recete;
        yeniIlac->sol = NULL;
        yeniIlac->sag = NULL;
        return yeniIlac;
    }
    if (barkod < kok->barkodNo) {
        kok->sol = ilacEkle(kok->sol, barkod, isim, stok, fiyat, recete);
    }
    else if (barkod > kok->barkodNo) {
        kok->sag = ilacEkle(kok->sag, barkod, isim, stok, fiyat, recete);
    }
    return kok;
}

 //Barkoda göre ilaç arar
Ilac* ilacAra(Ilac* kok, int barkod) {
    if (kok == NULL || kok->barkodNo == barkod) {
        return kok;
    }
    if (barkod < kok->barkodNo) {
        return ilacAra(kok->sol, barkod);
    }
    return ilacAra(kok->sag, barkod);
}

//Ağacın en küçük değerini bulur
Ilac* minDeger(Ilac* dugum) {
    Ilac* guncel = dugum;
    while (guncel && guncel->sol != NULL)
        guncel = guncel->sol;
    return guncel;
}

//Ağaçtan ilaç siler ve ağacın yapısını korur
Ilac* ilacSil(Ilac* kok, int barkod) {
    if (kok == NULL)
        return kok;

    if (barkod < kok->barkodNo)
        kok->sol = ilacSil(kok->sol, barkod);

    else if (barkod > kok->barkodNo)
        kok->sag = ilacSil(kok->sag, barkod);

    else {
        if (kok->sol == NULL) {
            Ilac* gecici = kok->sag;
            free(kok);
            return gecici;
        }
        else if (kok->sag == NULL) {
            Ilac* gecici = kok->sol;
            free(kok);
            return gecici;
        }
        //Çift çocuklu bir kökse
        Ilac* gecici = minDeger(kok->sag);
        kok->barkodNo = gecici->barkodNo;
        strcpy(kok->ilacAdi, gecici->ilacAdi);
        kok->stokAdedi = gecici->stokAdedi;
        kok->fiyat = gecici->fiyat;
        kok->receteTipi = gecici->receteTipi;
        kok->sag = ilacSil(kok->sag, gecici->barkodNo);
    }
    return kok;
}

void agaciTemizle(Ilac* kok) {
    if (kok != NULL) {
        agaciTemizle(kok->sol);
        agaciTemizle(kok->sag);
        free(kok);
    }
}

Ilac* dosyadanOku(Ilac* kok) {
    FILE* dosya = fopen("eczane.txt", "r");

    if (dosya == NULL) {
        printf("\n Dosya bulunamadi, envanter bos\n");
        return kok;
    }

    int barkod, stok, recete;
    float fiyat;
    char isim[50];

    while (fscanf(dosya, "%d,%[^,],%d,%f,%d", &barkod, isim, &stok, &fiyat, &recete) == 5) {
        kok = ilacEkle(kok, barkod, isim, stok, fiyat, recete);
    }
    fclose(dosya);
    return kok;
}

void dosyayaYaz(Ilac* kok, FILE* dosya) {
    if (kok != NULL) {
        fprintf(dosya, "%d,%s,%d,%.2f,%d\n", kok->barkodNo, kok->ilacAdi, kok->stokAdedi, kok->fiyat, kok->receteTipi);
        dosyayaYaz(kok->sol, dosya);
        dosyayaYaz(kok->sag, dosya);
    }
}

//İlacın stok veya fiyat bilgisini günceller
void ilacGuncelle(Ilac* kok) {
    int barkod;
    barkod = sayiAl("Guncellenecek Ilacin Barkodunu Girin: ");

    Ilac* bulunan = ilacAra(kok, barkod);
    if (bulunan == NULL) {
        printf(" %d barkodlu ilac bulunamadi.\n", barkod);
        return;
    }

    int secim;
    char receteAdi[25];

     if (bulunan -> receteTipi == 0)
        strcpy(receteAdi, "Normal Recete");
    else if (bulunan->receteTipi == 1)
        strcpy(receteAdi, "Yesil Recete");
    else if (bulunan->receteTipi == 2)
        strcpy(receteAdi, "Kirmizi Recete");

    printf("\n     Mevcut Bilgiler  \n");
    printf("Ad: %s | Stok: %d | Fiyat: %.2f TL | Tip: %s\n",bulunan->ilacAdi, bulunan->stokAdedi, bulunan->fiyat, receteAdi);
    secim = sayiAl("1. Stok Ekle\n2. Fiyat Guncelle\nSecim: ");

    if (secim == 1) {
        int yeniStok;
        yeniStok = sayiAl("Kac adet eklendi : ");
        bulunan->stokAdedi += yeniStok;
        printf(" Stok guncellendi. Yeni Stok: %d\n", bulunan->stokAdedi);
        heapGuncelle(kok);

    } else if (secim == 2) {
        float yeniFiyat;

        printf("Yeni Satis Fiyati : ");
        while (scanf("%f", &yeniFiyat) != 1) {
            printf(" Hatali giris.\n Lutfen gecerli bir fiyat giriniz : ");
            while(getchar() != '\n');
        }
        bulunan->fiyat = yeniFiyat;
        printf(" Fiyat guncellendi.\n");
    } else {
        printf("Gecersiz secim.\n");
    }
}

//İsmin içinde geçen kelimeye göre arama yapar
void ilacArama(Ilac* kok, const char* arananAd, int* bulunduMu) {
    if (kok != NULL) {
        ilacArama(kok->sol, arananAd, bulunduMu);

        if (strstr(kok->ilacAdi, arananAd) != NULL) {
            char tip[15];
            if (kok->receteTipi == 1) {
                strcpy(tip, "YESIL");
            }
            else if (kok->receteTipi == 2) {
                strcpy(tip, "KIRMIZI");
            }
            else strcpy(tip, "NORMAL");

            printf(" Ilac: %-15s | Barkod: %-6d | Stok: %-3d | Fiyat: %6.2f TL | [%s]\n",
                   kok->ilacAdi, kok->barkodNo, kok->stokAdedi, kok->fiyat, tip);
            *bulunduMu = 1;
        }
        ilacArama(kok->sag, arananAd, bulunduMu);
    }
}

//İlaç satar ve satışı Stack yapısına ekler
void ilacSat(Ilac* kok, int barkod) {
    Ilac* satilacak = ilacAra(kok,barkod);

    if (satilacak == NULL) {
        printf("\n Barkod bulunamadi.\n");
        return;
    }

    int adet = 1;

    if (satilacak->receteTipi == 0) {
        adet = sayiAl("Kac adet almak istiyorsunuz?: ");
        if (adet <= 0) {
            printf("Gecersiz adet.\n");
            return;
        }
    }

    if (satilacak->receteTipi > 0) {
        int cevap;
        printf("\nBu ilac icin recete gerekiyor.\n");
        printf("Recete mevcut mu?\n");
        cevap = sayiAl("1- Evet\n2- Hayir\nSecim: ");

        if (cevap != 1) {
            printf("Satis iptal edildi.\n");
            return;
        }
        adet = 1;
    }

    if (satilacak->stokAdedi < adet) {
        printf("Yetersiz stok!\n");
        return;
    }

    satilacak->stokAdedi -= adet;
    heapGuncelle(kok);

    float toplamTutar = adet * satilacak->fiyat;
    gunlukCiro += toplamTutar;
    toplamSatisAdedi += adet;

    printf("\n  SATIS BASARILI: %s | %d adet | Tutar: %.2f TL\n",satilacak->ilacAdi, adet, toplamTutar);

    if (satilacak->stokAdedi < KRITIK_STOK_ESIGI) {
        printf("UYARI: %s icin stok kritik seviyede.%d adet kaldi.\n", satilacak->ilacAdi, satilacak->stokAdedi);
    }

    Satis* yeniSatis = (Satis*)malloc(sizeof(Satis));
    if (yeniSatis == NULL) {
        printf("Bellek ayrilamadi!\n");
        return;
    }

    //İade geçmişini kaydetme
    yeniSatis->barkodNo = satilacak->barkodNo;
    strcpy(yeniSatis->ilacAdi, satilacak->ilacAdi);
    yeniSatis->adet = adet;
    yeniSatis->alt = sonSatislar;
    sonSatislar = yeniSatis;
}

// Stack yapısını kullanarak en son yapılan işlemi geri alır(Pop)
void satisiIptalEt(Ilac* kok) {
    if (sonSatislar == NULL) {
        printf("\nIptal edilecek bir islem yok.\n");
        return;
    }

    Satis* iptal = sonSatislar;
    sonSatislar = sonSatislar->alt;

    Ilac* iadeIlac = ilacAra(kok, iptal->barkodNo);
    if (iadeIlac != NULL) {
        iadeIlac->stokAdedi += iptal->adet;
        gunlukCiro -= iptal->adet * iadeIlac->fiyat;
        toplamSatisAdedi -= iptal->adet;

        printf("\n  Iade Tamamlandi: %s | %d adet geri alindi.\n",iptal->ilacAdi, iptal->adet);
        heapGuncelle(kok);
    }
    free(iptal);
}

void gunSonuRaporu(Ilac* kok) {
    printf("\n       GUN SONU RAPORU    \n");

    if (sonSatislar == NULL) {
        printf("Bugun hic satis yapilmadi.\n");
    } else {
        printf("  Bugun Yapilan Satislar:  \n");
        Satis* gecici = sonSatislar;

        while (gecici != NULL) {
            Ilac* satilanIlac = ilacAra(kok, gecici->barkodNo);
            float islemTutari = 0.0;

            if (satilanIlac != NULL) {
                islemTutari = gecici->adet * satilanIlac->fiyat;
            }

            printf(" %s | %2d Adet | Islem Tutari: %.2f TL\n", gecici->ilacAdi, gecici->adet, islemTutari);
            gecici = gecici->alt;
        }
    }

    printf("\nToplam Satilan Urun Adedi : %d\n", toplamSatisAdedi);
    printf("Gunluk Ciro        : %.2f TL\n", gunlukCiro);
}

// Ağacı küçükten büyüğe sıralı yazar
void envanterListele(Ilac* kok) {
    if (kok != NULL) {
        envanterListele(kok->sol);
        char tip[15];
        if (kok->receteTipi == 1) strcpy(tip, "YESIL");
        else if (kok->receteTipi == 2) strcpy(tip, "KIRMIZI");
        else strcpy(tip, "NORMAL");

        printf("Barkod: %-6d | Ilac: %-18s | Stok: %-4d | Fiyat: %6.2f TL | [%s]\n",
               kok->barkodNo, kok->ilacAdi, kok->stokAdedi, kok->fiyat, tip);
        envanterListele(kok->sag);
    }
}

//İkili Arama Ağaçları ile Dizilerin performans kıyaslamasını yapar
void performans() {
    printf("\n    PERFORMANS ANALIZI (BST vs DIZI) \n");

    int veriSetleri[3] = {1000, 10000, 50000};
    srand(time(NULL));

    for (int v = 0; v < 3; v++) {
        int veriSayisi = veriSetleri[v];
        int tekrar = 100000;

        printf("\nVeri Sayisi %d icin karsilastirma :\n", veriSayisi);

        int* dizi = (int*)malloc(veriSayisi * sizeof(int));
        Ilac* testAgac = NULL;


      //Rastgele veri oluşturma
        for (int i = 0; i < veriSayisi; i++) {
            int rastgele = rand() % 1000000;
            dizi[i] = rastgele;
            testAgac = ilacEkle(testAgac, rastgele, "Test", 1, 1.0, 0);
        }

        int aranan = dizi[veriSayisi / 2];
        clock_t basla, bitir;

        //Ağaçta bütün ağacı arama süresi ölçümü
        basla = clock();
        for (int i = 0; i < tekrar; i++) {
            ilacAra(testAgac, aranan);
        }
        bitir = clock();
        double bstSure = (double)(bitir - basla) *1000 / CLOCKS_PER_SEC;


        //Dizide arama süresi ölçümü
        basla = clock();
        for (int k = 0; k < tekrar; k++) {
            for (int i = 0; i < veriSayisi; i++) {
                if (dizi[i] == aranan)
                    break;
            }
        }
        bitir = clock();
        double diziSure = (double)(bitir - basla) *1000/ CLOCKS_PER_SEC;

        printf("BST Sure  : %.4f milisaniye\n", bstSure);
        printf("Dizi Sure : %.4f milisaniye\n", diziSure);

        if (bstSure > 0)
            printf("Hiz Orani : %.2f kat\n", diziSure / bstSure);

        free(dizi);
        agaciTemizle(testAgac);
    }
}

int main() {
    Ilac* root = NULL;
    kritikHeap.capacity = MAX_HEAP_BOYUT;
    int secim, barkod, stok, rTip;
    float fiyat;
    char ad[50];


    root = dosyadanOku(root); // Önceki kayıtları okuyup ağacı doldurur

    do {
        printf("\n\n        ECZANE STOK VE KASA YONETIM SISTEMI      \n");
        printf("---------------------------------------------------\n");
        printf("1. Yeni Ilac Kaydi  \n");
        printf("2. Ilac Kaydini Sil \n");
        printf("3. Stok/Fiyat Bilgisi Guncelle\n");
        printf("4. Ilac Ismine Gore Arama Yap \n");
        printf("5. Satis Yap \n");
        printf("6. Son Satisi Iptal Et (Iade Al)\n");
        printf("7. Ilac Listesi \n");
        printf("8. Kritik Stok Uyarilari\n");
        printf("9. Gun Sonu Raporu\n");
        printf("10. Sistem Performans Testi\n");
        printf("11. Cikis\n");
        printf("Seciminiz: ");

        if (scanf("%d", &secim) != 1) {
            while (getchar() != '\n');    // Yanlışlıkla harf girilirse döngüye girmeyi önler
            printf("Lutfen sadece rakam giriniz. \n");
            continue;
        }

        switch(secim) {
            case 1:
                barkod = sayiAl("Yeni Barkod: ");
                if(ilacAra(root, barkod)) {
                    printf(" Bu barkod zaten kayitli.\n");
                } else {
                        printf("Ilac Adi: ");
                        scanf(" %[^\n]", ad);

                    stok = sayiAl("Stok: ");
                    printf("Fiyat: ");
                    while (scanf("%f", &fiyat) != 1) {
                        printf("Hatali giris.Lutfen gecerli bir fiyat giriniz: ");
                        while(getchar() != '\n');
                    }
                    do {
                        rTip = sayiAl("Recete Tipi (0:Normal, 1:Yesil, 2:Kirmizi): ");
                        if (rTip < 0 || rTip > 2) {
                            printf("Lutfen sadece 0, 1 veya 2 rakamlarindan birini giriniz.\n");
                        }
                    } while (rTip < 0 || rTip > 2);

                    root = ilacEkle(root, barkod, ad, stok, fiyat, rTip);
                    printf("Ilac basariyla eklendi.\n");
                }
                break;
            case 2:
                barkod = sayiAl("Silinecek Barkod: ");
                if(ilacAra(root, barkod)) {
                    root = ilacSil(root, barkod);
                    printf(" Kayit silindi.\n");
                } else
                    printf("Kayit bulunamadi.\n");
                break;
            case 3:
                ilacGuncelle(root);
                break;
            case 4: {
                char arananIsim[50];
                int bulunduMu = 0;
                printf("Aradiginiz ilacin ismini veya bir kismini girin: ");
                scanf(" %[^\n]", arananIsim);

                printf("\n '%s' Icin Arama Sonuclari :\n", arananIsim);
                ilacArama(root, arananIsim, &bulunduMu);

                if (bulunduMu == 0) {
                    printf("Sistemde bu ismi iceren bir ilac bulunamadi.\n");
                }
                break;
            }
            case 5:
                barkod = sayiAl("Barkodu Okutun: ");
                ilacSat(root, barkod);
                break;
            case 6:
                satisiIptalEt(root);
                break;
            case 7:
                printf("\n--- Mevcut Envanter Listesi ---\n");
                envanterListele(root);
                break;
            case 8:
                heapGuncelle(root);
                kritikStoklariGoster(&kritikHeap);
                break;
            case 9:
                gunSonuRaporu(root);
                break;
            case 10:
                performans();
                break;
            case 11:
                printf("\nVeriler 'eczane.txt' dosyasina kaydediliyor.\n");
                FILE* dosya = fopen("eczane.txt", "w");
                if (dosya == NULL) {
                    printf("Dosya acilamadi!\n");
                    break;
                }
                dosyayaYaz(root, dosya);   // Çıkış yaparken verileri dosyaya kaydeder
                fclose(dosya);
                break;
            default:
                printf("Gecersiz secim.\n");
        }
    } while (secim != 11);

       //Bellek Temizliği
        agaciTemizle(root);

    while (sonSatislar != NULL) {
        Satis* silinecek = sonSatislar;
        sonSatislar = sonSatislar -> alt;
        free(silinecek);
    }
    return 0;
}