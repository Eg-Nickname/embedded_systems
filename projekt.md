Rozwiązac problem kosyntezy. Algorytm ewolucyjne i drzewa decyzyjne, algorytm rafinacyjny. Algorytmy ewolucyjny i generacja pokolenia zero (wiele rozwiązań gneerujemy). Pokolenie poczatkowe powstaje zerowe losowo, kolejne pokolenia na podstawie algorytmów genetycznych powstają z poprzedniego. Kontynuowane do warunku stopu, możliwe:

- Wygenerowanie stałej liczby pokoleń (podaje projektant, kto uruchamia)
- Generacja pokoleń do czasu (do kolejnych epsilon pokolen) nie zostanie stworzone lepsze rozwiązanie.

Wczytujemy graf zadań i tworzymy drzewo decyzji. Pierwszy węzeł drzewa sub-optymalne rozwiązanie (najszybsze lub losowe, wybór tylko jednego zasobu i przydzielenie zasobów). Tworzymy strukture w sposób losowy, w każdym węźle oprócz pierwszego istnieje funkcja, która na zmienia przydział części zadań. Każda funkcja ma kilka opcji wyboru. Drzewie pi = alpha _ liczba_proc _ liczba zadań (coś z beta i gama).

Działamy operatorami genetycznymi i sa 4 podstawowe:

- selekcja - dla różnych operatorów może wybierać różne osobniki. Można wybierać przy pomocy:
  - turniejowa - grupa rozwiązań wybierana w pary i lepsze rozwiazanie lepsze przechodzi dalej
  - rankingowa - tworzy liste rankingowa dla każdego pokolenia, prawdopodobieństwo od miejsca w rankingu dla danego genotypu. pi - pi/r gdzie r pozycja w rankingu (r od 0 żeby zawsze wybrać najlepsze rozwiązanie)
  - ruletka - kazdy osobnik w zależności od przystosowania wybrany wycinek koła (od 0 do 360) kryncimy kołem i wybieramy osobnika na któreog padło. Kryncimy odpowiednia liczbe razy.

  Tylko jeden operator selekcji. Wybiera osobniki dla pozostałych operatorów

- klonowanie - kopiuje phi osobników do nowego pokolenia bez jakiejkolwiek zmiany. phi = delta \* pi, gdzie delta in [0, 1]. Delta dobierana ekpserymentalnie zazwyczaj 0.1 - 0.2
- mutacja - dokonuje lokalnych zmian, przy pomocy selekcji wybiera omega rozwiazań gdzie omega = beta \* pi, beta in [0, 1]. Inny parametr niż Delta. Wybiera dowolny wezeł w genotypie (drzewie decyzyjnym) i go zamienia. (Np. dla zadań z ściezki krytycnzej wybieramy najsyzvsze procesory). Zmienia ta funkcje wyboru itd. Przeliczamy funkcje kosztu i czasu. Z przeliczenia genotypu na architektury to odwzorowanie genotypu w fenotyp
- krzyżowanie - wybiera xi (czegoś) .xi = gamma \* pi, gamma in [0, 1]. Wybieramy selekcja xi rozwiązań, łaczymy losowo w pary. W każdej parze wybieramy dwa punkty i zamieniamy w nich poddrzewa. (krzyżujemy, pierwsza cześc z pierwszego z druga z drugiego.) (1,2) i (2,1).

gamma + beta + Delta != 0

Kontynuujemy operacje do warunku stopu. Osobnik na samej górze jest najleszy. Mamy hardtime constraint, którego nie można przekroczyć. Wybieramy tylko to co jest możliwe. Trzeba sprawdzić czy wgl czas jest jakkolwiek realistyczny. Najlepszy osobnik to najtańszy nieprzekraczajacy czasu.

Przynajmniej 5-6 funkcji wyboru.
