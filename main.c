#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

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
} KOMORKA;

ALLEGRO_FONT* czcionka = NULL; // Zmienna globalna przechowujaca wczytana czcionke

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
	if (komorka.czyOdkryta == 0)
	{
		// Nieodkryta komórka
		al_draw_filled_rectangle(x, y, x + KOMORKA_ROZMIAR, y + KOMORKA_ROZMIAR, al_map_rgb(192, 192, 192));
	}
	else
	{
		// Odkryta komórka
		al_draw_filled_rectangle(x, y, x + KOMORKA_ROZMIAR, y + KOMORKA_ROZMIAR, al_map_rgb(255, 255, 255));
		// Rysowanie miny lub liczby s¹siaduj¹cych min, jeœli komórka nie zawiera miny
		if (komorka.czyMina == 1)
		{
			al_draw_text(czcionka, al_map_rgb(0, 0, 0), x + KOMORKA_ROZMIAR / 2, y + KOMORKA_ROZMIAR / 2, ALLEGRO_ALIGN_CENTER, "*");
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
			// Rysowanie pojedynczej komórkê
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
	ALLEGRO_BITMAP* obraz[1] = { NULL }; // Tablica bitmap przechowuj¹ca obrazy

	// Inicjalizacja biblioteki Allegro
	al_init();
	al_init_primitives_addon();
	al_install_mouse();
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

	// Tworzenie kolejki zdarzeñ
	event_queue = al_create_event_queue();
	if (!event_queue)
	{
		printf("Nie udalo sie stworzyc event_queue!\n");
		al_destroy_display(display);
		return -1;
	}

	// Rejestrowanie Ÿróde³ zdarzeñ dla okna wyœwietlacza i myszy
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_mouse_event_source());

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
		al_draw_text(czcionka, al_map_rgb(180, 20, 100), al_get_display_width(display), al_get_display_height(display) / 2 + 260, ALLEGRO_ALIGN_RIGHT, "Wyjscie");
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

			// Obs³uga przycisku wyjœcia
			if (x >= al_get_display_width(display) - 60 && x <= al_get_display_width(display))
			{
				if (y >= al_get_display_height(display) / 2 + 250 && y <= al_get_display_height(display) / 2 + 270)
				{
					break;
				}
			}
		}

		// Obs³uga wydarzenia wybrania poziomu trudnoœci
		if (wydarzenie == 1)
		{
			KOMORKA macierz[TRUDNY_ROZMIAR][TRUDNY_ROZMIAR]; // Deklaracja macierzy reprezentuj¹cej planszê

			inicjalizacja_Planszy(macierz, rozmiar);
			ustaw_Miny(macierz, rozmiar, liczbaMin);
			oblicz_Sasiadujace(macierz, rozmiar);

			int pozostaleKomorki = rozmiar * rozmiar - liczbaMin;

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

					if (wiersz >= 0 && wiersz < rozmiar && kolumna >= 0 && kolumna < rozmiar)
					{
						if (macierz[wiersz][kolumna].czyMina == 1)
						{
							wydarzenie = 0;
						}
						else
						{
							if (macierz[wiersz][kolumna].czyOdkryta == 0)
							{
								// Odkrycie komórki i aktualizacja pozosta³ych komórek
								Odkryj_Komorke(macierz, rozmiar, wiersz, kolumna, &pozostaleKomorki);
								printf("%d ", pozostaleKomorki); // Wyœwietlenie pozosta³ych komórek (dla celów testowych)
							}
							// Sprawdzenie warunku wygranej
							if (pozostaleKomorki == 0)
							{
								wydarzenie = 0;
							}
						}
					}
				}
			}
		}

	}

	// zwalnianie
	al_destroy_font(czcionka);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_shutdown_image_addon();

	return 0;
}