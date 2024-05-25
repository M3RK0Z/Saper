#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h> //muzyka
#include <allegro5/allegro_acodec.h>

#define LATWY_ROZMIAR 10
#define SREDNI_ROZMIAR 14
#define TRUDNY_ROZMIAR 18
#define KOMORKA_ROZMIAR 30

// Struktura reprezentujaca pojedyncza komorke
typedef struct
{
	int czyMina;
	int czyOdkryta;
	int sasiadujaceMiny;
	int czyFlaga;
} KOMORKA;

ALLEGRO_FONT* czcionka = NULL; // Zmienna globalna przechowujaca wczytana czcionke
ALLEGRO_BITMAP* obraz[4] = { NULL }; // Tablica bitmap przechowuj¹ca obrazy
time_t start_time; // Czas rozpoczêcia gry
time_t end_time; // Czas zakoñczenia gry

// Funkcja inicjalizujaca plansze, ustawiajaca wszystkie komorki na poczatkowe wartosci
void inicjalizacja_Planszy(KOMORKA macierz[][TRUDNY_ROZMIAR], int rozmiar)
{
	for (int i = 0; i < rozmiar; i++)
	{
		for (int j = 0; j < rozmiar; j++)
		{
			macierz[i][j].czyMina = 0;
			macierz[i][j].czyOdkryta = 0;
			macierz[i][j].sasiadujaceMiny = 0;
			macierz[i][j].czyFlaga = 0;
		}
	}
}

// Funkcja ustawiaj¹ca miny na planszy
void ustaw_Miny(KOMORKA macierz[][TRUDNY_ROZMIAR], int rozmiar, int liczbaMin)
{
	int postawioneMiny = 0;
	while (postawioneMiny < liczbaMin)
	{
		int x = rand() % rozmiar;
		int y = rand() % rozmiar;
		if (macierz[x][y].czyMina == 0)
		{
			macierz[x][y].czyMina = 1;
			postawioneMiny++;
		}
	}
}

// Funkcja obliczaj¹ca liczbê min s¹siaduj¹cych z ka¿d¹ komórk¹
void oblicz_Sasiadujace(KOMORKA macierz[][TRUDNY_ROZMIAR], int rozmiar)
{
	// Iteracja po wszystkich komorkach planszy
	for (int i = 0; i < rozmiar; i++)
	{
		for (int j = 0; j < rozmiar; j++)
		{
			// Sprawdzenie czy aktualna komorka nie jest mina
			if (macierz[i][j].czyMina == 0)
			{
				// Sprawdzenie czy aktualna komorka nie jest mina
				for (int dx = -1; dx <= 1; dx++)
				{
					for (int dy = -1; dy <= 1; dy++)
					{
						// Obliczenie wspolrzednych otaczajacej komorki
						int nx = i + dx;
						int ny = j + dy;
						// Sprawdzenie czy otaczajaca komorka miesci sie w granicach planszy
						if (nx >= 0 && nx < rozmiar && ny >= 0 && ny < rozmiar && macierz[nx][ny].czyMina)
						{
							macierz[i][j].sasiadujaceMiny++;
						}
					}
				}
			}
		}
	}
}

// Funkcja rysuj¹ca pojedyncz¹ komórkê na planszy
void rysuj_Komorke(KOMORKA komorka, int x, int y)
{
	if (komorka.czyOdkryta == 1)
	{
		// Nieodkryta komórka
		al_draw_filled_rectangle(x, y, x + KOMORKA_ROZMIAR, y + KOMORKA_ROZMIAR, al_map_rgb(192, 192, 192));

		//Rysowanie flagi
		if (komorka.czyFlaga == 1)
		{
			al_draw_bitmap(obraz[2], x + KOMORKA_ROZMIAR / 2 - 15, y + KOMORKA_ROZMIAR / 2 - 15, ALLEGRO_ALIGN_CENTER);
		}
	}
	else
	{
		// Odkryta komórka
		al_draw_filled_rectangle(x, y, x + KOMORKA_ROZMIAR, y + KOMORKA_ROZMIAR, al_map_rgb(255, 255, 255));

		// Rysowanie miny lub liczby s¹siaduj¹cych min, jeœli komórka nie zawiera miny
		if (komorka.czyMina == 1)
		{
			al_draw_bitmap(obraz[1], x + KOMORKA_ROZMIAR / 2 - 15, y + KOMORKA_ROZMIAR / 2 - 15, ALLEGRO_ALIGN_CENTER);
		}
		else {
			if (komorka.sasiadujaceMiny > 0) {
				al_draw_textf(czcionka, al_map_rgb(0, 0, 0), x + KOMORKA_ROZMIAR / 2, y + KOMORKA_ROZMIAR / 2, ALLEGRO_ALIGN_CENTER, "%d", komorka.sasiadujaceMiny);
			}
		}
	}
	// Rysowanie ramki wokó³ komórki
	al_draw_rectangle(x, y, x + KOMORKA_ROZMIAR, y + KOMORKA_ROZMIAR, al_map_rgb(0, 0, 0), 1);
}

