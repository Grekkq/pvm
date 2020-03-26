// Imie: Grzegorz Nazwisko: Zagata Grupa: TI1 Sekcja: 1
// Tresc zadania: szukanie minimum i maksimum macierzy
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "pvm3.h"
#include <limits>

// rozmiar macierzy
const int matrixSize = 10;

void rodzic() {
	struct pvmhostinfo *hostp;
	// wskaźnik do tid
	int result, check, i, ilhost, ilarch, *tid, tidmaster;

	// inicjalizacja macierzy i wypełnienie losowymi liczbami
	int matrix[matrixSize][matrixSize];
	srand(time(NULL));
	for (int r=0;r<matrixSize;r++)
		for (int c=0;c<matrixSize;c++)
			matrix[r][c] = rand() % 89 + 10;
	// wypisanie macierzy
	for (int r=0;r<matrixSize;r++) {
		for (int c=0;c<matrixSize;c++) {
			printf(" %d ", matrix[r][c]);
		}
		printf("\n");
	}
	// macierz do przechowywania wynikow
	int matrixMax = 0, matrixMin = std::numeric_limits<int>::max();
	// zmienne do przechowywania odebranych informacji
	int upMax = 0, upMin = 0;
	// obsługa wysyłania i odbierania danych
	int r = 0, rp, cos, ps;
	int matrixRow[matrixSize];

	if(tidmaster=pvm_mytid() <0) {
		pvm_perror("enroll");
		exit(1);
	}

	pvm_config(&ilhost, &ilarch, &hostp);
	// alokacja pamięci na tablice tidow
	tid = (int*)calloc(ilhost, sizeof(int));
	// inicjalizacja hostow i wyslanie pierwszych danych
	for(i=0; i<ilhost; i++) {
		check=pvm_spawn("/home/pvm/pvm3/lab/example", 0, PvmTaskHost, hostp[i].hi_name, 1, &tid[i]);
		if(!check)
			printf("Błąd powołania potomka do życia na %s\n", hostp[i].hi_name);

		// wysłanie wiersza do zainicjalizowanego hosta
		pvm_initsend(PvmDataDefault);
		for (int c = 0; c < matrixSize; c++) {
			matrixRow[c] = matrix[r][c];
		}
		pvm_pkint(matrixRow, matrixSize, 1);
		pvm_send(tid[i], 200);
		printf("Wyslano wiersz: %d\n", r);
		r++;
	}

	// odbieranie i wysylanie kolejnych danych
	while(r < matrixSize) {
		// odebranie policzonego wiersza
		rp = pvm_recv(-1, 200);
		pvm_upkint(&upMax, 1, 1);
		pvm_upkint(&upMin, 1, 1);
		printf("Odebrano wiersz: %d Max, Min: %d, %d\n", r, upMax, upMin);
		// sprawdzenie czy nowe dane są więskze/mniejsze od obecnego max/min
		if (upMax > matrixMax)
			matrixMax = upMax;
		if (upMin < matrixMin)
			matrixMin = upMin;
		// zapisanie do ps skąd przyszedł pakiet
		pvm_bufinfo(rp, &cos, &cos, &ps);
		//koniec odbierania teraz wysylanie kolejnego wiersza
		pvm_initsend(PvmDataDefault);
		for (int c = 0; c < matrixSize; c++) {
            matrixRow[c] = matrix[r][c];
        }
        pvm_pkint(matrixRow, matrixSize, 1);
        pvm_pkint(&r, 1, 1);
        pvm_send(ps, 200);
		printf("Wyslano wiersz: %d\n", r);
		r++;
	}

	// odbieranie ostatnich danych
	for (int x = 0; x < ilhost; x++) {
		pvm_recv(-1, 200);
		pvm_upkint(&upMax, 1, 1);
		pvm_upkint(&upMin, 1, 1);
		printf("Odebrano wiersz: %d Max, Min: %d, %d\n", r, upMax, upMin);
		// sprawdzenie czy nowe dane są więskze/mniejsze od obecnego max/min
		if (upMax > matrixMax)
			matrixMax = upMax;
		if (upMin < matrixMin)
			matrixMin = upMin;
	}

	printf("Minimum macierzy: %d\nMaksimum macierzy: %d\n", matrixMin, matrixMax);

	pvm_exit();
}

void potomek() {
	int ptid = pvm_parent();
	// inicjalizowanie zmiennych do przechowywania danych
	int row[matrixSize];
	int max = 0, min = std::numeric_limits<int>::max();
	while(1) {
		max = 0, min = std::numeric_limits<int>::max();
		// odebranie i odpakowanie pakietu
		pvm_recv(-1, 200);
		pvm_upkint(row, matrixSize, 1);
		// znalezienie minimum i maksimum
		for (int i = 0; i < matrixSize; i++) {
			if (row[i] > max)
				max = row[i];
			if (row[i] < min)
				min = row[i];
		}
		// odeslanie minimum i maksimum
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&max, 1, 1);
		pvm_pkint(&min, 1, 1);
		pvm_send(ptid, 200);
	}
}

main() {
	if(pvm_parent() == PvmNoParent)
		rodzic();
	else
		potomek();
}
