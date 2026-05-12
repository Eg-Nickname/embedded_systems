# Lab7 chyba idc

Generujemy grafy rozszerzone. Generujemy graf. Niektóre zadania moga byc rozdzielone. W lisice sasiedztwa:
T0 2 1(5) 2(19){3}
W nawiasie klamrowym losujemy czy nasze zdanie mozna podzielic. Jesli tak to w {} djaemy na ile pod zadan mozna podzielic.
Poźniej w @cost:
30 20 100
25 340 200
{10, 5, 12} {15, 8, 10} {30, 50, 80}

Podajemy poszczegolne czesci skladowe w {} lda kazdego procesora.
Tak samo robimy w @times:
100 200 10
120 230 8
{20, 30, 80} {70,80,90} {1,1,3}

Wazne zeby w adj list kazdy nastepnik mial poprawnie zaznaczone na ile subtaskow sie dzieli
T0 1(1) 2(2){3}
T1 2(2){3} // tutaj w T0 i T1 dziala git

Ale
T0 1(1) 2(2){3}
T1 2(2){4} // tutaj nie wiemy. Popsute bo ma byc na 3 czy na 4 subtaski? No fizycznie moze tylko na jedna z tych opcji.

Sztucznie ograniczmy na ile maks substastkow (minimum 2 subtaski)
Jakas tam szansa na losowe rozdzielenie zdania na subtaski