// Funkcja rysuj¹ca ca³¹ planszê
void rysuj_Plansze(KOMORKA macierz[][TRUDNY_ROZMIAR], int rozmiar)
{
	for (int i = 0; i < rozmiar; i++)
	{
		for (int j = 0; j < rozmiar; j++)
		{
			// Rysowanie pojedynczej komórki
			rysuj_Komorke(macierz[i][j], i * KOMORKA_ROZMIAR, j * KOMORKA_ROZMIAR);
		}
	}
}

// funkcja odkrywajaca komorke na planszy
void Odkryj_Komorke(KOMORKA macierz[][TRUDNY_ROZMIAR], int rozmiar, int x, int y, int* pozostaleKomorki)
{
	if (x < 0 || x >= rozmiar || y < 0 || y >= rozmiar || macierz[x][y].czyOdkryta)
	{
		return;
	}
	macierz[x][y].czyOdkryta = 1;
	if (macierz[x][y].sasiadujaceMiny == 0 && macierz[x][y].czyMina == 0)
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++)
			{
				// Rekurencyjne odkrywanie s¹siedniej komórki, jeœli s¹siaduj¹ca komórka nie zawiera miny
				Odkryj_Komorke(macierz, rozmiar, x + dx, y + dy, pozostaleKomorki);
			}
		}
	}
	(*pozostaleKomorki)--;
}

main()
{
	srand(time(NULL));

	int wydarzenie = 0;
	int liczbaMin = 0;
	int rozmiar = 0;

	ALLEGRO_DISPLAY* display = NULL; // WskaŸnik na obiekt reprezentuj¹cy wyœwietlacz
	ALLEGRO_EVENT_QUEUE* event_queue = NULL; // WskaŸnik na kolejkê zdarzeñ
	ALLEGRO_MOUSE_CURSOR* cursor = NULL; //WskaŸnik na kursor
	ALLEGRO_SAMPLE* sample = NULL;
	ALLEGRO_SAMPLE_INSTANCE* sampleInstance = NULL;

	// Inicjalizacja biblioteki Allegro
	al_init();
	al_init_primitives_addon();
	al_install_mouse();
	al_install_keyboard();
	al_init_acodec_addon();//inicjalizacja addon'ow obslugujacych muzyke
	al_install_audio();
	al_init_image_addon();

	// Tworzenie okna wyœwietlacza
	display = al_create_display(TRUDNY_ROZMIAR * KOMORKA_ROZMIAR, TRUDNY_ROZMIAR * KOMORKA_ROZMIAR);
	if (!display)
	{
		printf("Nie udalo sie utworzyc ekranu!\n");
		return -1;
	}

	// Wczytywanie obrazu reprezentuj¹cego logo
	obraz[0] = al_load_bitmap("saper.png");
	if (!obraz[0])
	{
		printf("Nie udalo sie wczytac obrazu!\n");
		al_destroy_display(display);
		return -1;
	}

	obraz[1] = al_load_bitmap("bomba.png");
	if (!obraz[1])
	{
		printf("Nie udalo sie wczytac obrazu!\n");
		al_destroy_display(display);
		return -1;
	}

	obraz[2] = al_load_bitmap("flaga.png");
	if (!obraz[2])
	{
		printf("Nie udalo sie wczytac obrazu!\n");
		al_destroy_display(display);
		return -1;
	}

	obraz[3] = al_load_bitmap("kursor.png");
	if (!obraz[2])
	{
		printf("Nie udalo sie wczytac obrazu!\n");
		al_destroy_display(display);
		return -1;
	}

	// Utworzenie kursora z bitmapy
	cursor = al_create_mouse_cursor(obraz[3], 0, 0);
	if (!cursor) {
		fprintf(stderr, "Failed to create mouse cursor.\n");
		al_destroy_bitmap(obraz[3]);
		al_destroy_display(display);
		return -1;
	}

	// Tworzenie kolejki zdarzeñ
	event_queue = al_create_event_queue();
	if (!event_queue)
	{
		printf("Nie udalo sie stworzyc event_queue!\n");
		al_destroy_display(display);
		return -1;
	}

	//obsluga plikow audio
	al_reserve_samples(1);
	sample = al_load_sample("wav/mario.wav");
	sampleInstance = al_create_sample_instance(sample);
	al_attach_sample_instance_to_mixer(sampleInstance, al_get_default_mixer());
	al_set_sample_instance_gain(sampleInstance, 0.2);

	// Rejestrowanie Ÿróde³ zdarzeñ dla okna wyœwietlacza i myszy
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	// Ustawienie kursora na ekranie
	al_set_mouse_cursor(display, cursor);

	czcionka = al_create_builtin_font(); // U¿ycie wbudowanej czcionki
	if (!czcionka)
	{
		printf("Nie udalo sie utworzyc czcionki!\n");
		al_destroy_display(display);
		return -1;
	}

	// Pêtla g³ówna programu
	while (wydarzenie == 0)
	{
		// Rysowanie menu wyboru poziomu trudnoœci
		al_clear_to_color(al_map_rgb(50, 50, 50));
		al_draw_bitmap(obraz[0], al_get_display_width(display) / 2 - al_get_bitmap_width(obraz[0]) / 2, al_get_display_height(display) / 2 - al_get_bitmap_height(obraz[0]) / 2 - 225, 0);
		al_draw_text(czcionka, al_map_rgb(128, 255, 255), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 185, ALLEGRO_ALIGN_CENTER, "Wybierz poziom trudnosci:");
		al_draw_text(czcionka, al_map_rgb(0, 255, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 160, ALLEGRO_ALIGN_CENTER, "1. Latwy");
		al_draw_text(czcionka, al_map_rgb(255, 255, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 130, ALLEGRO_ALIGN_CENTER, "2. Sredni");
		al_draw_text(czcionka, al_map_rgb(255, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 100, ALLEGRO_ALIGN_CENTER, "3. Trudny");
		al_draw_text(czcionka, al_map_rgb(255, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 40, ALLEGRO_ALIGN_CENTER, "Najlepsze wyniki");
		al_draw_text(czcionka, al_map_rgb(180, 20, 100), al_get_display_width(display), al_get_display_height(display) / 2 + 260, ALLEGRO_ALIGN_RIGHT, "Wyjscie");
		al_flip_display();

		ALLEGRO_EVENT event;
		al_wait_for_event(event_queue, &event);

		al_play_sample_instance(sampleInstance);//odpalenie muzyki w tle

		// Obs³uga zamkniêcia okna
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			break;
		}

		// Obs³uga klikniêæ myszy
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
		{
			int x = event.mouse.x;
			int y = event.mouse.y;

			// Obs³uga wyboru poziomu trudnoœci
			if (x >= al_get_display_width(display) / 2 - 30 && x <= al_get_display_width(display) / 2 + 30)
			{
				if (y >= al_get_display_height(display) / 2 - 170 && y <= al_get_display_height(display) / 2 - 150)
				{
					rozmiar = LATWY_ROZMIAR;
					liczbaMin = LATWY_ROZMIAR * LATWY_ROZMIAR * 0.1;
					wydarzenie = 1;
				}

				if (y >= al_get_display_height(display) / 2 - 140 && y <= al_get_display_height(display) / 2 - 120)
				{
					rozmiar = SREDNI_ROZMIAR;
					liczbaMin = SREDNI_ROZMIAR * SREDNI_ROZMIAR * 0.1;
					wydarzenie = 1;
				}

				if (y >= al_get_display_height(display) / 2 - 110 && y <= al_get_display_height(display) / 2 - 90)
				{
					rozmiar = TRUDNY_ROZMIAR;
					liczbaMin = TRUDNY_ROZMIAR * TRUDNY_ROZMIAR * 0.1;
					wydarzenie = 1;
				}
			}

			//Obsuga przycisku tabeli wynikow
			if (x >= al_get_display_width(display) / 2 - 70 && x <= al_get_display_width(display) / 2 + 55)
			{
				if (y >= al_get_display_height(display) / 2 - 45 && y <= al_get_display_height(display) / 2 - 35)
				{
					wydarzenie = 4;
				}
			}
			// Obs³uga przycisku wyjœcia
			if (x >= al_get_display_width(display) - 60 && x <= al_get_display_width(display))
			{
				if (y >= al_get_display_height(display) / 2 + 250 && y <= al_get_display_height(display) / 2 + 270)
				{
					break;
				}
			}
		}

		int czas_gry;
		bool czas_zapisany = false;

		// Obs³uga wydarzenia wybrania poziomu trudnoœci
		if (wydarzenie == 1)
		{
			KOMORKA macierz[TRUDNY_ROZMIAR][TRUDNY_ROZMIAR]; // Deklaracja macierzy reprezentuj¹cej planszê

			inicjalizacja_Planszy(macierz, rozmiar);
			ustaw_Miny(macierz, rozmiar, liczbaMin);
			oblicz_Sasiadujace(macierz, rozmiar);

			int pozostaleKomorki = rozmiar * rozmiar - liczbaMin;

			// Pocz¹tek pomiaru czasu
			int start = al_get_time();


			// Pêtla obs³uguj¹ca g³ówn¹ mechanikê gry
			while (wydarzenie == 1)
			{
				al_clear_to_color(al_map_rgb(192, 192, 192));
				rysuj_Plansze(macierz, rozmiar);
				al_flip_display();

				ALLEGRO_EVENT event;
				al_wait_for_event(event_queue, &event);

				// Obs³uga zamkniêcia okna
				if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
				{
					break;
				}


				// Obs³uga klikniêæ myszy
				if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
				{
					int wiersz = event.mouse.x / KOMORKA_ROZMIAR;
					int kolumna = event.mouse.y / KOMORKA_ROZMIAR;

					if (event.mouse.button & 2) // Sprawdzanie, czy naciœniêty zosta³ prawy przycisk myszy
					{
						if (wiersz >= 0 && wiersz < rozmiar && kolumna >= 0 && kolumna < rozmiar)
						{
							// Stawianie flagi tylko dla nieodkrytych komórek
							if (macierz[wiersz][kolumna].czyOdkryta == 0)
							{
								macierz[wiersz][kolumna].czyFlaga = !macierz[wiersz][kolumna].czyFlaga; // Obracanie flagi
							}
						}
					}
					else
					{
						if (wiersz >= 0 && wiersz < rozmiar && kolumna >= 0 && kolumna < rozmiar)
						{
							if (macierz[wiersz][kolumna].czyMina == 1 && macierz[wiersz][kolumna].czyFlaga == 0)
							{
								wydarzenie = 3;
								int end = al_get_time();
							}
							else
							{

								if (macierz[wiersz][kolumna].czyOdkryta == 0 && macierz[wiersz][kolumna].czyFlaga == 0)
								{
									// Odkrycie komórki i aktualizacja pozosta³ych komórek
									Odkryj_Komorke(macierz, rozmiar, wiersz, kolumna, &pozostaleKomorki);
									printf("%d ", pozostaleKomorki); // Wyœwietlenie pozosta³ych komórek (dla celów testowych)
								}
								// Sprawdzenie warunku wygranej
								if (pozostaleKomorki == 0)
								{
									// Koniec pomiaru czasu
									int end = al_get_time();
									czas_gry = end - start; // Obliczenie czasu gry

									wydarzenie = 2;
								}
							}
						}
					}
				}
			}
		}

		//Obsluga wydarzenia wygranej
		while (wydarzenie == 2)
		{
			al_clear_to_color(al_map_rgb(0, 192, 0));
			al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 30, ALLEGRO_ALIGN_CENTER, "Gratulacje! Wygrales!");
			al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2, ALLEGRO_ALIGN_CENTER, "Wpisz swoje imie:");
			al_flip_display();

			bool imie_wpisane = false;
			int index = 0;
			char nazwa_gracza[100];

			while (!imie_wpisane)
			{
				ALLEGRO_EVENT event;
				al_wait_for_event(event_queue, &event);

				if (event.type == ALLEGRO_EVENT_KEY_CHAR)
				{
					char znak = event.keyboard.unichar;

					// Sprawdzenie, czy wpisany znak jest liter¹ lub cyfr¹
					if ((znak >= 'a' && znak <= 'z') || (znak >= 'A' && znak <= 'Z') || (znak >= '0' && znak <= '9'))
					{
						if (index < sizeof(nazwa_gracza) - 1)
						{
							nazwa_gracza[index++] = znak;
							nazwa_gracza[index] = '\0';
						}
					}
					else if (znak == '\b' && index > 0) // Obs³uga backspace
					{
						nazwa_gracza[--index] = '\0';
					}
					else if (znak == '\r') // Obs³uga enter
					{
						imie_wpisane = true;
					}

					// Aktualizacja wyœwietlania tekstu
					al_clear_to_color(al_map_rgb(0, 192, 0));
					al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 30, ALLEGRO_ALIGN_CENTER, "Gratulacje! Wygrales!");
					al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2, ALLEGRO_ALIGN_CENTER, "Wpisz swoje imie:");
					al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 + 30, ALLEGRO_ALIGN_CENTER, nazwa_gracza);
					al_flip_display();
				}
			}

			if (!czas_zapisany)
			{
				FILE* plik = NULL;
				char* nazwa_pliku = NULL;

				// Okreœlenie nazwy pliku w zale¿noœci od poziomu trudnoœci
				if (rozmiar == LATWY_ROZMIAR)
				{
					nazwa_pliku = "czas_gry_latwy.txt";
				}
				else if (rozmiar == SREDNI_ROZMIAR)
				{
					nazwa_pliku = "czas_gry_sredni.txt";
				}
				else if (rozmiar == TRUDNY_ROZMIAR)
				{
					nazwa_pliku = "czas_gry_trudny.txt";
				}

				// Otwarcie pliku do odczytu
				fopen_s(&plik, nazwa_pliku, "r");
				if (plik != NULL)
				{
					int najlepszy_czas = 0;

					// Odczytanie najlepszego czasu i nazwy gracza z pliku
					fscanf_s(plik, "%d", &najlepszy_czas);
					fclose(plik);

					// Porównanie z bie¿¹cym czasem gry
					if (czas_gry < najlepszy_czas || najlepszy_czas == 0)
					{
						// Zapis nowego najlepszego czasu i nazwy gracza
						fopen_s(&plik, nazwa_pliku, "w");
						if (plik != NULL)
						{
							fprintf(plik, "%d %s", czas_gry, nazwa_gracza);
							fclose(plik);
						}
					}
				}
				else
				{
					// Utworzenie nowego pliku i zapis danych
					fopen_s(&plik, nazwa_pliku, "w");
					if (plik != NULL)
					{
						fprintf(plik, "%d %s", czas_gry, nazwa_gracza);
						fclose(plik);
					}
				}
				czas_zapisany = true;
			}

			al_clear_to_color(al_map_rgb(192, 192, 20));
			al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 30, ALLEGRO_ALIGN_CENTER, "Twoj czas:");
			al_draw_textf(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2, ALLEGRO_ALIGN_CENTER, "%d sekund", czas_gry);
			al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 + 30, ALLEGRO_ALIGN_CENTER, "Wcisnij klawisz aby kontynuowac");
			al_flip_display();

			while (true)
			{
				ALLEGRO_EVENT event;
				al_wait_for_event(event_queue, &event);
				if (event.type == ALLEGRO_EVENT_KEY_DOWN)
				{
					wydarzenie = 0;
					break;
				}
			}
		}

		//Obsluga wydarzenia przegranej
		while (wydarzenie == 3)
		{
			al_clear_to_color(al_map_rgb(192, 0, 0));
			al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 2 - 30, ALLEGRO_ALIGN_CENTER, "PRZEGRANA");
			al_flip_display();

			ALLEGRO_EVENT event;
			al_wait_for_event(event_queue, &event);

			// Obs³uga klikniêæ myszy
			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
			{
				if (event.mouse.button)
				{
					wydarzenie = 0;
				}
			}
		}

		//Obsluga tabeli wynikow
		while (wydarzenie == 4)
		{
			al_clear_to_color(al_map_rgb(20, 192, 192));

			// Wczytywanie i wyœwietlanie wyników z plików
			char* filenames[] = { "czas_gry_latwy.txt", "czas_gry_sredni.txt", "czas_gry_trudny.txt" };
			char* poziomy[] = { "Latwy", "Sredni", "Trudny" };
			int y_offset = 20; // pocz¹tkowy przesuniêcie w pionie
			char linia[256];

			for (int i = 0; i < 3; i++) {
				FILE* plik;
				fopen_s(&plik, filenames[i], "r");
				if (plik != NULL) {
					al_draw_textf(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, y_offset, ALLEGRO_ALIGN_CENTER, "Poziom: %s", poziomy[i]);
					y_offset += 30;
					while (fgets(linia, sizeof(linia), plik)) {
						al_draw_text(czcionka, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, y_offset, ALLEGRO_ALIGN_CENTER, linia);
						y_offset += 20;
					}
					fclose(plik);
					y_offset += 20; // dodatkowe przesuniêcie miêdzy poziomami
				}
			}

			al_flip_display();

			ALLEGRO_EVENT event;
			al_wait_for_event(event_queue, &event);

			// Obs³uga klikniêæ myszy
			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
			{
				if (event.mouse.button)
				{
					wydarzenie = 0;
				}
			}
		}
	}

	// zwalnianie
	al_destroy_font(czcionka);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_shutdown_image_addon();
	al_destroy_mouse_cursor(cursor);
	al_destroy_sample(sample);
	al_destroy_sample_instance(sampleInstance);
	al_uninstall_audio();

	return 0;
}